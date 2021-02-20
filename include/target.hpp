#ifndef TARGET_HPP
#define TARGET_HPP

#include <string>
#include <functional>
#include "file.hpp"

enum BackendFeatures{
  None = 0,
  PreserveName = 1 << 0,
  Http = 1 << 1,
  Https = 1 << 2
};

inline constexpr BackendFeatures operator~ (BackendFeatures a) { return (BackendFeatures)~(int)a; }
inline constexpr BackendFeatures operator| (BackendFeatures a, BackendFeatures b) { return (BackendFeatures)((int)a | (int)b); }
inline constexpr BackendFeatures operator& (BackendFeatures a, BackendFeatures b) { return (BackendFeatures)((int)a & (int)b); }
inline constexpr BackendFeatures operator^ (BackendFeatures a, BackendFeatures b) { return (BackendFeatures)((int)a ^ (int)b); }
inline BackendFeatures& operator|= (BackendFeatures& a, BackendFeatures b) { return (BackendFeatures&)((int&)a |= (int)b); }
inline BackendFeatures& operator&= (BackendFeatures& a, BackendFeatures b) { return (BackendFeatures&)((int&)a &= (int)b); }
inline BackendFeatures& operator^= (BackendFeatures& a, BackendFeatures b) { return (BackendFeatures&)((int&)a ^= (int)b); }

class Target{
public:
  virtual std::string getName() const = 0;
  virtual BackendFeatures getSupportedFeatures() const = 0;
  //Check if the backend can be used with these settings
  virtual bool staticSettingsCheck(BackendFeatures requiredFeatures) const = 0;
  //Check if the backend can accept that file
  virtual bool staticFileCheck(BackendFeatures requiredFeatures, const File& file) const = 0;
  //Check if the backend is reachable.
  virtual void dynamicSettingsCheck(BackendFeatures requiredFeatures, std::function<void()> successCallback, std::function<void(std::string)> errorCallback, int timeoutMillis) = 0;
  //Upload a file.
  virtual void uploadFile(BackendFeatures requiredFeatures, const File& file, std::function<void(std::string)> successCallback, std::function<void(std::string)> errorCallback) = 0;
};

#endif
