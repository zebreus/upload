#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <climits>
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
  bool continueUploading;

  static constexpr ArchiveType defaultArchiveType = ArchiveType::Zip;

 public:
  Settings(int argc, char** argv);
  [[nodiscard]] Mode getMode() const;
  [[nodiscard]] ArchiveType getArchiveType() const;
  [[nodiscard]] std::string getArchiveName() const;
  [[nodiscard]] std::vector<std::string> getRequestedBackends() const;
  [[nodiscard]] std::vector<std::string> getFiles() const;
  [[nodiscard]] bool getDirectoryArchive() const;
  [[nodiscard]] BackendRequirements getBackendRequirements() const;
  [[nodiscard]] bool getContinue() const;

 private:
  static cxxopts::Options generateParser();
  void parseOptions(int argc, char** argv);
  static std::string getArchiveExtension(const Settings::ArchiveType& archiveType);
  Mode parseMode(const auto& parseResult);
  ArchiveType parseArchiveType(const auto& parseResult);
  std::vector<std::string> parseFiles(const auto& parseResult, Settings::Mode mode);
  bool parseDirectoryArchive(const auto& parseResult, Settings::Mode mode);
  std::string parseArchiveName(const auto& parseResult, Settings::ArchiveType type);
  void initializeLogger(const auto& parseResult) const;
  BackendRequirements parseBackendRequirements(const auto& parseResult);

  [[nodiscard]] static bool isInteractiveSession();
  [[nodiscard]] static long long parseTimeString(const std::string& timeString);
  [[nodiscard]] static long parseSizeString(const std::string& timeString);
};

#endif
