#ifndef NULL_POINTER_BACKEND_HPP
#define NULL_POINTER_BACKEND_HPP

#include <httplib.h>

#include <backend.hpp>
#include <httplibbackend.hpp>
#include <logger.hpp>
#include <regex>
#include <string>
#include <variant>

class NullPointerBackend: public HttplibBackend {
 public:
  explicit NullPointerBackend(bool useSSL, const std::string& url = "0x0.st", const std::string& name = "THE NULL POINTER");
  [[nodiscard]] bool staticFileCheck(BackendRequirements requirements, const File& file) const override;
  void uploadFile(BackendRequirements requiredFeatures,
                  const File& file,
                  std::function<void(std::string)> successCallback,
                  std::function<void(std::string)> errorCallback) override;
  static std::vector<Backend*> loadBackends();

 private:
  [[nodiscard]] long long calculateRetentionPeriod(const File& f) const;
};

#endif
