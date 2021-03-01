#ifndef HTTPLIB_BACKEND_HPP
#define HTTPLIB_BACKEND_HPP

#include <httplib.h>

#include <backend.hpp>
#include <logger.hpp>
#include <utility>

class HttplibBackend: public Backend {
 public:
  HttplibBackend(bool useSSL, std::string url, std::string name, const std::string& userAgent = uploadUserAgent);
  ~HttplibBackend() override;
  [[nodiscard]] std::string getName() const override;
  [[nodiscard]] bool staticSettingsCheck(BackendRequirements requirements) const override;
  [[nodiscard]] bool staticFileCheck(BackendRequirements requirements, const File& file) const override;
  void dynamicSettingsCheck(BackendRequirements requirements,
                            std::function<void()> successCallback,
                            std::function<void(std::string)> errorCallback,
                            int timeoutMillis) override;
  void uploadFile(BackendRequirements requirements,
                  const File& file,
                  std::function<void(std::string)> successCallback,
                  std::function<void(std::string)> errorCallback) override = 0;

 protected:
  static constexpr auto uploadUserAgent = "upload/0.0";
  // TODO improve expression
  static constexpr auto defaultUrlRegex = "http[-\\]_.~!*'();:@&=+$,/?%#[A-z0-9]+";
  static constexpr auto randomCharacter = ' ';
  std::string name;
  std::string url;
  bool useSSL;
  BackendCapabilities capabilities;
  httplib::Client* client;

  [[nodiscard]] bool isReachable(std::string& errorMessage);
  [[nodiscard]] bool checkFile(const File& f) const;
  void initializeClient(const std::string& userAgent);
  std::string getErrorMessage(httplib::Error error);
  [[nodiscard]] static bool checkMimetype(const File& file, const std::vector<std::string>& blacklist);
  std::string postForm(const httplib::MultipartFormDataItems& form, const httplib::Headers& headers = {});
  std::string putFile(const File& file, const httplib::Headers& headers = {});
  static std::vector<std::string> findValidUrls(const std::string& input, const std::string& urlRegex = defaultUrlRegex);
  [[nodiscard]] long long determineRetention(const BackendRequirements& requirements) const;
  [[nodiscard]] long determineMaxDownloads(const BackendRequirements& requirements) const;
  [[nodiscard]] [[maybe_unused]] virtual std::string predictBaseUrl() const;
  [[nodiscard]] [[maybe_unused]] virtual std::string predictUrl(BackendRequirements requirements, const File& file) const;
  // Check a possible fullUrl. Random parts in the fullUrl should be replaced with randomCharacter
  [[nodiscard]] static bool checkUrl(const BackendRequirements& requirements, const std::string& fullUrl);
  [[nodiscard]] static bool checkUrl(const BackendRequirements& requirements, size_t length, size_t randomPart);
};

inline HttplibBackend::HttplibBackend(bool useSSL, std::string url, std::string name, const std::string& userAgent)
    : name(std::move(name)), url(std::move(url)), useSSL(useSSL), client(nullptr) {
  if(useSSL) {
    capabilities.http = false;
    capabilities.https = true;
  } else {
    capabilities.http = true;
    capabilities.https = false;
  }
  capabilities.maxSize = 0;
  capabilities.minRetention = 0ll;
  capabilities.maxRetention = 0ll;

  initializeClient(userAgent);
}

inline HttplibBackend::~HttplibBackend() {
  delete client;
}

inline std::string HttplibBackend::getName() const {
  return name;
}

inline bool HttplibBackend::staticFileCheck(BackendRequirements requirements, const File& file) const {
  const std::string& fullUrl = predictUrl(requirements, file);
  return checkFile(file) && checkUrl(requirements, fullUrl);
}

inline bool HttplibBackend::staticSettingsCheck(BackendRequirements requirements) const {
  if(!capabilities.meetsRequirements(requirements)) {
    logger.log(Logger::Topic::Debug) << "Not all requiredFeatures are supported\n";
    return false;
  }
  return true;
}

inline void HttplibBackend::dynamicSettingsCheck(BackendRequirements requirements,
                                                 std::function<void()> successCallback,
                                                 std::function<void(std::string)> errorCallback,
                                                 int timeoutMillis) {
  std::string errorMessage;
  client->set_connection_timeout(0, timeoutMillis * 1000);
  client->set_read_timeout(0, timeoutMillis * 1000);
  client->set_write_timeout(0, timeoutMillis * 1000);

  bool reachable = isReachable(errorMessage);

  client->set_read_timeout(CPPHTTPLIB_READ_TIMEOUT_SECOND, CPPHTTPLIB_READ_TIMEOUT_USECOND);
  client->set_write_timeout(CPPHTTPLIB_WRITE_TIMEOUT_SECOND, CPPHTTPLIB_WRITE_TIMEOUT_USECOND);
  if(reachable) {
    successCallback();
  } else {
    errorCallback(errorMessage);
  }
}

