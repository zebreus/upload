#ifndef OSHI_BACKEND_HPP
#define OSHI_BACKEND_HPP

#include <httplib.h>

#include <httplibbackend.hpp>
#include <logger.hpp>
#include <regex>
#include <string>
#include <variant>

class OshiBackend: public HttplibBackend {
  enum UrlType { Name, ShortRandom, LongRandom, None };

 public:
  explicit OshiBackend(bool useSSL, const std::string& url = "oshi.at", const std::string& name = "OshiUpload");
  [[nodiscard]] bool staticFileCheck(BackendRequirements requirements, const File& file) const override;
  void uploadFile(BackendRequirements requiredFeatures,
                  const File& file,
                  std::function<void(std::string)> successCallback,
                  std::function<void(std::string)> errorCallback) override;
  static std::vector<Backend*> loadBackends();

 private:
  std::shared_ptr<httplib::MultipartFormDataItems> generateFormData(const BackendRequirements& requirements, const File& file);
  // Determine the url type. If the url of the returned url type is not compatible with requirements, no other urlType is as well.
  [[nodiscard]] UrlType getUrlType(BackendRequirements requirements, const File& file) const;
};

#endif
