#include "localtarget.hpp"

LocalTarget::LocalTarget(std::filesystem::path basePath): basePath(basePath){
}

std::string LocalTarget::getName() const{
  return name + basePath.string();
}

BackendFeatures LocalTarget::getSupportedFeatures() const{
  return supportedFeatures;
}

bool LocalTarget::staticCheck(BackendFeatures requiredFeatures) const{
  if((requiredFeatures & supportedFeatures) != requiredFeatures){
    std::clog << "Not all requiredFeatures are supported" << requiredFeatures << " " << supportedFeatures << "\n";
    return false;
  }
  
  return true;
}

void LocalTarget::dynamicCheck(BackendFeatures requiredFeatures, const File& file, void (*successCallback)(), void (*errorCallback)(std::string), int timeoutMillis){
  // For now assume, that all required features exit
  std::string filename = file.getName();
  if( std::filesystem::path(file.getName()).remove_filename() != "" ){
    std::clog << "Filename has to be only a filename\n";
    errorCallback("Invalid filename");
  }
  
  successCallback();
}

void LocalTarget::uploadFile(BackendFeatures requiredFeatures, const File& file, void (*successCallback)(std::string), void (*errorCallback)(std::string)){
  std::filesystem::path targetFile(basePath);
  targetFile.append(file.getName());
  if(!fileCanBeCreated(targetFile)){
    errorCallback("Target file cannot be created");
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
      std::clog << "Failed to get information about the file " << filePath << ".\n";
      if(error){
        std::clog << error.message() << '\n';
      }
      return false;
    case std::filesystem::file_type::unknown:
      std::clog << "Failed to determine to filetype of " << filePath << " . You should check, that you have permissions to access that file.\n";
      if(error){
        std::clog << error.message() << '\n';
      }
      return false;
    default:
      message << "File " << filePath << " already exists.";
      return false;
    case std::filesystem::file_type::not_found:
#ifdef __unix__
      {
      int accessResult = access(basePath.c_str(), R_OK);
      if(accessResult != 0){
        std::clog << "You do not have the permission to create files in " << basePath << " .\n";
        return false;
      }
      }
#endif
      return true;
  }
}
