#include "keepshbackend.hpp"

setBackendType(KeepShBackend)

    KeepShBackend::KeepShBackend(bool useSSL, const std::string& url, const std::string& name)
    : HttplibBackend(useSSL, url, name, "curl/7.64.1") {
  capabilities.maxSize = 10ll * 1024 * 1024 * 1024;
  capabilities.preserveName.reset(new bool(true));
  capabilities.minRetention = 1ll * 24 * 60 * 60 * 1000;
  capabilities.maxRetention = 14ll * 24 * 60 * 60 * 1000;
  capabilities.maxDownloads.reset(new long(LONG_MAX));
}

void KeepShBackend::uploadFile(BackendRequirements requirements,
                               const File& file,
                               std::function<void(std::string)> successCallback,
                               std::function<void(std::string)> errorCallback) {
  httplib::Headers headers;
  long long retentionPeriod = determineRetention(requirements);
  int retentionDays = static_cast<int>(retentionPeriod / (24ll * 60 * 60 * 1000));
  headers.insert({"Expires-After", std::to_string(retentionDays)});

  if(requirements.maxDownloads != nullptr) {
    long maxDownloads = determineMaxDownloads(requirements);
    headers.insert({"Max-Downloads", std::to_string(maxDownloads)});
  }

  try {
    std::string response = putFile(file, headers);
    std::vector<std::string> urls = findValidUrls(response);
    if(!urls.empty()) {
      successCallback(urls.front());
    } else {
      std::string message = "Response did not contain any urls";
      errorCallback(message);
    }
  } catch(std::runtime_error& error) {
    errorCallback(error.what());
  }
}

std::vector<Backend*> KeepShBackend::loadBackends() {
  std::vector<Backend*> backends;

  try {
    Backend* httpsBackend = new KeepShBackend(true, "free.keep.sh", "keep.sh");
    backends.push_back(httpsBackend);
  } catch(std::invalid_argument& e) {
    logger.log(Logger::Info) << "Failed to load KeepShBackend:" << e.what() << "\n";
  }

  return backends;
}

std::string KeepShBackend::predictUrl(BackendRequirements requirements, const File& file) const {
  std::string fullUrl = predictBaseUrl();
  fullUrl.append("get/");
  for(int i = 0; i < 16; i++) {
    fullUrl.push_back(randomCharacter);
  }
  fullUrl.append("/");
  fullUrl.append(file.getName());
  return fullUrl;
}
