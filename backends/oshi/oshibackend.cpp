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
  httplib::MultipartFormDataItems items = {{"f", file.getContent(), file.getName(), file.getMimetype()}};

  httplib::Headers headers = generateHeaders(requirements, file);

  try {
    std::string response = postForm(items, headers);
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

httplib::Headers OshiBackend::generateHeaders(BackendRequirements requirements, const File& file) {
  httplib::Headers headers;

  long long retentionPeriod = determineRetention(requirements);
  int retentionMinutes = retentionPeriod / (60 * 1000);
  headers.insert({"expire", std::to_string(retentionMinutes)});

  if(requirements.maxDownloads != nullptr && *requirements.maxDownloads != 0) {
    headers.insert({"autodestroy", "1"});
  }

  if(requirements.preserveName != nullptr) {
    if(*requirements.preserveName) {
      headers.insert({"shorturl", "0"});
      headers.insert({"randomizefn", "0"});
    } else {
      headers.insert({"randomizefn", "1"});
    }
  }

  return headers;
}

std::vector<Backend*> OshiBackend::loadBackends() {
  std::vector<Backend*> backends;

  try {
    Backend* httpBackend = new OshiBackend(false, "oshi.at", "oshi (HTTP)");
    backends.push_back(httpBackend);
  } catch(std::invalid_argument& e) {
    logger.log(Logger::Info) << "Failed to load oshi (HTTP):" << e.what() << "\n";
  }

  try {
    Backend* httpsBackend = new OshiBackend(true, "oshi.at", "oshi");
    backends.push_back(httpsBackend);
  } catch(std::invalid_argument& e) {
    logger.log(Logger::Info) << "Failed to load oshi (https):" << e.what() << "\n";
  }

  return backends;
}
