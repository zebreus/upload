#include "nullpointertarget.hpp"

setTargetType(NullPointerTarget)

    NullPointerTarget::NullPointerTarget(bool useSSL, const std::string& url, const std::string& name)
    : HttplibTarget(useSSL, url, name) {
  capabilities.maxSize = 512 * 1024 * 1024;
  capabilities.preserveName.reset(new bool(false));
  capabilities.minRetention = (long long)30 * 24 * 60 * 1000;
  capabilities.maxRetention = (long long)365 * 24 * 60 * 1000;
}

bool NullPointerTarget::staticFileCheck(BackendRequirements requirements, const File& file) const {
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

void NullPointerTarget::uploadFile(BackendRequirements requirements,
                                   const File& file,
                                   std::function<void(std::string)> successCallback,
                                   std::function<void(std::string)> errorCallback) {
  httplib::MultipartFormDataItems items = {{"file", file.getContent(), file.getName(), file.getMimetype()}};

  try {
    std::string response = postForm(items);
    std::vector<std::string> urls = findValidUrls(response);
    if(urls.size() >= 1) {
      successCallback(urls.front());
    } else {
      std::string message = "Response did not contain any urls";
      errorCallback(message);
    }
  } catch(std::runtime_error& error) {
    errorCallback(error.what());
  }
}

long long NullPointerTarget::calculateRetentionPeriod(const File& f) const {
  long long min_age = capabilities.minRetention;
  long long max_age = capabilities.maxRetention;
  long long max_size = capabilities.maxSize;
  long long file_size = f.getContent().size();
  long long retention = min_age + (-max_age + min_age) * pow((file_size / max_size - 1), 3);
  if(retention < min_age) {
    return min_age;
  } else if(retention > max_age) {
    return max_age;
  } else {
    return retention;
  }
}

std::vector<Target*> NullPointerTarget::loadTargets() {
  std::vector<Target*> targets;

  try {
    Target* httpTarget = new NullPointerTarget(false, "0x0.st", "THE NULL POINTER (HTTP)");
    targets.push_back(httpTarget);
  } catch(std::invalid_argument& e) {
    logger.log(Logger::Info) << "Failed to load nullpointertarget (http):" << e.what() << "\n";
  }

  try {
    Target* httpsTarget = new NullPointerTarget(true, "0x0.st", "THE NULL POINTER");
    targets.push_back(httpsTarget);
  } catch(std::invalid_argument& e) {
    logger.log(Logger::Info) << "Failed to load nullpointertarget (https):" << e.what() << "\n";
  }

  return targets;
}
