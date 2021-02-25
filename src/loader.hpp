#ifndef LOADER_HPP
#define LOADER_HPP

#include <cstddef>
#include <iostream>
#include <iterator>
#include <memory>
#include <queue>
#include <string>
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

class Loader {
  struct FileIterator {
    using iterator_category = std::input_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = File;
    using pointer = File*;
    using reference = File&;

    FileIterator();
    FileIterator(FileIterator& other);
    ~FileIterator();
    bool operator==(const FileIterator& other);
    bool operator!=(const FileIterator& other);
    File& operator*();
    File* operator->();
    FileIterator operator++(int);
    FileIterator& operator++();

    Loader* myLoader;
    std::shared_ptr<File> file;
  };

  // All files/directories that are not yet loaded
  std::queue<std::filesystem::path> unprocessedFiles;
  std::vector<std::istream*> streams;
  Settings settings;

 public:
  Loader(const Settings& settings);
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

  // Ensure that a path exists and information about it can be optained
  std::filesystem::file_status ensureFileStatus(const std::filesystem::path& path);

  // Ensure that a file has read permissions
  bool isReadable(const std::filesystem::path& status);

  // Load a new path into unprocessed files. This may block for a while. Returns false, if every stream is finished.
  // Guarantees, that unprocessedFiles is not empty, if true is returned
  bool getUnprocessedPath();

  bool isDirectory(const std::filesystem::path& path);
  std::shared_ptr<File> createArchive(const std::vector<std::filesystem::path>& files, const std::string& name, bool directoryCreation);
};

#endif
