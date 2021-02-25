#include <localbackend.hpp>
#include <nullpointerbackend.hpp>

#include "backendloader.hpp"

std::vector<std::shared_ptr<Backend>> loadBackends() {
  std::vector<std::shared_ptr<Backend>> backends;

  for(Backend* backend : NullPointerBackend::loadBackends()) {
    backends.push_back(std::shared_ptr<Backend>(backend));
  }

  for(Backend* backend : LocalBackend::loadBackends()) {
    backends.push_back(std::shared_ptr<Backend>(backend));
  }

  return backends;
}
