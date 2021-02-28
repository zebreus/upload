#include "loader.hpp"

#include <zip_file.hpp>

Loader::Loader(const Settings& settings): settings(settings), threadCounter(0) {
  loadFilesFromSettings();
}

Loader::~Loader() = default;

void Loader::loadFilesFromSettings() {
  if(settings.getMode() == Settings::Mode::Archive || settings.getMode() == Settings::Mode::Individual) {
    for(const std::string& fileName : settings.getFiles()) {
      if(fileName == "-") {
        startStreamThread("-");
      } else {
        loadPath(fileName);
      }
    }
  }
}

void Loader::loadPath(const std::filesystem::path& path) {
  std::filesystem::file_status fileStatus = ensureFileStatus(path);

  switch(fileStatus.type()) {
    case std::filesystem::file_type::directory:
      loadDirectory(path);
      break;
    case std::filesystem::file_type::regular:
      loadRegularFile(path);
      break;
    case std::filesystem::file_type::symlink:
      loadSymlink(path);
      break;
    case std::filesystem::file_type::character:
      loadCharacterSpecialFile(path);
      break;
    case std::filesystem::file_type::fifo:
      loadFifoFile(path);
      break;
    case std::filesystem::file_type::block:
      logger.log(Logger::Topic::Fatal) << "Block special files are not yet implemented. You probably wanted to specify another file." << '\n';
      quit::failedReadingFiles();
    case std::filesystem::file_type::socket:
      logger.log(Logger::Topic::Fatal) << "Socket special files are not yet implemented. You probably wanted to specify another file." << '\n';
      quit::failedReadingFiles();
    case std::filesystem::file_type::unknown:
    case std::filesystem::file_type::none:
    case std::filesystem::file_type::not_found:
    default:
      logger.log(Logger::Fatal) << "This should not have happened, as the fileStatus for this switch should be checked before" << '\n';
      quit::failedReadingFiles();
  }
}

void Loader::loadRegularFile(const std::filesystem::path& path) {
  if(isReadable(path)) {
    {
      std::unique_lock<std::mutex> lock(unprocessedFilesAccessMutex);
      unprocessedFiles.push(path);
    }
    unprocessedFilesConditionVariable.notify_one();
  } else {
    logger.log(Logger::Fatal) << "You do not have the permission to access the file " << path
                              << " . Contact your system administrator about that, or something. You can give everyone read "
                                 "permissions to that file with 'sudo chmod a+r "
                              << path << "'" << '\n';
    quit::failedReadingFiles();
  }
}

void Loader::loadSymlink(std::filesystem::path path) {
  constexpr int maxRedirects = 16;
  int redirects = 0;
  do {
    try {
      if(isReadable(path)) {
        path = std::filesystem::read_symlink(path);
      } else {
        logger.log(Logger::Fatal) << "Failed to upload the target of the symlink " << path
                                  << ", because you do not have read permissions for the symlink." << '\n';
        quit::failedReadingFiles();
      }
      redirects++;
      if(redirects > maxRedirects) {
        logger.log(Logger::Fatal) << "Stopped reading symlink path " << path << ", because there were too many redirections." << '\n';
        quit::failedReadingFiles();
      }
    } catch(const std::filesystem::filesystem_error& error) {
      logger.log(Logger::Fatal) << "You tried to upload the symlink " << path << ", but it cannot be followed." << '\n';
      logger.log(Logger::Debug) << error.what() << '\n';
      quit::failedReadingFiles();
    }
  } while(ensureFileStatus(path).type() == std::filesystem::file_type::symlink);
  loadPath(path);
}

void Loader::loadDirectory(const std::filesystem::path& path) {
  if(isReadable(path)) {
    {
      std::unique_lock<std::mutex> lock(unprocessedFilesAccessMutex);
      unprocessedFiles.push(path);
    }
    unprocessedFilesConditionVariable.notify_one();
  } else {
    logger.log(Logger::Fatal) << "You do not have the permission to access the directory " << path
                              << " . Contact your system administrator about that, or something. You can give everyone read "
                                 "permissions to that file with 'sudo chmod -R a+r "
                              << path << "'" << '\n';
    quit::failedReadingFiles();
  }
}

