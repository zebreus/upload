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
  bool staticCheck(BackendFeatures requiredFeatures) const override;
  void dynamicCheck(BackendFeatures requiredFeatures, const File& file, void (*successCallback)(), void (*errorCallback)(std::string), int timeoutMillis = 200) override;
  void uploadFile(BackendFeatures requiredFeatures, const File& file, void (*successCallback)(std::string), void (*errorCallback)(std::string)) override;
private:
  bool fileCanBeCreated(std::filesystem::path path);
};

#endif
