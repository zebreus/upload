#ifndef NULL_POINTER_TARGET_HPP
#define NULL_POINTER_TARGET_HPP

#include "target.hpp"
#include "httplib.h"
#include <string>
#include <regex>
#include "logger.hpp"

class NullPointerTarget : public Target {
  static constexpr BackendFeatures supportedFeatures = BackendFeatures::Http | BackendFeatures::PreserveName;
  static constexpr auto name = "THE NULL POINTER";

  static constexpr auto url = "0x0.st";
  static constexpr auto userAgent = "upload/0.0";

public:
  NullPointerTarget();
  ~NullPointerTarget() override;
  std::string getName() const override;
  BackendFeatures getSupportedFeatures() const override;
  bool staticSettingsCheck(BackendFeatures requiredFeatures) const override;
  bool staticFileCheck(BackendFeatures requiredFeatures, const File& file) const override;
  void dynamicSettingsCheck(BackendFeatures requiredFeatures, std::function<void()> successCallback, std::function<void(std::string)> errorCallback, int timeoutMillis) override;
  void uploadFile(BackendFeatures requiredFeatures, const File& file, std::function<void(std::string)> successCallback, std::function<void(std::string)> errorCallback) override;
  static std::vector<Target*> loadTargets();
private:
  bool isReachable(std::string& errorMessage);
  bool checkFile(const File& f) const;
};

#endif
