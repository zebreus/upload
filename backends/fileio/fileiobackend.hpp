#ifndef FILE_IO_BACKEND_HPP
#define FILE_IO_BACKEND_HPP

#include <httplib.h>

#include <backend.hpp>
#include <httplibbackend.hpp>
#include <logger.hpp>
#include <regex>
#include <string>
#include <variant>

class FileIoBackend: public HttplibBackend {
 public:
  explicit FileIoBackend(bool useSSL, const std::string& url = "file.io", const std::string& name = "file.io");
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
