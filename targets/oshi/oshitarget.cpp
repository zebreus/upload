#include "oshitarget.hpp"

setTargetType(OshiTarget)

    OshiTarget::OshiTarget(bool useSSL, const std::string& url, const std::string& name)
    : HttplibTarget(useSSL, url, name) {
  capabilities.maxSize = 512 * 1024 * 1024;
  capabilities.minRetention = 1ll * 60 * 1000;
  capabilities.maxRetention = 90ll * 24 * 60 * 1000;
  capabilities.maxDownloads.reset(new long(1));
}

void OshiTarget::uploadFile(BackendRequirements requirements,
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

httplib::Headers OshiTarget::generateHeaders(BackendRequirements requirements, const File& file) {
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

std::vector<Target*> OshiTarget::loadTargets() {
  std::vector<Target*> targets;

  try {
    Target* httpTarget = new OshiTarget(false, "oshi.at", "oshi (HTTP)");
    targets.push_back(httpTarget);
  } catch(std::invalid_argument& e) {
    logger.log(Logger::Info) << "Failed to load oshi (HTTP):" << e.what() << "\n";
  }

  try {
    Target* httpsTarget = new OshiTarget(true, "oshi.at", "oshi");
    targets.push_back(httpsTarget);
  } catch(std::invalid_argument& e) {
    logger.log(Logger::Info) << "Failed to load oshi (https):" << e.what() << "\n";
  }

  return targets;
}
