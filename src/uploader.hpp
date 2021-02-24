#ifndef UPLOADER_HPP
#define UPLOADER_HPP

#include <future>
#include <list>
#include <memory>
#include <stdexcept>
#include <string>
#include <target.hpp>
#include <vector>

#include "logger.hpp"
#include "quit.hpp"
#include "settings.hpp"
#include "targetloader.hpp"

class Uploader {
  std::list<std::shared_ptr<Target>> targets;
  std::vector<std::shared_ptr<Target>> checkedTargets;
  Settings settings;

 public:
  Uploader(const Settings& settings);
  std::string uploadFile(const File& file);

 private:
  std::string uploadFile(const File& file, std::shared_ptr<Target> target);
  void printAvailableTargets();
  void initializeTargets();
  void checkNextTarget();
  void checkNextTarget(std::promise<std::shared_ptr<Target>>& promise);
};

#endif
