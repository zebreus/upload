#include <fileiobackend.hpp>
#include <nullpointerbackend.hpp>
#include <oshibackend.hpp>
#include <transfershbackend.hpp>

#include "backendloader.hpp"

std::vector<std::shared_ptr<Backend>> loadBackends() {
  std::vector<std::shared_ptr<Backend>> backends;

  for(Backend* backend : NullPointerBackend::loadBackends()) {
    backends.push_back(std::shared_ptr<Backend>(backend));
  }

  for(Backend* backend : OshiBackend::loadBackends()) {
    backends.push_back(std::shared_ptr<Backend>(backend));
  }

  for(Backend* backend : TransferShBackend::loadBackends()) {
    backends.push_back(std::shared_ptr<Backend>(backend));
  }

  for(Backend* backend : FileIoBackend::loadBackends()) {
    backends.push_back(std::shared_ptr<Backend>(backend));
  }

  return backends;
}
