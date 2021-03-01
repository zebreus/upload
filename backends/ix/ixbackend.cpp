#include "ixbackend.hpp"

setBackendType(IxBackend)

    IxBackend::IxBackend(bool useSSL, const std::string& url, const std::string& name)
    : HttplibBackend(useSSL, url, name) {
  capabilities.maxSize = 1 * 1024 * 1024;
  capabilities.preserveName.reset(new bool(false));
  capabilities.minRetention = 365ll * 24 * 60 * 60 * 1000;
  capabilities.maxRetention = 365ll * 24 * 60 * 60 * 1000;
}

bool IxBackend::staticFileCheck(BackendRequirements requirements, const File& file) const {
  if(!checkFile(file)) {
    return false;
  }

  const std::string& predictedUrl = predictUrl(requirements, file);
  if(!checkUrl(requirements, predictedUrl)) {
    return false;
  }

  return true;
}

void IxBackend::uploadFile(BackendRequirements requirements,
                           const File& file,
                           std::function<void(std::string)> successCallback,
                           std::function<void(std::string)> errorCallback) {
  httplib::MultipartFormDataItems items = {{"f:1", file.getContent(), file.getName(), file.getMimetype()}};

  try {
    std::string response = postForm(items);
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

std::vector<Backend*> IxBackend::loadBackends() {
  std::vector<Backend*> backends;

  try {
    Backend* httpBackend = new IxBackend(false, "ix.io", "ix");
    backends.push_back(httpBackend);
  } catch(std::invalid_argument& e) {
    logger.log(Logger::Info) << "Failed to load IxBackend (http):" << e.what() << "\n";
  }

  return backends;
}

std::string IxBackend::predictUrl(BackendRequirements requirements, const File& file) const {
  std::string fullUrl = predictBaseUrl();
  for(int i = 0; i < 4; i++) {
    fullUrl.push_back(randomCharacter);
  }
  return fullUrl;
}
