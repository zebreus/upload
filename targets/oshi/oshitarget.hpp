#ifndef OSHI_TARGET_HPP
#define OSHI_TARGET_HPP

#include <httplibtarget.hpp>
#include <string>
#include <regex>
#include <logger.hpp>
#include <httplib.h>
#include <variant>

class OshiTarget : public HttplibTarget {
public:
  OshiTarget(bool useSSL, const std::string& url = "oshi.at", const std::string& name = "OshiUpload");
  void uploadFile(BackendRequirements requiredFeatures, const File& file, std::function<void(std::string)> successCallback, std::function<void(std::string)> errorCallback) override;
  static std::vector<Target*> loadTargets();
private:
  httplib::Headers generateHeaders(BackendRequirements requirements, const File& file);
};


#endif
