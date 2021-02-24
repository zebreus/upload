#include "transfershtarget.hpp"

setTargetType(TransferShTarget)

    TransferShTarget::TransferShTarget(bool useSSL, const std::string& url, const std::string& name)
    : HttplibTarget(useSSL, url, name) {
  capabilities.maxSize = (long long)10 * 1024 * 1024 * 1024;
  capabilities.preserveName.reset(new bool(true));
  capabilities.minRetention = (long long)1 * 24 * 60 * 1000;
  capabilities.maxRetention = (long long)14 * 24 * 60 * 1000;
  capabilities.maxDownloads.reset(new long(LONG_MAX));
}

void TransferShTarget::uploadFile(BackendRequirements requirements,
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
      int afterHost = resultUrl.find(url) + url.size() + 1;
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

std::vector<Target*> TransferShTarget::loadTargets() {
  std::vector<Target*> targets;

  try {
    Target* httpTarget = new TransferShTarget(false, "transfer.sh", "transfer.sh (HTTP)");
    targets.push_back(httpTarget);
  } catch(std::invalid_argument& e) {
    logger.log(Logger::Info) << "Failed to load TransferShTarget (http):" << e.what() << "\n";
  }

  try {
    Target* httpsTarget = new TransferShTarget(true, "transfer.sh", "transfer.sh");
    targets.push_back(httpsTarget);
  } catch(std::invalid_argument& e) {
    logger.log(Logger::Info) << "Failed to load TransferShTarget (https):" << e.what() << "\n";
  }

  return targets;
}
