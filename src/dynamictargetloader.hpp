#include "targetloader.hpp"
#include "target.hpp"
#include "logger.hpp"

#ifdef __unix__

#include <dlfcn.h>

std::vector<std::shared_ptr<Target>> loadTargetsFromFile(const std::filesystem::path& file){
  void* handle = dlopen(file.c_str(), RTLD_LAZY);
  
  if(handle == NULL){
    // Failed to open library
    std::string message(dlerror());
    logger.log(Logger::Info) << "Failed to load target library " << file << "." << '\n';
    logger.log(Logger::Debug) << "This is the error message from your system: " << message << '\n';
    return std::vector<std::shared_ptr<Target>>{};
  }
  TargetList (*load_targets_dynamically)();
  dlerror();
  load_targets_dynamically = (TargetList (*)())dlsym(handle, "load_targets_dynamically");
  const char* errorMessage = dlerror();
  if(errorMessage != NULL){
    // Library does not contain 'TargetList load_targets_dynamically()'
    logger.log(Logger::Info) << "The target library " << file << " doesn't seem to be a valid upload target." << '\n';
    logger.log(Logger::Debug) << "Library " << file << " does not contain 'TargetList load_targets_dynamically()'. Check, if " << file << " really is an upload target. If you build the library yourself you can use the setTargetType() macro to generate 'TargetList load_targets_dynamically()' for you. " << '\n';
    logger.log(Logger::Debug) << "This is the error message from your system: " << errorMessage << '\n';
    return std::vector<std::shared_ptr<Target>>{};
  }
  TargetList loadedTargetList = (TargetList)load_targets_dynamically();
  std::vector<std::shared_ptr<Target>> loadedTargets;
  for(int i = 0; i < loadedTargetList.size ; i++){
    loadedTargets.push_back(std::shared_ptr<Target>(loadedTargetList.targets[i]));
  }
  
  delete loadedTargetList.targets;

  return loadedTargets;
}

std::vector<std::filesystem::path> findLibraries(){
  std::string pluginDirectory;
#ifndef UPLOAD_PLUGIN_DIR
#warning "No plugin directory specified. You should define UPLOAD_PLUGIN_DIR as the directory where you want to load plugins from"
  logger.log(Logger::Debug) << "No plugin directory specified. You should define UPLOAD_PLUGIN_DIR as the directory where you want to load plugins from, when building upload" << '\n';
  return {};
#else
  #define STRINGIFY(x) #x
  #define TOSTRING(x) STRINGIFY(x)
  std::vector<std::filesystem::path> targetLibraries;
  for(auto& directoryEntry: std::filesystem::directory_iterator(TOSTRING(UPLOAD_PLUGIN_DIR))){
    if(directoryEntry.path().extension() == ".so"){
      targetLibraries.push_back(directoryEntry.path());
    }
  }
  return targetLibraries;
#endif
}

std::vector<std::shared_ptr<Target>> loadTargets(){
  std::vector<std::shared_ptr<Target>> targets;
  std::vector<std::filesystem::path> libraries = findLibraries();
  for(const std::filesystem::path& file : libraries){
    std::vector<std::shared_ptr<Target>> loaded = loadTargetsFromFile(file);
    targets.insert(targets.begin(), loaded.begin(), loaded.end());
  }
  return targets;
}

#else
#error "Dynamic target loading is not yet implemented for your operating system. Currently only unix is supported. You could try building with STATIC_LOADER to disable dynamic target loading." 
#endif
