#ifndef IX_BACKEND_HPP
#define IX_BACKEND_HPP

#include <httplib.h>

#include <backend.hpp>
#include <httplibbackend.hpp>
#include <logger.hpp>
#include <regex>
#include <string>
#include <variant>

class IxBackend: public HttplibBackend {
 public:
  explicit IxBackend(bool useSSL, const std::string& url = "ix.io", const std::string& name = "ix");
  [[nodiscard]] bool staticFileCheck(BackendRequirements requirements, const File& file) const override;
  void uploadFile(BackendRequirements requiredFeatures,
                  const File& file,
                  std::function<void(std::string)> successCallback,
                  std::function<void(std::string)> errorCallback) override;
  static std::vector<Backend*> loadBackends();

 private:
  [[nodiscard]] std::string predictUrl(BackendRequirements requirements, const File& file) const override;
};

#endif
