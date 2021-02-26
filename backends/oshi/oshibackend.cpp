#include "oshibackend.hpp"

setBackendType(OshiBackend)

    OshiBackend::OshiBackend(bool useSSL, const std::string& url, const std::string& name)
    : HttplibBackend(useSSL, url, name) {
  capabilities.maxSize = 512 * 1024 * 1024;
  capabilities.minRetention = 1ll * 60 * 1000;
  capabilities.maxRetention = 90ll * 24 * 60 * 60 * 1000;
  capabilities.maxDownloads.reset(new long(1));
  capabilities.randomPart = 6;
  capabilities.randomPartWithRandomFilename = 6;
  capabilities.urlLength = 22 + (useSSL ? 1 : 0);
  capabilities.urlLengthWithRandomFilename = 21 + (useSSL ? 1 : 0);
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

std::shared_ptr<httplib::MultipartFormDataItems> OshiBackend::generateFormData(BackendRequirements requirements, const File& file) {
  std::shared_ptr<httplib::MultipartFormDataItems> formData(new httplib::MultipartFormDataItems());
  formData->push_back(httplib::MultipartFormData("f", file.getContent(), file.getName(), file.getMimetype()));

  long long retentionPeriod = determineRetention(requirements);
  int retentionMinutes = retentionPeriod / (60 * 1000);
  formData->push_back({"expire", std::to_string(retentionMinutes)});

  if(requirements.maxDownloads != nullptr && *requirements.maxDownloads != 0) {
    formData->push_back({"autodestroy", "1"});
  }

  if(capabilities.determinePreserveName(requirements)) {
    formData->push_back({"shorturl", "0"});
    formData->push_back({"randomizefn", "0"});
  } else {
    formData->push_back({"randomizefn", "1"});
    formData->push_back({"shorturl", "1"});
  }

  return formData;
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
