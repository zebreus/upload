#include "backend.hpp"
#include "backendloader.hpp"
#include "logger.hpp"

#ifdef __unix__

#include <dlfcn.h>

inline std::vector<std::shared_ptr<Backend>> loadBackendsFromFile(const std::filesystem::path& file) {
  void* handle = dlopen(file.c_str(), RTLD_LAZY);

  if(handle == NULL) {
    // Failed to open library
    std::string message(dlerror());
    logger.log(Logger::Info) << "Failed to load backend library " << file << "." << '\n';
    logger.log(Logger::Debug) << "This is the error message from your system: " << message << '\n';
    return std::vector<std::shared_ptr<Backend>>{};
  }
  BackendList (*load_backends_dynamically)();
  dlerror();
  load_backends_dynamically = reinterpret_cast<BackendList (*)()>(dlsym(handle, "load_backends_dynamically"));
  const char* errorMessage = dlerror();
  if(errorMessage != NULL) {
    // Library does not contain 'BackendList load_backends_dynamically()'
    logger.log(Logger::Info) << "The backend library " << file << " doesn't seem to be a valid upload backend." << '\n';
    logger.log(Logger::Debug)
        << "Library " << file << " does not contain 'BackendList load_backends_dynamically()'. Check, if " << file
        << " really is an upload backend. If you build the library yourself you can use the setBackendType() macro to "
           "generate 'BackendList load_backends_dynamically()' for you. "
        << '\n';
    logger.log(Logger::Debug) << "This is the error message from your system: " << errorMessage << '\n';
    return std::vector<std::shared_ptr<Backend>>{};
  }
  BackendList loadedBackendList = load_backends_dynamically();
  std::vector<std::shared_ptr<Backend>> loadedBackends;
  for(unsigned int i = 0; i < loadedBackendList.size; i++) {
    loadedBackends.push_back(std::shared_ptr<Backend>(loadedBackendList.backends[i]));
  }

  delete loadedBackendList.backends;

  return loadedBackends;
}

inline std::vector<std::filesystem::path> findLibraries() {
  std::string pluginDirectory;
#ifndef UPLOAD_PLUGIN_DIR
#warning "No plugin directory specified. You should define UPLOAD_PLUGIN_DIR as the directory where you want to load plugins from"
  logger.log(Logger::Debug) << "No plugin directory specified. You should define UPLOAD_PLUGIN_DIR as the directory where you want to load "
                               "plugins from, when building upload"
                            << '\n';
  return {};
#else
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
  std::vector<std::filesystem::path> backendLibraries;
  for(auto& directoryEntry : std::filesystem::directory_iterator(TOSTRING(UPLOAD_PLUGIN_DIR))) {
    if(directoryEntry.path().extension() == ".so") {
      backendLibraries.push_back(directoryEntry.path());
    }
  }
  return backendLibraries;
#endif
}

std::vector<std::shared_ptr<Backend>> loadBackends() {
  std::vector<std::shared_ptr<Backend>> backends;
  std::vector<std::filesystem::path> libraries = findLibraries();
  for(const std::filesystem::path& file : libraries) {
    std::vector<std::shared_ptr<Backend>> loaded = loadBackendsFromFile(file);
    backends.insert(backends.begin(), loaded.begin(), loaded.end());
  }
  return backends;
}

#else
#error "Dynamic backend loading is not yet implemented for your operating system. Currently only unix is supported. You could try building with STATIC_LOADER to disable dynamic backend loading."
#endif
