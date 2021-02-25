#include "loader.hpp"

#include <zip_file.hpp>

Loader::Loader(const Settings& settings): settings(settings) {
  loadFilesFromSettings();
}

Loader::~Loader() {
  for(std::istream* stream : streams) {
    if(stream->good()) {
      logger.log(Logger::Topic::Debug) << "Closing good stream\n";
    }
    if(stream != &std::cin) {
      delete stream;
    }
  }
}

void Loader::loadFilesFromSettings() {
  if(settings.getMode() == Settings::Mode::Archive || settings.getMode() == Settings::Mode::Individual) {
    for(const std::string& fileName : settings.getFiles()) {
      if(fileName == "-") {
        streams.push_back(&std::cin);
      } else {
        loadPath(fileName);
      }
    }
  }
  return;
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
    unprocessedFiles.push(path);
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
        // TODO This check wont work
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
    unprocessedFiles.push(path);
  } else {
    logger.log(Logger::Fatal) << "You do not have the permission to access the directory " << path
                              << " . Contact your system administrator about that, or something. You can give everyone read "
                                 "permissions to that file with 'sudo chmod -R a+r "
                              << path << "'" << '\n';
    quit::failedReadingFiles();
  }
}

void Loader::loadFifoFile(const std::filesystem::path& path) {
  logger.log(Logger::Topic::Fatal) << "Fifo files are not yet implemented." << '\n';
  quit::failedReadingFiles();
}

void Loader::loadCharacterSpecialFile(const std::filesystem::path& path) {
  logger.log(Logger::Topic::Fatal) << "Character special files are not yet implemented." << '\n';
  quit::failedReadingFiles();
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

bool Loader::getUnprocessedPath() {
  return false;
}

std::shared_ptr<File> Loader::getNextFile() {
  switch(settings.getMode()) {
    default:
      logger.log(Logger::Topic::Debug) << "Unknown mode.";
    case Settings::Mode::List:
      break;
    case Settings::Mode::Individual: {
      if(unprocessedFiles.empty()) {
        if(!getUnprocessedPath()) {
          logger.log(Logger::Topic::Debug) << "All files loaded.";
          return std::shared_ptr<File>(nullptr);
        }
      }

      std::filesystem::path path = unprocessedFiles.front();
      unprocessedFiles.pop();
      if(std::filesystem::is_directory(path)) {
        return createArchive(std::vector<std::filesystem::path>{path}, path.filename(), settings.getDirectoryArchive());
      } else {
        return std::make_shared<File>(path);
      }
    }
    case Settings::Mode::Archive: {
      while(getUnprocessedPath()) {
      }
      if(unprocessedFiles.empty()) {
        return std::shared_ptr<File>(nullptr);
      }
      std::vector<std::filesystem::path> allPaths;
      while(!unprocessedFiles.empty()) {
        allPaths.push_back(unprocessedFiles.front());
        unprocessedFiles.pop();
      }
      return createArchive(allPaths, settings.getArchiveName(), settings.getDirectoryArchive());
    }
  }
  return std::shared_ptr<File>(nullptr);
}

Loader::FileIterator Loader::begin() {
  FileIterator begin = FileIterator();
  begin.myLoader = this;
  begin.file = getNextFile();
  return begin;
}

Loader::FileIterator Loader::end() {
  return FileIterator();
}

Loader::FileIterator::FileIterator() {}

Loader::FileIterator::FileIterator(FileIterator& other) {
  myLoader = other.myLoader;
  file = other.file;
}

Loader::FileIterator::~FileIterator() {}

bool Loader::FileIterator::operator==(const Loader::FileIterator& other) {
  if(file == nullptr) {
    if(other.file == nullptr) {
      return true;
    } else {
      return false;
    }
  }
  if(other.file == nullptr) {
    return false;
  }
  return file->getName() == other.file->getName();
}

bool Loader::FileIterator::operator!=(const Loader::FileIterator& other) {
  return !operator==(other);
}

File& Loader::FileIterator::operator*() {
  return *file;
}

File* Loader::FileIterator::operator->() {
  return file.get();
}

Loader::FileIterator Loader::FileIterator::operator++(int) {
  if(file != nullptr) {
    file = myLoader->getNextFile();
  }
  return *this;
}

Loader::FileIterator& Loader::FileIterator::operator++() {
  if(file != nullptr) {
    file = myLoader->getNextFile();
  }
  return *this;
}
