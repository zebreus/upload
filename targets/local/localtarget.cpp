#include "localtarget.hpp"



LocalTarget::LocalTarget(std::filesystem::path basePath): basePath(basePath){
}

std::string LocalTarget::getName() const{
  return name + basePath.string();
}

BackendFeatures LocalTarget::getSupportedFeatures() const{
  return supportedFeatures;
}

bool LocalTarget::staticSettingsCheck(BackendRequirements requiredFeatures) const{
  if((requiredFeatures & supportedFeatures) != requiredFeatures){
    //std::clog << "Not all requiredFeatures are supported" << requiredFeatures << " " << supportedFeatures << "\n";
    return false;
  }
  
  return true;
}

bool LocalTarget::staticFileCheck(BackendRequirements requiredFeatures, const File& file) const{
  std::string filename = file.getName();
  if( std::filesystem::path(file.getName()).remove_filename() != "" ){
    //std::clog << "Filename has to be only a filename\n";
    return false;
  }
  return true;
}

void LocalTarget::dynamicSettingsCheck(BackendRequirements requiredFeatures, std::function<void()> successCallback, std::function<void(std::string)> errorCallback, int timeoutMillis){
  if(validBasePath()){
    successCallback();
  }else{
    errorCallback("Invalid basePath");
  }
}

void LocalTarget::uploadFile(BackendRequirements requiredFeatures, const File& file, std::function<void(std::string)> successCallback, std::function<void(std::string)> errorCallback){
  std::filesystem::path targetFile(basePath);
  targetFile.append(file.getName());
  if(!fileCanBeCreated(targetFile)){
    errorCallback("Target file cannot be created");
    return;
  }
  
  std::ofstream fileStream(targetFile, std::ios::binary | std::ios::out);
  fileStream << file.getContent();
  fileStream.close();
  
  successCallback(targetFile.string());
}

bool LocalTarget::fileCanBeCreated(std::filesystem::path filePath){
  std::error_code error;
  std::filesystem::file_status fileStatus = std::filesystem::status(filePath, error);
  std::stringstream message;
  switch(fileStatus.type()){
    case std::filesystem::file_type::none:
      //std::clog << "Failed to get information about the file " << filePath << ".\n";
      if(error){
        //std::clog << error.message() << '\n';
      }
      return false;
    case std::filesystem::file_type::unknown:
      //std::clog << "Failed to determine to filetype of " << filePath << " . You should check, that you have permissions to access that file.\n";
      if(error){
        //std::clog << error.message() << '\n';
      }
      return false;
    default:
      return false;
    case std::filesystem::file_type::not_found:
      return true;
  }
}

bool LocalTarget::validBasePath(){
  std::error_code error;
  if(!std::filesystem::is_directory(basePath, error)){
    return false;
  }
#ifdef __unix__
  int accessResult = access(basePath.c_str(), R_OK);
  if(accessResult != 0){
    //std::clog << "You do not have the permission to create files in " << basePath << " .\n";
    return false;
  }
#endif
  return true;
}

std::vector<Target*> LocalTarget::loadTargets(){
  Target* myTarget = new LocalTarget("./beta");
  return std::vector<Target*>{myTarget};
}

setTargetType(LocalTarget)
