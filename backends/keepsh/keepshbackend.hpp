#ifndef KEEP_SH_BACKEND_HPP
#define KEEP_SH_BACKEND_HPP

#include <httplib.h>

#include <httplibbackend.hpp>
#include <logger.hpp>
#include <regex>
#include <string>
#include <variant>

//TODO The KeepShBackend should not be used yet, as the generated links do not support direct downloads
#warning "The KeepShBackend should not be used yet, as the generated links do not support direct downloads"
class KeepShBackend: public HttplibBackend {
 public:
  explicit KeepShBackend(bool useSSL, const std::string& url = "free.keep.sh", const std::string& name = "free.keep.sh");
  void uploadFile(BackendRequirements requiredFeatures,
                  const File& file,
                  std::function<void(std::string)> successCallback,
                  std::function<void(std::string)> errorCallback) override;
  static std::vector<Backend*> loadBackends();

 private:
  [[nodiscard]] std::string predictUrl(BackendRequirements requirements, const File& file) const override;
};

#endif
