#ifndef BACKEND_HPP
#define BACKEND_HPP

#include <functional>
#include <iostream>
#include <string>

#include "backendrequirements.hpp"
#include "file.hpp"

class Backend {
 public:
  virtual ~Backend() = default;
  [[nodiscard]] virtual std::string getName() const = 0;
  // Check if the backend can be used with these settings
  [[nodiscard]] virtual bool staticSettingsCheck(BackendRequirements requirements) const = 0;
  // Check if the backend can accept that file
  [[nodiscard]] virtual bool staticFileCheck(BackendRequirements requirements, const File& file) const = 0;
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
concept ValidBackend = requires(T a) {
  { T::loadBackends() }
  ->std::convertible_to<std::vector<Backend*>>;
};

extern "C" struct BackendList {
  unsigned int size;
  Backend** backends;
};

// Defines a load_backends_dynamically function, that calls type::loadBackends().
// You need to use this macro (or define 'BackendList load_backends_dynamically()' yourself) in your backend module, if you want it to be
// dynamically loadable.
#define setBackendType(type)                                                                                                        \
  extern "C" BackendList __attribute__((weak)) load_backends_dynamically();                                                         \
  extern "C" BackendList __attribute__((weak)) load_backends_dynamically() {                                                        \
    static_assert(                                                                                                                  \
        std::is_base_of<Backend, type>::value,                                                                                      \
        "You have to call the setBackendType makro with your backend type like 'setBackendType(NullPointerBackend)'. Your backend " \
        "type has to inherit from Backend.'");                                                                                      \
    static_assert(ValidBackend<type>,                                                                                               \
                  "You have not defined 'static std::vector<Backend*> loadBackends()' in your Backend class (" #type                \
                  ". You need to define this method, as it will be called, to to create the Backend objects.");                     \
    std::vector<Backend*> backends = type::loadBackends();                                                                          \
    BackendList backendList;                                                                                                        \
    backendList.size = backends.size();                                                                                             \
    backendList.backends = new Backend*[backends.size()];                                                                           \
    for(unsigned int i = 0; i < backendList.size; i++) {                                                                            \
      backendList.backends[i] = backends[i];                                                                                        \
    }                                                                                                                               \
    return backendList;                                                                                                             \
  }

// Ways to declare weak symbol in gcc
//#pragma weak load_backends_dynamically
//_Pragma("weak load_backends_dynamically")
//__attribute__((weak))

#endif
