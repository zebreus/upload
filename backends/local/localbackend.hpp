#ifndef LOCAL_BACKEND_HPP
#define LOCAL_BACKEND_HPP

#include <filesystem>
#include <fstream>
#include <string>

#include "backend.hpp"

#ifdef __unix__
#include <unistd.h>
#endif

class LocalBackend: public Backend {
  static constexpr BackendFeatures supportedFeatures = BackendFeatures::Http | BackendFeatures::PreserveName;
  static constexpr auto name = "localbackend";
  std::filesystem::path basePath;

 public:
  LocalBackend(std::filesystem::path basePath);
  std::string getName() const override;
  bool staticSettingsCheck(BackendRequirements requiredFeatures) const override;
  bool staticFileCheck(BackendRequirements requiredFeatures, const File& file) const override;
  void dynamicSettingsCheck(BackendRequirements requiredFeatures,
                            std::function<void()> successCallback,
                            std::function<void(std::string)> errorCallback,
                            int timeoutMillis) override;
  void uploadFile(BackendRequirements requiredFeatures,
                  const File& file,
                  std::function<void(std::string)> successCallback,
                  std::function<void(std::string)> errorCallback) override;
  static std::vector<Backend*> loadBackends();

 private:
  bool fileCanBeCreated(std::filesystem::path path);
  bool validBasePath();
};

#endif
