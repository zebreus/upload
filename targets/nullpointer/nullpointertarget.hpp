#ifndef NULL_POINTER_TARGET_HPP
#define NULL_POINTER_TARGET_HPP

#include <target.hpp>
#include <string>
#include <regex>
#include <logger.hpp>

class NullPointerTarget : public Target {
  static constexpr auto userAgent = "upload/0.0";
  
  std::string name;
  std::string url;
  bool useSSL;
  BackendCapabilities capabilities;
  httplib::ClientImpl* client;
  
public:
  NullPointerTarget(bool useSSL, const std::string& url = "0x0.st", const std::string& name = "THE NULL POINTER");
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
  void NullPointerTarget::initializeClient();
};

#endif