inline bool HttplibBackend::isReachable(std::string& errorMessage) {
  if(auto result = client->Post("/")) {
    logger.log(Logger::Topic::Debug) << "Received response from " << name << " (" << result->status << "): " << result->body << '\n';
    return true;
  } else {
    errorMessage = getErrorMessage(result.error());
    return false;
  }
}

inline bool HttplibBackend::checkFile(const File& file) const {
  if(file.getContent().size() > capabilities.maxSize) {
    logger.log(Logger::Topic::Info) << name << " has a size limit of 512 MiB per file." << '\n';
    return false;
  }
  return true;
}

#ifdef INTEGRATED_CERTIFICATES
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
#include <openssl/bio.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

#include <cacert.hpp>

inline void loadIntegratedCerts(SSL_CTX* ctx) {
  BIO* cbio = BIO_new_mem_buf(cacertpem, sizeof(cacertpem));
  X509_STORE* cts = SSL_CTX_get_cert_store(ctx);
  if(!cts || !cbio) {
    logger.log(Logger::Debug) << "Loading integrated certificates failed.";
    return;
  }
  X509_INFO* itmp;
  int i, count = 0, type = X509_FILETYPE_PEM;
  STACK_OF(X509_INFO)* inf = PEM_X509_INFO_read_bio(cbio, NULL, NULL, NULL);

  if(!inf) {
    BIO_free(cbio);  // cleanup
    logger.log(Logger::Debug) << "Loading integrated certificates failed.";
    return;
  }
  // iterate over all entries from the pem file, add them to the x509_store one by one
  for(i = 0; i < sk_X509_INFO_num(inf); i++) {
    itmp = sk_X509_INFO_value(inf, i);
    if(itmp->x509) {
      X509_STORE_add_cert(cts, itmp->x509);
      count++;
    }
    if(itmp->crl) {
      X509_STORE_add_crl(cts, itmp->crl);
      count++;
    }
  }
  sk_X509_INFO_pop_free(inf, X509_INFO_free);  // cleanup
  BIO_free(cbio);                              // cleanup
}
#endif
#endif

inline void HttplibBackend::initializeClient(const std::string& userAgent) {
  if(client == nullptr) {
    std::string httpUrl;
    if(useSSL) {
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
      logger.log(Logger::Topic::Debug) << "HTTPS is supported\n";
      httpUrl = "https://";
#else
      logger.log(Logger::Topic::Debug) << "HTTPS is not supported\n";
      throw std::invalid_argument("https is disabled");
#endif
    } else {
      httpUrl = "http://";
    }
    httpUrl.append(url);

    client = new httplib::Client(httpUrl.c_str());
#ifdef INTEGRATED_CERTIFICATES
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
    if(useSSL) {
      loadIntegratedCerts(client->ssl_context());
    }
#else
#error "You have activated integrated certificates, did not activate openssl support. Maybe try defining CPPHTTPLIB_OPENSSL_SUPPORT."
#endif
#endif
    httplib::Headers headers = {{"Accept", "*/*"}, {"User-Agent", userAgent}};
    client->set_default_headers(headers);
  }
}

inline std::string HttplibBackend::getErrorMessage(httplib::Error error) {
  std::stringstream message;
  switch(error) {
    case httplib::Connection:
    case httplib::BindIPAddress:
      message << name << " is not online. Check your internet connection.";
      break;
    case httplib::Read:
    case httplib::Write:
    case httplib::Canceled:
      message << name << " seems to be online, but something went wrong.";
      break;
    case httplib::SSLConnection:
      message << "Failed establishing a SSL connection";
      break;
    case httplib::SSLLoadingCerts:
      message << "Unable to load SSL certificates";
      break;
    case httplib::SSLServerVerification:
      message << "Unable to verify the servers SSL certificate";
      break;
    case httplib::ExceedRedirectCount:
      message << "Too many redirects. This is probably not your fault.";
      break;
    case httplib::Success:
    case httplib::Unknown:
    case httplib::UnsupportedMultipartBoundaryChars:
    case httplib::Compression:
    default:
      message << "Failed to establish connection. Maybe try again later.";
  }
  return message.str();
}

