#ifndef NULL_POINTER_TARGET_HPP
#define NULL_POINTER_TARGET_HPP

#include <target.hpp>
#include <httplibtarget.hpp>
#include <string>
#include <regex>
#include <logger.hpp>
#include <httplib.h>
#include <variant>

class NullPointerTarget : public HttplibTarget {
public:
  NullPointerTarget(bool useSSL, const std::string& url = "0x0.st", const std::string& name = "THE NULL POINTER");
  bool staticFileCheck(BackendRequirements requirements, const File& file) const override;
  void uploadFile(BackendRequirements requiredFeatures, const File& file, std::function<void(std::string)> successCallback, std::function<void(std::string)> errorCallback) override;
  static std::vector<Target*> loadTargets();
private:
  long long calculateRetentionPeriod(const File& f) const;
};

#endif
