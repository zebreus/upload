#ifndef UPLOADER_HPP
#define UPLOADER_HPP

#include <vector>
#include <string>
#include <memory>
#include "settings.hpp"
#include "localtarget.hpp"
#include "quit.hpp"

class Uploader{
  std::vector<std::shared_ptr<Target>> targets;
  Settings settings;

public:
  Uploader(const Settings& settings);
  void uploadFile(const File& file);

private:
  void printAvailableTargets();
  void initializeTargets(const Settings& settings);
};

#endif