void Loader::loadFifoFile(const std::filesystem::path& path) {
  if(isReadable(path)) {
    startStreamThread(path);
  } else {
    logger.log(Logger::Fatal) << "You do not have the permission to access the fifo file " << path
                              << " . Contact your system administrator about that, or something." << '\n';
    quit::failedReadingFiles();
  }
}

void Loader::loadCharacterSpecialFile(const std::filesystem::path& path) {
  if(isReadable(path)) {
    startStreamThread(path);
  } else {
    logger.log(Logger::Fatal) << "You do not have the permission to access the character special file " << path
                              << " . Contact your system administrator about that, or something." << '\n';
    quit::failedReadingFiles();
  }
}

void Loader::startStreamThread(const std::filesystem::path& path) {
  threads.emplace_back([this, path]() {
    {
      std::unique_lock<std::mutex> lock(unprocessedFilesAccessMutex);
      ++threadCounter;
    }
    std::istream* stream;
    if(path == "-") {
      stream = &std::cin;
    } else {
      stream = new std::ifstream(path);
    }
    while(stream->good()) {
      static constexpr int maxPathSize = 512;
      char buffer[maxPathSize];
      stream->getline(buffer, maxPathSize);
      logger.log(Logger::Debug) << "After  Good: " << stream->good() << "Bad: " << stream->bad() << "fail: " << stream->fail()
                                << "eof: " << stream->eof() << '\n';
      if(!stream->fail()) {
        if(buffer[0] != 0) {
          loadPath(std::string(buffer));
        }
      } else if(!stream->eof()) {
        logger.log(Logger::Fatal) << "Failed to read file from stream" << '\n';
        quit::failedReadingFiles();
      }
    }
    if(stream != &std::cin) {
      delete stream;
    }
    {
      std::unique_lock<std::mutex> lock(unprocessedFilesAccessMutex);
      --threadCounter;
    }
    unprocessedFilesConditionVariable.notify_one();
    logger.log(Logger::Debug) << "Stream finished" << '\n';
  });
}

std::filesystem::file_status Loader::ensureFileStatus(const std::filesystem::path& path) {
  try {
    std::filesystem::file_status fileStatus = std::filesystem::status(path);
    switch(fileStatus.type()) {
      case std::filesystem::file_type::none:
        logger.log(Logger::Fatal) << "Failed to get information about the file " << path << ". " << '\n';
        quit::failedReadingFiles();
      case std::filesystem::file_type::not_found:
        logger.log(Logger::Fatal) << path << " does not exist. Maybe check for typos." << '\n';
        quit::failedReadingFiles();
      case std::filesystem::file_type::unknown:
        logger.log(Logger::Fatal) << "Failed to determine to filetype of " << path
                                  << " . You should check, that you have permissions to access that file. " << '\n';
        quit::failedReadingFiles();
      case std::filesystem::file_type::symlink:
      case std::filesystem::file_type::block:
      case std::filesystem::file_type::character:
      case std::filesystem::file_type::fifo:
      case std::filesystem::file_type::socket:
      case std::filesystem::file_type::regular:
      case std::filesystem::file_type::directory:
      default:
        return fileStatus;
    }
  } catch(const std::filesystem::filesystem_error& error) {
    logger.log(Logger::Fatal) << "Failed to get information about the file " << path << ". Maybe the path is not formatted correctly?" << '\n';
    logger.log(Logger::Debug) << error.what() << '\n';
    quit::failedReadingFiles();
  }
}

bool Loader::isReadable(const std::filesystem::path& path) {
#ifdef __unix__
  int accessResult = access(path.c_str(), R_OK);
  if(accessResult != 0) {
    return false;
  }
#endif
  return true;
}

