#ifndef TARGET_HPP
#define TARGET_HPP

#include <string>
#include <functional>
#include <iostream>
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
  virtual ~Target(){}
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

template<typename T>
concept ValidTarget = requires(T a) {
  { T::loadTargets() } -> std::convertible_to<std::vector<Target*>>;
};

extern "C" struct TargetList {
    int size;
    Target** targets;
};

// Defines a load_targets_dynamically function, that calls type::loadTargets().
// You need to use this makro (or define 'TargetList load_targets_dynamically()' yourself) in your target module, if you want it to be dynamically loadable.
#define setTargetType(type) \
 __attribute__((weak)) extern "C" TargetList load_targets_dynamically() \
  { \
  static_assert(std::is_base_of<Target, type>::value, "You have to call the setTargetType makro with your target type like 'setTargetType(NullPointerTarget)'. Your target type has to inherit from Target.'"); \
  static_assert(ValidTarget<type>, "You have not defined 'static std::vector<Target*> loadTargets()' in your Target class (" #type ". You need to define this method, as it will be called, to to create the Target objects."); \
  std::clog << "create_object: " #type "\n"; \
  std::vector<Target*> targets = type::loadTargets(); \
  TargetList targetList; \
  targetList.size = targets.size(); \
  targetList.targets = new Target*[targets.size()]; \
  for(int i = 0; i < targetList.size;i++){ \
    targetList.targets[i] = targets[i]; \
  } \
  return targetList; \
} 

// Ways to declare weak symbol in gcc
//#pragma weak load_targets_dynamically
//_Pragma("weak load_targets_dynamically")
//__attribute__((weak))

#endif
