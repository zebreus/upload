#ifndef UPLOADER_HPP
#define UPLOADER_HPP

#include <backend.hpp>
#include <future>
#include <list>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "backendloader.hpp"
#include "logger.hpp"
#include "quit.hpp"
#include "settings.hpp"

class Uploader {
  std::list<std::shared_ptr<Backend>> backends;
  std::vector<std::shared_ptr<Backend>> checkedBackends;
  Settings settings;

 public:
  explicit Uploader(const Settings& settings);
  std::string uploadFile(const File& file);

 private:
  std::string uploadFile(const File& file, const std::shared_ptr<Backend>& backend);
  void printAvailableBackends();
  void initializeBackends();
  bool checkNextBackend();
  void checkNextBackend(std::promise<std::shared_ptr<Backend>>& promise);
};

#endif
