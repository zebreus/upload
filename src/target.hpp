#ifndef TARGET_HPP
#define TARGET_HPP

#include <string>
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
  //Check if the backend can be used
  virtual bool staticCheck(BackendFeatures requiredFeatures) const = 0;
  //Check if the backend is available
  virtual void dynamicCheck(BackendFeatures requiredFeatures, const File& file, void (*successCallback)(), void (*errorCallback)(std::string), int timeoutMillis) = 0;
  virtual void uploadFile(BackendFeatures requiredFeatures, const File& file, void (*successCallback)(std::string), void (*errorCallback)(std::string)) = 0;
};

#endif
