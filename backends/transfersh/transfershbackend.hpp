#ifndef TRANSFER_SH_BACKEND_HPP
#define TRANSFER_SH_BACKEND_HPP

#include <httplib.h>

#include <httplibbackend.hpp>
#include <logger.hpp>
#include <regex>
#include <string>
#include <variant>

class TransferShBackend: public HttplibBackend {
 public:
  TransferShBackend(bool useSSL, const std::string& url = "transfer.sh", const std::string& name = "transfer.sh");
  void uploadFile(BackendRequirements requiredFeatures,
                  const File& file,
                  std::function<void(std::string)> successCallback,
                  std::function<void(std::string)> errorCallback) override;
  static std::vector<Backend*> loadBackends();
};

#endif
