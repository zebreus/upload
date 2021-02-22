#ifndef LOCAL_TARGET_HPP
#define LOCAL_TARGET_HPP

#include "target.hpp"
#include <filesystem>
#include <fstream>
#include <string>

#ifdef __unix__
#include <unistd.h>
#endif


class LocalTarget : public Target {
  static constexpr BackendFeatures supportedFeatures = BackendFeatures::Http | BackendFeatures::PreserveName;
  static constexpr auto name = "localtarget";
  
  std::filesystem::path basePath;

public:
  LocalTarget(std::filesystem::path basePath);
  std::string getName() const override;
  BackendFeatures getSupportedFeatures() const override;
  bool staticSettingsCheck(BackendFeatures requiredFeatures) const override;
  bool staticFileCheck(BackendFeatures requiredFeatures, const File& file) const override;
  void dynamicSettingsCheck(BackendFeatures requiredFeatures, std::function<void()> successCallback, std::function<void(std::string)> errorCallback, int timeoutMillis) override;
  void uploadFile(BackendFeatures requiredFeatures, const File& file, std::function<void(std::string)> successCallback, std::function<void(std::string)> errorCallback) override;
  static std::vector<Target*> loadTargets();
private:
  bool fileCanBeCreated(std::filesystem::path path);
  bool validBasePath();
};

#endif
