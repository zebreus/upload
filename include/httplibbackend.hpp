#ifndef HTTPLIB_BACKEND_HPP
#define HTTPLIB_BACKEND_HPP

#include <httplib.h>

#include <backend.hpp>
#include <logger.hpp>

class HttplibBackend: public Backend {
 public:
  HttplibBackend(bool useSSL, const std::string& url, const std::string& name);
  virtual ~HttplibBackend();
  virtual std::string getName() const override;
  virtual bool staticSettingsCheck(BackendRequirements requirements) const override;
  virtual bool staticFileCheck(BackendRequirements requirements, const File& file) const override;
  virtual void dynamicSettingsCheck(BackendRequirements requirements,
                                    std::function<void()> successCallback,
                                    std::function<void(std::string)> errorCallback,
                                    int timeoutMillis) override;
  virtual void uploadFile(BackendRequirements requirements,
                          const File& file,
                          std::function<void(std::string)> successCallback,
                          std::function<void(std::string)> errorCallback) override = 0;

 protected:
  static constexpr auto userAgent = "upload/0.0";
  // TODO improve expression
  static constexpr auto urlRegexString = "http[-\\]_.~!*'();:@&=+$,/?%#[A-z0-9]+";
  std::string name;
  std::string url;
  bool useSSL;
  BackendCapabilities capabilities;
  httplib::Client* client;

  bool isReachable(std::string& errorMessage);
  bool checkFile(const File& f) const;
  void initializeClient();
  std::string getErrorMessage(httplib::Error error);
  bool checkMimetype(const File& file, const std::vector<std::string>& blacklist) const;
  std::string postForm(const httplib::MultipartFormDataItems& form, const httplib::Headers& headers = {});
  std::string putFile(const File& file, const httplib::Headers& headers = {});
  std::vector<std::string> findValidUrls(const std::string& input);
  long long determineRetention(BackendRequirements requirements);
  long determineMaxDownloads(BackendRequirements requirements);
};

inline HttplibBackend::HttplibBackend(bool useSSL, const std::string& url, const std::string& name)
    : name(name), url(url), useSSL(useSSL), client(nullptr) {
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

  initializeClient();
}

inline HttplibBackend::~HttplibBackend() {
  delete client;
}

inline std::string HttplibBackend::getName() const {
  return name;
}

inline bool HttplibBackend::staticFileCheck(BackendRequirements requirements, const File& file) const {
  return checkFile(file);
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
  if(isReachable(errorMessage)) {
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

inline void HttplibBackend::initializeClient() {
  if(client == nullptr) {
    std::string httpUrl;
    if(useSSL) {
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
      logger.log(Logger::Topic::Debug) << "HTTPS is supported\n";
      httpUrl.append("https://");
#else
      logger.log(Logger::Topic::Debug) << "HTTPS is not supported\n";
      throw std::invalid_argument("https is disabled");
#endif
    } else {
      std::string httpUrl = "http://";
    }
    httpUrl.append(url);

    client = new httplib::Client(httpUrl.c_str());

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
    case httplib::SSLLoadingCerts:
    case httplib::SSLServerVerification:
      message << "Some kind of SSL error happened.";
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

inline bool HttplibBackend::checkMimetype(const File& file, const std::vector<std::string>& blacklist) const {
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

inline std::vector<std::string> HttplibBackend::findValidUrls(const std::string& input) {
  std::regex urlExpression(urlRegexString, std::regex::icase | std::regex::ECMAScript);
  auto urlsBegin = std::sregex_iterator(input.begin(), input.end(), urlExpression);
  auto urlsEnd = std::sregex_iterator();

  std::vector<std::string> resultsVector;
  for(std::sregex_iterator i = urlsBegin; i != urlsEnd; ++i) {
    std::smatch match = *i;
    resultsVector.push_back(match.str());
  }

  return resultsVector;
}

inline long long HttplibBackend::determineRetention(BackendRequirements requirements) {
  // Assumes that a valid retention duration exists
  long long period = capabilities.maxRetention;
  if(requirements.maxRetention != nullptr) {
    if(*requirements.maxRetention < period) {
      period = *requirements.maxRetention;
    }
  }
  return period;
}

inline long HttplibBackend::determineMaxDownloads(BackendRequirements requirements) {
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
#endif
