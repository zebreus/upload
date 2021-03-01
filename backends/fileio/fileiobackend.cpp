#include "fileiobackend.hpp"

setBackendType(FileIoBackend)

    FileIoBackend::FileIoBackend(bool useSSL, const std::string& url, const std::string& name)
    : HttplibBackend(useSSL, url, name) {
  capabilities.maxSize = 100 * 1024 * 1024;
  capabilities.preserveName.reset(new bool(false));
  capabilities.minRetention = 1ll * 24 * 60 * 60 * 1000;
  capabilities.maxRetention = 365ll * 24 * 60 * 60 * 1000;
}

bool FileIoBackend::staticFileCheck(BackendRequirements requirements, const File& file) const {
  if(!checkFile(file)) {
    return false;
  }

  const std::string& predictedUrl = predictUrl(requirements, file);
  if(!checkUrl(requirements, predictedUrl)) {
    return false;
  }

  return true;
}

void FileIoBackend::uploadFile(BackendRequirements requirements,
                               const File& file,
                               std::function<void(std::string)> successCallback,
                               std::function<void(std::string)> errorCallback) {
  httplib::MultipartFormDataItems items = {{"file", file.getContent(), file.getName(), file.getMimetype()}};

  std::string endpoint;

  long long retentionPeriod = determineRetention(requirements);
  int retentionDays = static_cast<int>(retentionPeriod / (24ll * 60 * 60 * 1000));
  endpoint.append("?expires=");
  endpoint.append(std::to_string(retentionDays));
  endpoint.append("d");

  try {
    std::string response = postForm(items, {}, endpoint);
    std::vector<std::string> urls = findValidUrls(response, "https://file.io/[a-zA-Z0-9]+");
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

std::vector<Backend*> FileIoBackend::loadBackends() {
  std::vector<Backend*> backends;

  try {
    Backend* httpsBackend = new FileIoBackend(true, "file.io", "file.io");
    backends.push_back(httpsBackend);
  } catch(std::invalid_argument& e) {
    logger.log(Logger::Info) << "Failed to load FileIoBackend (https):" << e.what() << "\n";
  }

  return backends;
}

std::string FileIoBackend::predictUrl(BackendRequirements requirements, const File& file) const {
  std::string fullUrl = predictBaseUrl();
  for(int i = 0; i < 12; i++) {
    fullUrl.push_back(randomCharacter);
  }
  return fullUrl;
}
