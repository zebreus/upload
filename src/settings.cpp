#include "settings.hpp"

#include "logger.hpp"

Settings::Settings(int argc, char** argv) {
  parseOptions(argc, argv);
}

Settings::Mode Settings::getMode() const {
  return mode;
}

Settings::ArchiveType Settings::getArchiveType() const {
  return archiveType;
}

Settings::HttpsSetting Settings::getHttpsSetting() const {
  return httpsSetting;
}

bool Settings::getPreserveName() const {
  return preserveName;
}

std::string Settings::getArchiveName() const {
  return archiveName;
}

std::vector<std::string> Settings::getRequestedBackends() const {
  return requestedBackends;
}

std::vector<std::string> Settings::getFiles() const {
  return files;
}

bool Settings::getDirectoryArchive() const {
  return directoryArchive;
}

BackendRequirements Settings::getBackendRequirements() const {
  BackendRequirements requirements;

  if(preserveName) {
    requirements.preserveName.reset(new bool(true));
  }

  switch(httpsSetting) {
    case HttpsSetting::Forbid:
      requirements.http.reset(new bool(true));
      requirements.https.reset(new bool(false));
      break;
    case HttpsSetting::Force:
      requirements.http.reset(new bool(false));
      requirements.https.reset(new bool(true));
      break;
    case HttpsSetting::Allow:
    default:
      break;
  }

  return requirements;
}

cxxopts::Options Settings::generateParser() {
  cxxopts::Options options("upload", "Upload files to the internet");
  // clang-format off
  options.add_options()
  ("h,help", "Displays this help screen")
  ("version", "Displays version information")
  ("v", "Increase verbosity")
  ("a,archive", "Pack all files and directories into an archive")
  ("i,individual", "Upload all files or directory individually")
  ("l,list", "List all available upload backends for the current request, ordered by preference")
  ("archive-type", "Sets the archive type", cxxopts::value<std::string>())
  ("d,directory-archive", "Put the contents of directories in a directory in the archive.")
  ("r,root-archive", "Put the contents of directories in the root of the archive.")
  ("t,backend", "Add a specific backend. If this option is used, the default order is discarded.", cxxopts::value<std::vector<std::string>>())
  ("p,preserve-name", "Ensure that the filenames are preserved.")
  ("s,ssl", "Ensure the use of https.")
  ("no-ssl", "Ensure the use of http.")
  ("n,name", "The name of the created archive in archive mode.", cxxopts::value<std::string>())
  ("file", "The files that will be uploaded.", cxxopts::value<std::vector<std::string>>())
  ;
  // clang-format on
  options.parse_positional({"file"});
  return options;
}

void Settings::parseOptions(int argc, char** argv) {
  try {
    cxxopts::Options options = generateParser();
    auto result = options.parse(argc, argv);

    if(result.count("help")) {
      logger.log(Logger::Print) << options.help() << std::endl;
      quit::success();
    }

    initializeLogger(result);
    mode = parseMode(result);
    files = parseFiles(result, mode);
    archiveType = parseArchiveType(result);
    httpsSetting = parseHttpsSetting(result);
    if(result.count("backend")) {
      requestedBackends = result["backend"].template as<std::vector<std::string>>();
    }
    archiveName = parseArchiveName(result, archiveType);
    preserveName = result.count("preserve-name");
    directoryArchive = parseDirectoryArchive(result, mode);
  } catch(const cxxopts::OptionException& e) {
    logger.log(Logger::Fatal) << e.what() << '\n';
    quit::invalidCliUsage();
  }
}

std::string Settings::getArchiveExtension(Settings::ArchiveType archiveType) {
  switch(archiveType) {
    case ArchiveType::Zip:
    default:
      return "zip";
  }
}

Settings::Mode Settings::parseMode(const auto& parseResult) {
  if(parseResult.count("archive") && parseResult.count("individual")) {
    logger.log(Logger::Fatal) << "You cannot set archive mode and individual mode." << '\n';
    quit::invalidCliUsage();
  }

  // Test for flags
  if(parseResult.count("list")) {
    return Mode::List;
  }
  if(parseResult.count("archive")) {
    return Mode::Archive;
  }
  if(parseResult.count("individual")) {
    return Mode::Individual;
  }

  // Default mode depends on the number of files
  if(parseResult.count("file") <= 1) {
    return Mode::Individual;
  } else {
    return Mode::Archive;
  }
}