inline bool HttplibBackend::checkMimetype(const File& file, const std::vector<std::string>& blacklist) {
  std::string mimetype = file.getMimetype();
  for(const std::string& blacklistEntry : blacklist) {
    if(blacklistEntry == mimetype) {
      logger.log(Logger::Topic::Info) << " does not allow the upload of " << mimetype << " files." << '\n';
      return false;
    }
  }
  return true;
}

inline std::string HttplibBackend::postForm(const httplib::MultipartFormDataItems& form, const httplib::Headers& headers) {
  if(auto result = client->Post("/", headers, form)) {
    logger.log(Logger::Topic::Debug) << "Received response from " << name << " (" << result->status << "): " << result->body << '\n';
    if(result->status != 200) {
      std::stringstream message;
      message << "Request failed, responsecode " << httplib::detail::status_message(result->status) << "(" << result->status << ").";
      throw(std::runtime_error(message.str()));
    }

    return result->body;
  } else {
    std::stringstream message;
    message << "Request failed: " << getErrorMessage(result.error()) << ".";
    throw(std::runtime_error(message.str()));
  }
}

inline std::string HttplibBackend::putFile(const File& file, const httplib::Headers& headers) {
  std::string path = "/";
  path.append(file.getName());
  if(auto result = client->Put(path.c_str(), headers, file.getContent().data(), file.getContent().size(), file.getMimetype().c_str())) {
    logger.log(Logger::Topic::Debug) << "Received response from " << name << " (" << result->status << "): " << result->body << '\n';
    if(result->status != 200) {
      std::stringstream message;
      message << "Request failed, responsecode " << httplib::detail::status_message(result->status) << "(" << result->status << ").";
      throw(std::runtime_error(message.str()));
    }

    return result->body;
  } else {
    std::stringstream message;
    message << "Request failed: " << getErrorMessage(result.error()) << ".";
    throw(std::runtime_error(message.str()));
  }
}

inline std::vector<std::string> HttplibBackend::findValidUrls(const std::string& input, const std::string& urlRegex) {
  std::regex urlExpression(urlRegex, std::regex::icase | std::regex::ECMAScript);
  auto urlsBegin = std::sregex_iterator(input.begin(), input.end(), urlExpression);
  auto urlsEnd = std::sregex_iterator();

  std::vector<std::string> resultsVector;
  for(std::sregex_iterator i = urlsBegin; i != urlsEnd; ++i) {
    std::smatch match = *i;
    resultsVector.push_back(match.str());
  }

  return resultsVector;
}

inline long long HttplibBackend::determineRetention(const BackendRequirements& requirements) const {
  // Assumes that a valid retention duration exists
  long long period = capabilities.maxRetention;
  if(requirements.maxRetention != nullptr) {
    if(*requirements.maxRetention < period) {
      period = *requirements.maxRetention;
    }
  }
  return period;
}

inline long HttplibBackend::determineMaxDownloads(const BackendRequirements& requirements) const {
  // Assumes that a valid download limit exists
  long maxDownloads = LONG_MAX;
  if(capabilities.maxDownloads != nullptr) {
    maxDownloads = *capabilities.maxDownloads;
  }
  if(requirements.maxDownloads != nullptr) {
    if(*requirements.maxDownloads < maxDownloads) {
      maxDownloads = *requirements.maxDownloads;
    }
  }
  return maxDownloads;
}

inline bool HttplibBackend::checkUrl(const BackendRequirements& requirements, const std::string& fullUrl) {
  unsigned int randomCharacters = 0;
  for(char c : fullUrl) {
    if(c == randomCharacter) {
      randomCharacters++;
    }
  }

  return checkUrl(requirements, fullUrl.size(), randomCharacters);
}

inline bool HttplibBackend::checkUrl(const BackendRequirements& requirements, size_t length, size_t randomPart) {
  if(requirements.minRandomPart) {
    if(*requirements.minRandomPart > randomPart) {
      return false;
    }
  }

  if(requirements.maxRandomPart) {
    if(*requirements.maxRandomPart < randomPart) {
      return false;
    }
  }

  if(requirements.maxUrlLength) {
    if(*requirements.maxUrlLength < length) {
      return false;
    }
  }

  return true;
}

[[maybe_unused]] inline std::string HttplibBackend::predictBaseUrl() const {
  std::string predictedUrl;
  if(useSSL) {
    predictedUrl.append("https://");
  } else {
    predictedUrl.append("http://");
  }

  predictedUrl.append(url);
  predictedUrl.append("/");

  return predictedUrl;
}

inline std::string HttplibBackend::predictUrl(BackendRequirements, const File&) const {
  logger.log(Logger::Debug) << "Called default predictUrl, this should not happen. You should check your backend implementation"
                            << "\n";
  return std::string();
}

#endif
