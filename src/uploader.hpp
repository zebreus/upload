#ifndef UPLOADER_HPP
#define UPLOADER_HPP

#include <backend.hpp>
#include <future>
#include <list>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <queue>

#include "backendloader.hpp"
#include "logger.hpp"
#include "quit.hpp"
#include "settings.hpp"

class Uploader {

  //Lock the mutex, when accessing checkedBackends;
  std::mutex checkedBackendsMutex;
  std::queue<std::future<void>> backends;
  std::vector<std::shared_ptr<Backend>> checkedBackends;

  Settings settings;

 public:
  explicit Uploader(const Settings& settings);
  std::string uploadFile(const File& file);

 private:
  std::string uploadFile(const File& file, const std::shared_ptr<Backend>& backend);
  void printAvailableBackends();
  void initializeBackends();
  void checkBackend(const std::shared_ptr<Backend>& backend);
};

#endif
