#ifndef TARGET_HPP
#define TARGET_HPP

#include <functional>
#include <iostream>
#include <string>

#include "backendrequirements.hpp"
#include "file.hpp"

class Target {
 public:
  virtual ~Target() {}
  virtual std::string getName() const = 0;
  // Check if the backend can be used with these settings
  virtual bool staticSettingsCheck(BackendRequirements requirements) const = 0;
  // Check if the backend can accept that file
  virtual bool staticFileCheck(BackendRequirements requirements, const File& file) const = 0;
  // Check if the backend is reachable.
  virtual void dynamicSettingsCheck(BackendRequirements requirements,
                                    std::function<void()> successCallback,
                                    std::function<void(std::string)> errorCallback,
                                    int timeoutMillis) = 0;
  // Upload a file.
  virtual void uploadFile(BackendRequirements requirements,
                          const File& file,
                          std::function<void(std::string)> successCallback,
                          std::function<void(std::string)> errorCallback) = 0;
};

template<typename T>
concept ValidTarget = requires(T a) {
  { T::loadTargets() }
  ->std::convertible_to<std::vector<Target*>>;
};

extern "C" struct TargetList {
  int size;
  Target** targets;
};

// Defines a load_targets_dynamically function, that calls type::loadTargets().
// You need to use this makro (or define 'TargetList load_targets_dynamically()' yourself) in your target module, if you want it to be
// dynamically loadable.
#define setTargetType(type)                                                                                                              \
  extern "C" TargetList __attribute__((weak)) load_targets_dynamically() {                                                               \
    static_assert(std::is_base_of<Target, type>::value,                                                                                  \
                  "You have to call the setTargetType makro with your target type like 'setTargetType(NullPointerTarget)'. Your target " \
                  "type has to inherit from Target.'");                                                                                  \
    static_assert(ValidTarget<type>,                                                                                                     \
                  "You have not defined 'static std::vector<Target*> loadTargets()' in your Target class (" #type                        \
                  ". You need to define this method, as it will be called, to to create the Target objects.");                           \
    std::vector<Target*> targets = type::loadTargets();                                                                                  \
    TargetList targetList;                                                                                                               \
    targetList.size = targets.size();                                                                                                    \
    targetList.targets = new Target*[targets.size()];                                                                                    \
    for(int i = 0; i < targetList.size; i++) {                                                                                           \
      targetList.targets[i] = targets[i];                                                                                                \
    }                                                                                                                                    \
    return targetList;                                                                                                                   \
  }

// Ways to declare weak symbol in gcc
//#pragma weak load_targets_dynamically
//_Pragma("weak load_targets_dynamically")
//__attribute__((weak))

#endif
