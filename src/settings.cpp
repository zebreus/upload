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
  return requirements;
}

bool Settings::getContinueLoading() const {
  return continueUploading;
}
bool Settings::getContinueUploading() const {
  return continueUploading;
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
  ("file", "The files that will be uploaded.", cxxopts::value<std::vector<std::string>>(), "FILE")
  ;
  options.add_options("Backend selection")
  ("b,backend", "Add a specific backend. If this option is used, the default order is discarded.", cxxopts::value<std::vector<std::string>>())
  ("p,preserve-name", "Ensure that the filenames are preserved.")
  ("no-preserve-name", "Ensure that the filenames are not preserved.")
  ("s,ssl", "Ensure the use of https.")
  ("no-ssl", "Ensure the use of http.")
  ("min-retention", "Make sure the file is kept longer than TIME.", cxxopts::value<std::string>(), "TIME")
  ("max-retention", "Make sure the file is not kept longer than TIME.", cxxopts::value<std::string>(), "TIME")
  ("min-size", "Only use backend that accept files bigger than BYTES.", cxxopts::value<std::string>(), "BYTES")
  ("autodelete", "Delete the file after NUM downloads.", cxxopts::value<int>()->implicit_value("1"), "NUM")
  ("min-random-part", "Each url has a random part longer than NUM characters", cxxopts::value<int>(), "NUM")
  ("max-random-part", "Each url has a random part shorter than NUM characters", cxxopts::value<int>(), "NUM")
  ("max-url-length", "Each generated url will be shorter than NUM characters.", cxxopts::value<int>(), "NUM")
  ;
  options.add_options("Mode independent")
  ("archive-type", "Sets the archive type", cxxopts::value<std::string>()->default_value("1"), "TYPE")
  ("r,root-archive", "Put the contents of directories in the root of the archive.")
  ("d,directory-archive", "Put the contents of directories in a directory in the archive.")
  ("c,continue", "Do not fail if opening or uploading a file failed.")
  ("continue-file", "Do not fail if opening a file failed.")
  ("continue-upload", "Do not fail if uploading a file failed.")
  ;
  options.add_options("Individual mode")
  ;
  options.add_options("Archive mode")
  ("n,name", "The name of the created archive in archive mode.", cxxopts::value<std::string>())
  ;
  // clang-format on
  options.parse_positional({"file"});
  options.positional_help("[FILE]...");
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
    if(result.count("backend")) {
      requestedBackends = result["backend"].template as<std::vector<std::string>>();
    }
    archiveName = parseArchiveName(result, archiveType);
    directoryArchive = parseDirectoryArchive(result, mode);
    requirements = parseBackendRequirements(result);
    parseContinue(result);
  } catch(const cxxopts::OptionException& e) {
    logger.log(Logger::Fatal) << e.what() << '\n';
    quit::invalidCliUsage();
  }
}

std::string Settings::getArchiveExtension(const Settings::ArchiveType& archiveType) {
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
    std::string archiveTypeString = parseResult["archive-type"].template as<std::string>();

    if(archiveTypeString == "zip") {
      return ArchiveType::Zip;
    } else {
      logger.log(Logger::Fatal) << "You specified an invalid archive-type. The only possible value is 'zip'" << '\n';
      quit::invalidCliUsage();
    }
  }
  return defaultArchiveType;
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
    if(name.empty()) {
      logger.log(Logger::Fatal) << "You set an invalid archive name " << name
                                << " . At the moment I cannot make any recommendations for improvement." << '\n';
      quit::invalidCliUsage();
    }
    return name;
  }

  std::string name;

  unsigned int nameLength = 8;
  name.reserve(nameLength + 1);
  srand(time(nullptr));
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

void Settings::parseContinue(const auto& parseResult) {
  continueLoading = false;
  continueUploading = false;

  if(parseResult.count("continue")){
    continueLoading = true;
    continueUploading = true;
  }
  if(parseResult.count("continue-file")){
    continueLoading = true;
  }
  if(parseResult.count("continue-upload")){
    continueUploading = true;
  }

  if(parseResult.count("continue-file") && parseResult.count("continue-upload")){
    logger.log(Logger::Info) << "You set '--continue-file' and '--continue-upload'. You can achieve the same effect, by just setting '--continue'." << '\n';
  }
}