std::shared_ptr<File> Loader::createArchive(const std::vector<std::filesystem::path>& files, const std::string& name, bool directoryCreation) {
  miniz_cpp::zip_file file;
  logger.log(Logger::Debug) << "Creating archive " << name << ". " << '\n';
  for(const std::filesystem::path& path : files) {
    if(std::filesystem::is_directory(path)) {
      std::error_code error;
      std::filesystem::path canonicalPath = std::filesystem::canonical(path, error);
      if(error) {
        logger.log(Logger::Fatal) << "Failed to create canonical path of " << path << " . This should not happen." << '\n';
        quit::failedReadingFiles();
      }

      std::filesystem::path basePath = canonicalPath;
      if(directoryCreation) {
        basePath.remove_filename();
      }

      for(auto& p : std::filesystem::recursive_directory_iterator(canonicalPath)) {
        // Absolute path in filesystem
        std::filesystem::file_status status = ensureFileStatus(p.path());
        if(status.type() != std::filesystem::file_type::regular && status.type() != std::filesystem::file_type::directory) {
          logger.log(Logger::Fatal) << "Only regular files and directories can be archived. You tried to archive " << path
                                    << ", which is neither." << '\n';
          quit::failedReadingFiles();
        }
        std::filesystem::path realPath = std::filesystem::canonical(p.path(), error);
        if(error) {
          logger.log(Logger::Fatal) << "Failed to create canonical path of " << path << " . This should not happen." << '\n';
          quit::failedReadingFiles();
        }

        // Path in archive
        std::filesystem::path resultPath = std::filesystem::relative(realPath, basePath);

        if(std::filesystem::is_directory(realPath)) {
          std::string resultDirPath = resultPath.string() + "/";
          file.writestr(resultDirPath, "");
        } else {
          File f(realPath);
          file.writestr(resultPath, f.getContent());
        }
      }

    } else {
      File f(path);
      file.writestr(f.getName(), f.getContent());
    }
  }

  std::stringstream archiveStream;
  file.save(archiveStream);

  return std::make_shared<File>(name, archiveStream.str());
}

std::filesystem::path Loader::getUnprocessedPath() {
  // Read until a real path is read
  std::unique_lock<std::mutex> lock(unprocessedFilesAccessMutex);
  unprocessedFilesConditionVariable.wait(lock, [this]() {
    return (!unprocessedFiles.empty()) || (threadCounter == 0);
  });
  if(!unprocessedFiles.empty()) {
    std::filesystem::path path = unprocessedFiles.front();
    unprocessedFiles.pop();
    return path;
  } else if(threadCounter == 0) {
    throw std::runtime_error("No more files");
  }

  logger.log(Logger::Debug) << "Condition variable stopped waiting, but no condition was fulfilled. This should not happen." << '\n';
  throw std::runtime_error("Something went wrong");
}

std::shared_ptr<File> Loader::getNextFile() {
  switch(settings.getMode()) {
    default:
      logger.log(Logger::Topic::Debug) << "Unknown mode.";
    case Settings::Mode::List:
      break;
    case Settings::Mode::Individual: {
      try {
        std::filesystem::path path = getUnprocessedPath();
        if(std::filesystem::is_directory(path)) {
          return createArchive(std::vector<std::filesystem::path>{path}, path.filename(), settings.getDirectoryArchive());
        } else {
          return std::make_shared<File>(path);
        }
      } catch(const std::runtime_error& error) {
        logger.log(Logger::Topic::Debug) << "All files loaded.";
        return std::shared_ptr<File>(nullptr);
      }
    }
    case Settings::Mode::Archive: {
      std::vector<std::filesystem::path> allPaths;
      try {
        while(true) {
          allPaths.push_back(getUnprocessedPath());
        }
      } catch(const std::runtime_error& error) {
      }

      if(allPaths.empty()) {
        return std::shared_ptr<File>(nullptr);
      }
      return createArchive(allPaths, settings.getArchiveName(), settings.getDirectoryArchive());
    }
  }
  return std::shared_ptr<File>(nullptr);
}

Loader::FileIterator Loader::begin() {
  return FileIterator(this, getNextFile());
}

Loader::FileIterator Loader::end() {
  return FileIterator(this, nullptr);
}

Loader::FileIterator::FileIterator(Loader* myLoader, std::shared_ptr<File> file): myLoader(myLoader), file(std::move(file)) {}

Loader::FileIterator::FileIterator(FileIterator& other) {
  myLoader = other.myLoader;
  file = other.file;
}

Loader::FileIterator::~FileIterator() = default;

bool Loader::FileIterator::operator==(const Loader::FileIterator& other) {
  if(file == other.file) {
    return true;
  }
  if(file == nullptr || other.file == nullptr) {
    return false;
  }
  return file->getName() == other.file->getName();
}

File& Loader::FileIterator::operator*() {
  return *file;
}

File* Loader::FileIterator::operator->() {
  return file.get();
}

Loader::FileIterator& Loader::FileIterator::operator++() {
  if(file != nullptr) {
    file = myLoader->getNextFile();
  }
  return *this;
}
