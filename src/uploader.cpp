#include "uploader.hpp"

Uploader::Uploader(const Settings& settings): settings(settings){
  initializeTargets(settings);
  if(settings.getMode() == Settings::Mode::List){
    printAvailableTargets();
    quit::success();
  }
}

void Uploader::uploadFile(const File& file){
  std::clog << "Upload not implemented\n";
}

void Uploader::printAvailableTargets(){
  for(const std::shared_ptr<Target>& target: targets){
    std::cout << target->getName() << '\n';
  }
  std::cout << std::flush;
}

void Uploader::initializeTargets(const Settings& settings){
  for(std::string testPath : {"./alpha", "./beta", "./gamma"}){
    std::shared_ptr<Target> target(new LocalTarget(testPath));
    if(target->staticCheck(settings.getRequiredFeatures())){
      targets.push_back(target);
    }
  }
}
