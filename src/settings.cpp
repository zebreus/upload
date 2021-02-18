#include "settings.hpp"

Settings::Settings(int argc, char** argv){
  
}

Settings::Mode Settings::getMode() const{
  return mode;
}

Settings::VerbosityLevel Settings::getVerbosity() const{
  return verbosity;
}

Settings::ArchiveType Settings::getArchiveType() const{
  return archiveType;
}

Settings::HttpsSetting Settings::getHttpsSetting() const{
  return httpsSetting;
}

bool Settings::getPreserveName() const{
  return preserveName;
}

std::string Settings::getArchiveName() const{
  return archiveName;
}

std::vector<std::string> Settings::getRequestedTargets() const{
  return requestedTargets;
}

std::vector<std::string> Settings::getFiles() const{
  return files;
}
