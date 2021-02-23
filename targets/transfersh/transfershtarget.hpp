#ifndef TRANSFER_SH_TARGET_HPP
#define TRANSFER_SH_TARGET_HPP

#include <httplibtarget.hpp>
#include <string>
#include <regex>
#include <logger.hpp>
#include <httplib.h>
#include <variant>

class TransferShTarget : public HttplibTarget {
public:
  TransferShTarget(bool useSSL, const std::string& url = "transfer.sh", const std::string& name = "transfer.sh");
  void uploadFile(BackendRequirements requiredFeatures, const File& file, std::function<void(std::string)> successCallback, std::function<void(std::string)> errorCallback) override;
  static std::vector<Target*> loadTargets();
private:
  long long calculateRetentionPeriod(const File& f) const;
};

#endif