BackendRequirements Settings::parseBackendRequirements(const auto& parseResult) {
  BackendRequirements requirements;

  if(parseResult.count("preserve-name") && parseResult.count("no-preserve-name")) {
    logger.log(Logger::Fatal) << "You set an '--preserve-name' and '--no-preserve-name' together. If you want to use hosts that preserve "
                                 "your name and those that do not, just leave both options away."
                              << '\n';
    quit::invalidCliUsage();
  }
  if(parseResult.count("preserve-name")) {
    requirements.preserveName.reset(new bool(true));
  }
  if(parseResult.count("no-preserve-name")) {
    requirements.preserveName.reset(new bool(false));
  }

  if(parseResult.count("ssl") && parseResult.count("no-ssl")) {
    logger.log(Logger::Fatal) << "You cannot have ssl and no-ssl, try leaving one away" << '\n';
    quit::invalidCliUsage();
  }

  if(parseResult.count("ssl")) {
    requirements.https.reset(new bool(true));
  }
  if(parseResult.count("no-ssl")) {
    requirements.http.reset(new bool(true));
  }

  if(parseResult.count("min-size")) {
    requirements.minSize.reset();
  }

  if(parseResult.count("min-retention")) {
    std::string minRetentionString = parseResult["min-retention"].template as<std::string>();
    long long minRetention = parseTimeString(minRetentionString);
    requirements.minRetention.reset(new long long(minRetention));
  }

  if(parseResult.count("max-retention")) {
    std::string maxRetentionString = parseResult["max-retention"].template as<std::string>();
    long long maxRetention = parseTimeString(maxRetentionString);
    requirements.maxRetention.reset(new long long(maxRetention));
  }

  if(requirements.maxRetention != nullptr && requirements.minRetention != nullptr) {
    if(*requirements.maxRetention < *requirements.minRetention) {
      logger.log(Logger::Fatal) << "You specified a maximum retention period of " << *requirements.maxRetention
                                << "ms, but that is smaller than your minimum rention period of " << *requirements.minRetention
                                << "ms. You could increase your minimum retention period." << '\n';
      quit::invalidCliUsage();
    }
  }

  if(parseResult.count("min-random-part")) {
    requirements.minRandomPart.reset((new long(parseResult["min-random-part"].template as<int>())));
  }

  if(parseResult.count("max-random-part")) {
    requirements.maxRandomPart.reset(new long(parseResult["max-random-part"].template as<int>()));
  }

  if(requirements.maxRandomPart != nullptr && requirements.minRandomPart != nullptr) {
    if(*requirements.maxRandomPart < *requirements.minRandomPart) {
      logger.log(Logger::Fatal) << "You specified a maximum random part of " << *requirements.maxRandomPart
                                << " characters, but that is smaller than your maximum random part of " << *requirements.minRandomPart
                                << " characters. You could increase your minimum random part." << '\n';
      quit::invalidCliUsage();
    }
  }

  if(parseResult.count("autodelete")) {
    requirements.maxDownloads.reset(new long(parseResult["autodelete"].template as<int>()));
  }

  if(parseResult.count("min-size")) {
    std::string minSizeString = parseResult["min-size"].template as<std::string>();
    unsigned long minSize = parseSizeString(minSizeString);
    requirements.minSize.reset(new unsigned long(minSize));
  }

  if(parseResult.count("max-url-length")) {
    int maxUrlLength = parseResult["max-url-length"].template as<int>();
    if(maxUrlLength < 11) {
      logger.log(Logger::Fatal) << "You specified a maximum url length of less than 11 characters (" << maxUrlLength
                                << "). That does not seem logical, as the shortest url possible is only 11 characters. http:/x.xx/" << '\n';
      quit::invalidCliUsage();
    }
    if(maxUrlLength < 14) {
      logger.log(Logger::Info) << "You specified a maximum url length of less than 10 characters (" << maxUrlLength
                               << "). This is not an error, but there probably are no backends with urls that short." << '\n';
    }
    requirements.maxUrlLength.reset(new long(maxUrlLength));
  }

  return requirements;
}

