#ifndef LOADER_HPP
#define LOADER_HPP

#include <condition_variable>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <memory>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include "file.hpp"
#include "logger.hpp"
#include "settings.hpp"

#ifdef __unix__
#include <unistd.h>
#endif

template<typename T>
concept FileProcessingFunction = requires(T function, const File& file) {
  function(file);
};

// TODO split this megaclass
class Loader {
  struct FileIterator {
    using iterator_category = std::input_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = File;
    using pointer = File*;
    using reference = File&;

    FileIterator(Loader* myLoader, std::shared_ptr<File> file);
    FileIterator(FileIterator& other);
    ~FileIterator();
    bool operator==(const FileIterator& other);
    bool operator!=(const FileIterator& other);
    File& operator*();
    File* operator->();
    FileIterator& operator++();

    Loader* myLoader;
    std::shared_ptr<File> file;
  };

  std::vector<std::jthread> threads;
  int threadCounter;

  // The unprocessed files mutex should be locked, when unprocessedFiles or threadCounter gets modified.
  std::condition_variable unprocessedFilesConditionVariable;
  std::mutex unprocessedFilesAccessMutex;

  // All files/directories that are not yet loaded
  std::queue<std::filesystem::path> unprocessedFiles;
  Settings settings;

 public:
  explicit Loader(const Settings& settings);
  ~Loader();
  std::shared_ptr<File> getNextFile();

  FileIterator begin();
  FileIterator end();

 private:
  void loadFilesFromSettings();

  // Loads a path into this object
  void loadPath(const std::filesystem::path& path);
  void loadRegularFile(const std::filesystem::path& path);
  void loadSymlink(std::filesystem::path path);
  void loadDirectory(const std::filesystem::path& path);
  void loadFifoFile(const std::filesystem::path& path);
  void loadCharacterSpecialFile(const std::filesystem::path& path);

  // Starts a thread that reads new filenames from the file at
  // Undefined behaviour, if path is not a readable file
  void startStreamThread(const std::filesystem::path& path);

  // Ensure that a path exists and information about it can be optained
  static std::filesystem::file_status ensureFileStatus(const std::filesystem::path& path);

  // Ensure that a file has read permissions
  static bool isReadable(const std::filesystem::path& status);

  // Wait until the next path is available and return it.
  // Throws std::runtime_error, when all paths have been read
  std::filesystem::path getUnprocessedPath();

  static std::shared_ptr<File> createArchive(const std::vector<std::filesystem::path>& files, const std::string& name, bool directoryCreation);
};

#endif
