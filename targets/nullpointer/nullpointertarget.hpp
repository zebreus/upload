#ifndef NULL_POINTER_TARGET_HPP
#define NULL_POINTER_TARGET_HPP

#include <target.hpp>
#include <string>
#include <regex>
#include <logger.hpp>

class NullPointerTarget : public Target {
  static constexpr auto name = "THE NULL POINTER";

  static constexpr auto url = "0x0.st";
  static constexpr auto userAgent = "upload/0.0";
  
  BackendCapabilities capabilities;

public:
  NullPointerTarget();
  ~NullPointerTarget() override;
  std::string getName() const override;
  bool staticSettingsCheck(BackendRequirements requiredFeatures) const override;
  bool staticFileCheck(BackendRequirements requiredFeatures, const File& file) const override;
  void dynamicSettingsCheck(BackendRequirements requiredFeatures, std::function<void()> successCallback, std::function<void(std::string)> errorCallback, int timeoutMillis) override;
  void uploadFile(BackendRequirements requiredFeatures, const File& file, std::function<void(std::string)> successCallback, std::function<void(std::string)> errorCallback) override;
  static std::vector<Target*> loadTargets();
private:
  bool isReachable(std::string& errorMessage);
  bool checkFile(const File& f) const;
};

#endif