long long Settings::parseTimeString(const std::string& timeString) {
  size_t suffixStart = timeString.find_first_not_of("0123456789");
  if(suffixStart == std::string::npos) {
    suffixStart = timeString.size();
  }
  std::string numberString = timeString.substr(0, suffixStart);
  std::string suffixString = timeString.substr(suffixStart);
  try {
    long long number = std::stoll(numberString);
    if(suffixString.size() <= 1) {
      switch(suffixString.c_str()[0]) {
        case 'd':
        case 'D':
          number *= 24;
        case 'h':
        case 'H':
          number *= 60;
        case 'm':
        case 'M':
          number *= 60;
        case 's':
        case 'S':
          number *= 1000;
        case 0:
          return number;
        default:
          break;
      }
    }
    logger.log(Logger::Fatal) << "Your time value " << timeString
                              << " has an invalid suffix. It should be a positive integer suffixed by the unit of time ('s'econds "
                                 ",'m'inutes ,'h'ours or 'd'ays). If you do not add a suffix the number is interpreted as milliseconds."
                              << '\n';
    quit::invalidCliUsage();

  } catch(const std::invalid_argument& error) {
    logger.log(Logger::Fatal)
        << "Your time value " << timeString
        << " does not seem to start with a number. It should be a positive integer suffixed by the unit of time ('s'econds ,'m'inutes "
           ",'h'ours or 'd'ays). If you do not add a suffix the number is interpreted as milliseconds."
        << '\n';
    quit::invalidCliUsage();
  } catch(const std::out_of_range& error) {
    logger.log(Logger::Fatal) << "The number part of your time value " << timeString << " is too big, the maximum is " << LLONG_MAX
                              << ". It should be a positive integer suffixed by the unit of time ('s'econds ,'m'inutes ,'h'ours or "
                                 "'d'ays). If you do not add a suffix the number is interpreted as milliseconds."
                              << '\n';
    quit::invalidCliUsage();
  }
}

long Settings::parseSizeString(const std::string& sizeString) {
  size_t suffixStart = sizeString.find_first_not_of("0123456789");
  if(suffixStart == std::string::npos) {
    suffixStart = sizeString.size();
  }
  std::string numberString = sizeString.substr(0, suffixStart);
  std::string suffixString = sizeString.substr(suffixStart);
  for(char& i : suffixString) {
    i = std::tolower(i);
  }
  try {
    long number = std::stol(numberString);
    int multiplier = 1;
    if(suffixString.empty()) {
      multiplier = 1;
    } else if(suffixString.size() == 2 && suffixString[1] == 'b') {
      multiplier = 1000;
    } else if(suffixString.size() == 1 || (suffixString.size() == 3 && suffixString[1] == 'i' && suffixString[1] == 'b')) {
      multiplier = 1024;
    } else {
      logger.log(Logger::Fatal) << "Your size value " << sizeString
                                << " has an invalid suffix. It should be a positive integer suffixed by the unit of size (for example K, "
                                   "KB or KiB). If you do not add a suffix the number is interpreted as bytes."
                                << '\n';
      quit::invalidCliUsage();
    }

    switch(suffixString.c_str()[0]) {
      case 'g':
        number *= multiplier;
      case 'm':
        number *= multiplier;
      case 'k':
        number *= multiplier;
      case 0:
        return number;
      default:
        logger.log(Logger::Fatal)
            << "Your size value " << sizeString
            << " has an invalid suffix. It should be a positive integer suffixed by the unit of size (for example K, KB or KiB). If you do "
               "not add a suffix the number is interpreted as bytes. Currently only sizes with Kilo, Mega or Gigabyte are supported"
            << '\n';
        quit::invalidCliUsage();
    }

  } catch(const std::invalid_argument& error) {
    logger.log(Logger::Fatal) << "Your size value " << sizeString
                              << " does not seem to start with a number. It should be a positive integer suffixed by the unit of size (for "
                                 "example K, KB or KiB). If you do not add a suffix the number is interpreted as bytes."
                              << '\n';
    quit::invalidCliUsage();
  } catch(const std::out_of_range& error) {
    logger.log(Logger::Fatal) << "The number part of your size value " << sizeString << " is too big, the maximum is " << LONG_MAX
                              << ". It should be a positive integer suffixed by the unit of size (for example K, KB or KiB). If you do not "
                                 "add a suffix the number is interpreted as bytes."
                              << '\n';
    quit::invalidCliUsage();
  }
}

#ifdef __unix__
bool Settings::isInteractiveSession() {
  return isatty(fileno(stdin));
}
#elif defined(__windows__)
bool Settings::isInteractiveSession() {
  return _isatty(_fileno(stdin));
}
#else
#error "Settings::isStdinInteractive is not implemented for your operating system. It is probably quite trivial to implement it yourself."
#endif
