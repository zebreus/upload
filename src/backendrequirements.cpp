#include "backendrequirements.hpp"

bool BackendCapabilities::meetsRequirements(BackendRequirements requirements) const {
  if(requirements.http != nullptr) {
    if(!*requirements.http && *requirements.http != http) {
      return false;
    }
  }

  if(requirements.https != nullptr) {
    if(!*requirements.https && *requirements.https != https) {
      return false;
    }
  }

  if(requirements.minSize != nullptr) {
    if(*requirements.minSize > maxSize) {
      return false;
    }
  }

  if(requirements.preserveName != nullptr && preserveName != nullptr) {
    if(*requirements.preserveName != *preserveName) {
      return false;
    }
  }

  if(requirements.minRetention != nullptr) {
    if(*requirements.minRetention > maxRetention) {
      return false;
    }
  }

  if(requirements.maxRetention != nullptr) {
    if(*requirements.maxRetention < minRetention) {
      return false;
    }
  }

  if(requirements.maxDownloads != nullptr) {
    if(maxDownloads == nullptr || (*requirements.maxDownloads > *maxDownloads)) {
      return false;
    }
  }

  return true;
}