Settings::ArchiveType Settings::parseArchiveType(const auto& parseResult) {
  if(parseResult.count("archive-type")) {
    std::string archiveType = parseResult["archive-type"].template as<std::string>();

    if(archiveType == "zip") {
      return ArchiveType::Zip;
    } else {
      logger.log(Logger::Fatal) << "You specified an invalid archive-type. The only possible value is 'zip'" << '\n';
      quit::invalidCliUsage();
    }
  }
  return defaultArchiveType;
}

Settings::HttpsSetting Settings::parseHttpsSetting(const auto& parseResult) {
  if(parseResult.count("ssl") && parseResult.count("no-ssl")) {
    logger.log(Logger::Fatal) << "You cannot have ssl and no-ssl, try leaving one away" << '\n';
    quit::invalidCliUsage();
  }

  if(parseResult.count("ssl")) {
    return HttpsSetting::Force;
  }

  if(parseResult.count("no-ssl")) {
    return HttpsSetting::Forbid;
  }

  return HttpsSetting::Allow;
}

std::vector<std::string> Settings::parseFiles(const auto& parseResult, Settings::Mode mode) {
  bool filesRequired = (mode == Mode::Archive || mode == Mode::Individual);
  if(parseResult.count("file") == 0) {
    if(filesRequired) {
      if(isInteractiveSession()) {
        logger.log(Logger::Fatal) << "You have to specify files to upload. Use upload like 'upload file.txt file2.txt'" << '\n';
        quit::invalidCliUsage();
      } else {
        return {"-"};
      }
    } else {
      return {};
    }
  }
  return parseResult["file"].template as<std::vector<std::string>>();
}

bool Settings::parseDirectoryArchive(const auto& parseResult, Settings::Mode mode) {
  bool settingRequired = (mode == Mode::Archive || mode == Mode::Individual);
  if(settingRequired) {
    if(parseResult.count("root-archive") && parseResult.count("directory-archive")) {
      logger.log(Logger::Fatal)
          << "You cannot have root-archive and directory-archive. Based on your mode the recommended and default setting is '"
          << (mode == Mode::Archive ? "--directory-archive" : "--root-archive") << "' ." << '\n';
      quit::invalidCliUsage();
    }

    if(parseResult.count("root-archive")) {
      return false;
    }

    if(parseResult.count("directory-archive")) {
      return true;
    }

    if(mode == Mode::Archive) {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

std::string Settings::parseArchiveName(const auto& parseResult, Settings::ArchiveType archiveType) {
  if(parseResult.count("name")) {
    std::string name = parseResult["name"].template as<std::string>();
    // TODO better check for valid names
    if(name == "") {
      logger.log(Logger::Fatal) << "You set an invalid archive name " << name
                                << " . At the moment I cannot make any recommendations for improvement." << '\n';
      quit::invalidCliUsage();
    }
    return name;
  }

  std::string name;

  unsigned int nameLength = 8;
  name.reserve(nameLength + 1);
  srand(time(NULL));
  for(unsigned int x = 0; x < nameLength; x++) {
    name.push_back(65 + (rand() % 2) * 32 + (rand() % 26));
  }

  name.push_back('.');
  name.append(getArchiveExtension(archiveType));
  return name;
}

void Settings::initializeLogger(const auto& parseResult) const {
  switch(parseResult.count("v")) {
    case 0:
      logger.setTopicState(Logger::Topic::Fatal, true);
      logger.setTopicState(Logger::Topic::Print, true);
      logger.setTopicState(Logger::Topic::Info, false);
      logger.setTopicState(Logger::Topic::Debug, false);
      logger.setTopicState(Logger::Topic::Url, true);
      break;
    case 1:
      logger.setTopicState(Logger::Topic::Fatal, true);
      logger.setTopicState(Logger::Topic::Print, true);
      logger.setTopicState(Logger::Topic::Info, true);
      logger.setTopicState(Logger::Topic::Debug, false);
      logger.setTopicState(Logger::Topic::Url, true);
      break;
    case 2:
    default:
      logger.setTopicState(Logger::Topic::Fatal, true);
      logger.setTopicState(Logger::Topic::Print, true);
      logger.setTopicState(Logger::Topic::Info, true);
      logger.setTopicState(Logger::Topic::Debug, true);
      logger.setTopicState(Logger::Topic::Url, true);
      break;
  }
}

#ifdef __unix__
bool Settings::isInteractiveSession() const {
  return isatty(fileno(stdin));
}
#elif defined(__windows__)
bool Settings::isInteractiveSession() const {
  return _isatty(_fileno(stdin));
}
#else
#error "Settings::isStdinInteractive is not implemented for your operating system. It is probably quite trivial to implement it yourself."
#endif
