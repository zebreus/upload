#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <cxxopts.hpp>
#include <iostream>
#include <string>
#include <vector>
#include "quit.hpp"

class Settings{
public:
  enum Mode{
    Individual,
    Archive,
    List
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
  
  static constexpr ArchiveType defaultArchiveType = ArchiveType::Zip;
  static constexpr HttpsSetting defaultHttpsSetting = HttpsSetting::Allow;
  static constexpr bool defaultPreserveName = false;

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

private:
  cxxopts::Options generateParser();
  void parseOptions(int argc, char** argv);
  std::string generateArchiveName();
  Mode parseMode(const auto& parseResult);
  VerbosityLevel parseVerbosity(const auto& parseResult);
  ArchiveType parseArchiveType(const auto& parseResult);
  HttpsSetting parseHttpsSetting(const auto& parseResult);
  std::vector<std::string> parseFiles(const auto& parseResult, Settings::Mode mode);
};

#endif
