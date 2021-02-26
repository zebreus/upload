#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <limits.h>

#include <cxxopts.hpp>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "backend.hpp"
#include "backendrequirements.hpp"
#include "quit.hpp"

#ifdef __unix__
#include <unistd.h>
#endif

class Settings {
 public:
  enum Mode { Individual, Archive, List };
  enum ArchiveType { Zip };

 private:
  Mode mode;
  ArchiveType archiveType;
  std::string archiveName;
  std::vector<std::string> requestedBackends;
  std::vector<std::string> files;
  bool directoryArchive;
  BackendRequirements requirements;

  static constexpr ArchiveType defaultArchiveType = ArchiveType::Zip;

 public:
  Settings(int argc, char** argv);
  Mode getMode() const;
  ArchiveType getArchiveType() const;
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
  std::vector<std::string> parseFiles(const auto& parseResult, Settings::Mode mode);
  bool parseDirectoryArchive(const auto& parseResult, Settings::Mode mode);
  std::string parseArchiveName(const auto& parseResult, Settings::ArchiveType type);
  void initializeLogger(const auto& parseResult) const;
  BackendRequirements parseBackendRequirements(const auto& parseResult);

  bool isInteractiveSession() const;
  long long parseTimeString(const std::string& timeString);
  long parseSizeString(const std::string& timeString);
};

#endif
