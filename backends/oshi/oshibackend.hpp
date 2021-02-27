#ifndef OSHI_BACKEND_HPP
#define OSHI_BACKEND_HPP

#include <httplib.h>

#include <httplibbackend.hpp>
#include <logger.hpp>
#include <regex>
#include <string>
#include <variant>

class OshiBackend: public HttplibBackend {
 public:
  explicit OshiBackend(bool useSSL, const std::string& url = "oshi.at", const std::string& name = "OshiUpload");
  void uploadFile(BackendRequirements requiredFeatures,
                  const File& file,
                  std::function<void(std::string)> successCallback,
                  std::function<void(std::string)> errorCallback) override;
  static std::vector<Backend*> loadBackends();

 private:
  std::shared_ptr<httplib::MultipartFormDataItems> generateFormData(const BackendRequirements& requirements, const File& file);
};

#endif
