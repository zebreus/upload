#include "oshibackend.hpp"

setBackendType(OshiBackend)

    OshiBackend::OshiBackend(bool useSSL, const std::string& url, const std::string& name)
    : HttplibBackend(useSSL, url, name) {
  capabilities.maxSize = 512 * 1024 * 1024;
  capabilities.minRetention = 1ll * 60 * 1000;
  capabilities.maxRetention = 90ll * 24 * 60 * 60 * 1000;
  capabilities.maxDownloads.reset(new long(1));
}

void OshiBackend::uploadFile(BackendRequirements requirements,
                             const File& file,
                             std::function<void(std::string)> successCallback,
                             std::function<void(std::string)> errorCallback) {
  std::shared_ptr<httplib::MultipartFormDataItems> items = generateFormData(requirements, file);

  try {
    std::string response = postForm(*items);
    std::vector<std::string> urls = findValidUrls(response);
    logger.log(Logger::Topic::Debug) << "Received " << urls.size() << " urls." << '\n';
    if(urls.size() == 2) {
      std::string managementUrl = urls[0];
      std::string downloadUrl = urls[1];
      successCallback(downloadUrl);
    } else {
      std::string message = "Response contained an unexpected amount of urls.";
      errorCallback(message);
    }
  } catch(std::runtime_error& error) {
    errorCallback(error.what());
  }
}

std::shared_ptr<httplib::MultipartFormDataItems> OshiBackend::generateFormData(const BackendRequirements& requirements, const File& file) {
  std::shared_ptr<httplib::MultipartFormDataItems> formData(new httplib::MultipartFormDataItems());
  formData->push_back({"f", file.getContent(), file.getName(), file.getMimetype()});

  long long retentionPeriod = determineRetention(requirements);
  int retentionMinutes = static_cast<int>(retentionPeriod / (60ll * 1000ll));
  formData->push_back({"expire", std::to_string(retentionMinutes)});

  if(requirements.maxDownloads != nullptr && *requirements.maxDownloads != 0) {
    formData->push_back({"autodestroy", "1"});
  }

  UrlType type = getUrlType(requirements, file);
  switch(type) {
    case UrlType::ShortRandom:
      formData->push_back({"randomizefn", "1"});
      formData->push_back({"shorturl", "1"});
      break;
    case UrlType::LongRandom:
      formData->push_back({"randomizefn", "1"});
      formData->push_back({"shorturl", "0"});
      break;
    case UrlType::Name:
      formData->push_back({"randomizefn", "0"});
      formData->push_back({"shorturl", "0"});
      break;
    case UrlType::None:
    default:
      logger.log(Logger::Info) << "No valid url type available for uploading to oshi.at. This should not happen, because staticFileCheck "
                                  "should have failed. Continuing anyway, as it is too late too abort."
                               << '\n';
  }

  return formData;
}

std::vector<Backend*> OshiBackend::loadBackends() {
  std::vector<Backend*> backends;

  try {
    Backend* httpsBackend = new OshiBackend(true, "oshi.at", "oshi");
    backends.push_back(httpsBackend);
  } catch(std::invalid_argument& e) {
    logger.log(Logger::Info) << "Failed to load oshi (https):" << e.what() << "\n";
  }

  try {
    Backend* httpBackend = new OshiBackend(false, "oshi.at", "oshi (HTTP)");
    backends.push_back(httpBackend);
  } catch(std::invalid_argument& e) {
    logger.log(Logger::Info) << "Failed to load oshi (HTTP):" << e.what() << "\n";
  }

  return backends;
}

OshiBackend::UrlType OshiBackend::getUrlType(BackendRequirements requirements, const File& file) const {
  // Check if requirements or capabilities specify preserveName
  bool nameUrlPossible = true;
  bool shortRandomUrlPossible = true;
  bool longRandomUrlPossible = true;

  if(requirements.preserveName) {
    if(*requirements.preserveName) {
      shortRandomUrlPossible = false;
      longRandomUrlPossible = false;
    } else {
      nameUrlPossible = false;
    }
  }

  size_t baseUrlLength = predictBaseUrl().size();

  size_t shortRandomPart = 6;
  size_t longRandomPart = 12;

  size_t shortRandomUrlLength = baseUrlLength + shortRandomPart;
  size_t longRandomUrlLength = baseUrlLength + longRandomPart + 1;  //+1 = separator in the middle
  size_t nameUrlLength = baseUrlLength + shortRandomPart + 1 + file.getName().size();

  if(shortRandomUrlPossible) {
    shortRandomUrlPossible = checkUrl(requirements, shortRandomUrlLength, shortRandomPart);
  }

  if(longRandomUrlPossible) {
    longRandomUrlPossible = checkUrl(requirements, longRandomUrlLength, longRandomPart);
  }

  if(nameUrlPossible) {
    nameUrlPossible = checkUrl(requirements, nameUrlLength, shortRandomPart);
  }

  if(nameUrlPossible) {
    return UrlType::Name;
  } else if(shortRandomUrlPossible) {
    return UrlType::ShortRandom;
  } else if(longRandomUrlPossible) {
    return UrlType::LongRandom;
  } else {
    return UrlType::None;
  }
}

bool OshiBackend::staticFileCheck(BackendRequirements requirements, const File& file) const {
  UrlType type = getUrlType(requirements, file);
  return checkFile(file) && type != UrlType::None;
}
