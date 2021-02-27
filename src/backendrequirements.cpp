#include "backendrequirements.hpp"

bool BackendCapabilities::meetsRequirements(BackendRequirements requirements) const {
  if(requirements.http != nullptr) {
    if(*requirements.http != http) {
      return false;
    }
  }

  if(requirements.https != nullptr) {
    if(*requirements.https != https) {
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

  bool preserveNameForRequirements = determinePreserveName(requirements);
  if(requirements.minRandomPart != nullptr) {
    if(preserveNameForRequirements) {
      if(*requirements.minRandomPart > randomPart) {
        return false;
      }
    } else {
      if(*requirements.minRandomPart > randomPartWithRandomFilename) {
        return false;
      }
    }
  }

  if(requirements.maxRandomPart != nullptr) {
    if(preserveNameForRequirements) {
      if(*requirements.maxRandomPart < randomPart) {
        return false;
      }
    } else {
      if(*requirements.maxRandomPart < randomPartWithRandomFilename) {
        return false;
      }
    }
  }

  if(requirements.maxUrlLength != nullptr) {
    if(preserveNameForRequirements) {
      if(*requirements.maxUrlLength < urlLength) {
        return false;
      }
    } else {
      if(*requirements.maxUrlLength < urlLengthWithRandomFilename) {
        return false;
      }
    }
  }

  return true;
}

// Helper to determine whether to preserve the filename for the given requirements or not
bool BackendCapabilities::determinePreserveName(const BackendRequirements& requirements) const {
  // Check if requirements or capabilities specify preserveName
  if(requirements.preserveName != nullptr) {
    return *requirements.preserveName;
  }
  if(preserveName != nullptr) {
    return *preserveName;
  }

  // Check if one option violates min or max random part
  if(requirements.minRandomPart != nullptr) {
    if(*requirements.minRandomPart > randomPart) {
      return false;
    }
    if(*requirements.minRandomPart > randomPartWithRandomFilename) {
      return true;
    }
  }

  if(requirements.maxRandomPart != nullptr) {
    if(*requirements.maxRandomPart < randomPart) {
      return false;
    }
    if(*requirements.maxRandomPart < randomPartWithRandomFilename) {
      return true;
    }
  }

  // Check if one option violates max url length
  if(requirements.maxUrlLength != nullptr) {
    if(*requirements.maxUrlLength < urlLength) {
      return false;
    }
    if(*requirements.maxUrlLength < urlLengthWithRandomFilename) {
      return true;
    }
  }

  // If still all both options are valid, return preserve name.
  return true;
}
