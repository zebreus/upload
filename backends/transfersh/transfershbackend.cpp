#include "transfershbackend.hpp"

setBackendType(TransferShBackend)

    TransferShBackend::TransferShBackend(bool useSSL, const std::string& url, const std::string& name)
    : HttplibBackend(useSSL, url, name) {
  capabilities.maxSize = 10ll * 1024 * 1024 * 1024;
  capabilities.preserveName.reset(new bool(true));
  capabilities.minRetention = 1ll * 24 * 60 * 1000;
  capabilities.maxRetention = 14ll * 24 * 60 * 1000;
  capabilities.maxDownloads.reset(new long(LONG_MAX));
}

void TransferShBackend::uploadFile(BackendRequirements requirements,
                                   const File& file,
                                   std::function<void(std::string)> successCallback,
                                   std::function<void(std::string)> errorCallback) {
  httplib::Headers headers;
  long long retentionPeriod = determineRetention(requirements);
  int retentionDays = retentionPeriod / (24ll * 60 * 1000);
  headers.insert({"Max-Days", std::to_string(retentionDays)});

  if(requirements.maxDownloads != nullptr) {
    int maxDownloads = determineMaxDownloads(requirements);
    headers.insert({"Max-Downloads", std::to_string(maxDownloads)});
  }

  try {
    std::string response = putFile(file, headers);
    std::vector<std::string> urls = findValidUrls(response);
    if(urls.size() >= 1) {
      std::string resultUrl = urls.front();
      size_t afterHost = resultUrl.find(url) + url.size() + 1;
      try {
        resultUrl.insert(afterHost, "get/");
        successCallback(resultUrl);
      } catch(...) {
        errorCallback("Result url was not formed as expected.");
      }
    } else {
      std::string message = "Response did not contain any urls";
      errorCallback(message);
    }
  } catch(std::runtime_error& error) {
    errorCallback(error.what());
  }
}

std::vector<Backend*> TransferShBackend::loadBackends() {
  std::vector<Backend*> backends;

  try {
    Backend* httpBackend = new TransferShBackend(false, "transfer.sh", "transfer.sh (HTTP)");
    backends.push_back(httpBackend);
  } catch(std::invalid_argument& e) {
    logger.log(Logger::Info) << "Failed to load TransferShBackend (http):" << e.what() << "\n";
  }

  try {
    Backend* httpsBackend = new TransferShBackend(true, "transfer.sh", "transfer.sh");
    backends.push_back(httpsBackend);
  } catch(std::invalid_argument& e) {
    logger.log(Logger::Info) << "Failed to load TransferShBackend (https):" << e.what() << "\n";
  }

  return backends;
}
