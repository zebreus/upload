#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <cxxopts.hpp>
#include <iostream>
#include <string>
#include <vector>

#include "backend.hpp"
#include "backendrequirements.hpp"
#include "quit.hpp"

class Settings {
 public:
  enum Mode { Individual, Archive, List };
  enum ArchiveType { Zip };
  enum HttpsSetting { Forbid, Allow, Force };

 private:
  Mode mode;
  ArchiveType archiveType;
  HttpsSetting httpsSetting;
  bool preserveName;
  std::string archiveName;
  std::vector<std::string> requestedBackends;
  std::vector<std::string> files;
  bool directoryArchive;

  static constexpr ArchiveType defaultArchiveType = ArchiveType::Zip;
  static constexpr HttpsSetting defaultHttpsSetting = HttpsSetting::Allow;
  static constexpr bool defaultPreserveName = false;

 public:
  Settings(int argc, char** argv);
  Mode getMode() const;
  ArchiveType getArchiveType() const;
  HttpsSetting getHttpsSetting() const;
  bool getPreserveName() const;
  std::string getArchiveName() const;
  std::vector<std::string> getRequestedBackends() const;
  std::vector<std::string> getFiles() const;
  bool getDirectoryArchive() const;
  BackendRequirements getBackendRequirements() const;

 private:
  cxxopts::Options generateParser();
  void parseOptions(int argc, char** argv);
  std::string getArchiveExtension(const Settings::ArchiveType archiveType);
  Mode parseMode(const auto& parseResult);
  ArchiveType parseArchiveType(const auto& parseResult);
  HttpsSetting parseHttpsSetting(const auto& parseResult);
  std::vector<std::string> parseFiles(const auto& parseResult, Settings::Mode mode);
  bool parseDirectoryArchive(const auto& parseResult, Settings::Mode mode);
  std::string parseArchiveName(const auto& parseResult, Settings::ArchiveType type);
  void initializeLogger(const auto& parseResult) const;
};

#endif
