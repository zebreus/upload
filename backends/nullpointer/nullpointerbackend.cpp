#include "nullpointerbackend.hpp"

setBackendType(NullPointerBackend)

    NullPointerBackend::NullPointerBackend(bool useSSL, const std::string& url, const std::string& name)
    : HttplibBackend(useSSL, url, name) {
  capabilities.maxSize = 512 * 1024 * 1024;
  capabilities.preserveName.reset(new bool(false));
  capabilities.minRetention = 30ll * 24 * 60 * 60 * 1000;
  capabilities.maxRetention = 365ll * 24 * 60 * 60 * 1000;
  capabilities.randomPart = 4;
  capabilities.randomPartWithRandomFilename = 4;
  capabilities.urlLength = 14 + (useSSL ? 1 : 0);
  capabilities.urlLengthWithRandomFilename = 14 + (useSSL ? 1 : 0);
}

bool NullPointerBackend::staticFileCheck(BackendRequirements requirements, const File& file) const {
  if(!checkFile(file)) {
    return false;
  }

  static std::vector<std::string> blacklistedMimetypes = {"application/x-dosexec",
                                                          "application/x-executable",
                                                          "application/x-hdf5",
                                                          "application/vnd.android.package-archive",
                                                          "application/java-archive",
                                                          "application/java-vm"};
  if(!checkMimetype(file, blacklistedMimetypes)) {
    return false;
  }

  long long retention = calculateRetentionPeriod(file);
  if(requirements.minRetention != nullptr) {
    if(*requirements.minRetention > retention) {
      return false;
    }
  }
  if(requirements.maxRetention != nullptr) {
    if(*requirements.maxRetention < retention) {
      return false;
    }
  }

  return true;
}

void NullPointerBackend::uploadFile(BackendRequirements requirements,
                                    const File& file,
                                    std::function<void(std::string)> successCallback,
                                    std::function<void(std::string)> errorCallback) {
  httplib::MultipartFormDataItems items = {{"file", file.getContent(), file.getName(), file.getMimetype()}};

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

long long NullPointerBackend::calculateRetentionPeriod(const File& f) const {
  long long min_age = capabilities.minRetention;
  long long max_age = capabilities.maxRetention;
  size_t max_size = capabilities.maxSize;
  size_t file_size = f.getContent().size();
  long long retention = min_age + (-max_age + min_age) * static_cast<long long>(std::pow((file_size / max_size - 1), 3));
  if(retention < min_age) {
    return min_age;
  } else if(retention > max_age) {
    return max_age;
  } else {
    return retention;
  }
}

std::vector<Backend*> NullPointerBackend::loadBackends() {
  std::vector<Backend*> backends;

  try {
    Backend* httpBackend = new NullPointerBackend(false, "0x0.st", "THE NULL POINTER (HTTP)");
    backends.push_back(httpBackend);
  } catch(std::invalid_argument& e) {
    logger.log(Logger::Info) << "Failed to load NullPointerBackend (http):" << e.what() << "\n";
  }

  try {
    Backend* httpsBackend = new NullPointerBackend(true, "0x0.st", "THE NULL POINTER");
    backends.push_back(httpsBackend);
  } catch(std::invalid_argument& e) {
    logger.log(Logger::Info) << "Failed to load NullPointerBackend (https):" << e.what() << "\n";
  }

  return backends;
}
