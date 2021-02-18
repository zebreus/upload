#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <cxxopts.hpp>
#include <string>
#include <vector>

class Settings{
public:
  enum Mode{
    Individual,
    Archive,
    List,
    Help
  };
  enum VerbosityLevel{
    Silent,
    Default,
    Verbose,
    Debug
  };
  enum ArchiveType{
    Zip
  };
  enum HttpsSetting{
    Forbid,
    Allow,
    Force
  };
  
private:

  Mode mode;
  VerbosityLevel verbosity;
  ArchiveType archiveType;
  HttpsSetting httpsSetting;
  bool preserveName;
  std::string archiveName;
  std::vector<std::string> requestedTargets;
  std::vector<std::string> files;

public:
  Settings(int argc, char** argv);
  Mode getMode() const;
  VerbosityLevel getVerbosity() const;
  ArchiveType getArchiveType() const;
  HttpsSetting getHttpsSetting() const;
  bool getPreserveName() const;
  std::string getArchiveName() const;
  std::vector<std::string> getRequestedTargets() const;
  std::vector<std::string> getFiles() const;
};

#endif
