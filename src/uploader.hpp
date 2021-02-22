#ifndef UPLOADER_HPP
#define UPLOADER_HPP

#include <vector>
#include <list>
#include <string>
#include <memory>
#include <stdexcept>
#include "settings.hpp"
#include <target.hpp>
#include "quit.hpp"
#include <future>
#include "targetloader.hpp"
#include "logger.hpp"

class Uploader{
  std::list<std::shared_ptr<Target>> targets;
  std::vector<std::shared_ptr<Target>> checkedTargets;
  Settings settings;

public:
  Uploader(const Settings& settings);
  std::string uploadFile(const File& file);

private:
  std::string uploadFile(const File& file, std::shared_ptr<Target> target);
  void printAvailableTargets();
  void initializeTargets(const Settings& settings);
  void checkNextTarget();
  void checkNextTarget(std::promise<std::shared_ptr<Target>>& promise);
};

#endif
