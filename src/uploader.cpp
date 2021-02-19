#include "uploader.hpp"

Uploader::Uploader(const Settings& settings): settings(settings){
  initializeTargets(settings);
  if(settings.getMode() == Settings::Mode::List){
    printAvailableTargets();
    quit::success();
  }
}

std::string Uploader::uploadFile(const File& file){  
  for(const std::shared_ptr<Target>& target: checkedTargets){
    try{
      return uploadFile(file, target);
    }catch(std::exception e){
      //std::clog << "Error" << e.what() << '\n';
    }
  }
  
  while(true){
    checkNextTarget();
    try{
      return uploadFile(file, checkedTargets.back());
    }catch(std::exception e){
      //std::clog << "Error" << e.what() << '\n';
    }
  }
}

std::string Uploader::uploadFile(const File& file, std::shared_ptr<Target> target){
  std::promise<std::string> urlPromise;
  target->uploadFile(settings.getRequiredFeatures(), file, [this, &urlPromise](std::string url){
    try{
      urlPromise.set_value(url);
    }catch(std::exception e){
      quit::unexpectedFailure("Failed to set urlPromise value. If your are compiling this code yourself, you probably forgot to enable threads . Try '-pthread'.");
    }
  },[&urlPromise](std::string message){
    urlPromise.set_exception(std::make_exception_ptr(std::runtime_error("lool")));
  });
  
  return urlPromise.get_future().get();
}

void Uploader::printAvailableTargets(){
  for(const std::shared_ptr<Target>& target: checkedTargets){
    std::cout << target->getName() << '\n';
  }
  for(const std::shared_ptr<Target>& target: targets){
    std::cout << target->getName() << '\n';
  }
  std::cout << std::flush;
}

void Uploader::initializeTargets(const Settings& settings){
  for(std::string testPath : {"./alpha", "./beta", "./gamma"}){
    std::shared_ptr<Target> target(new LocalTarget(testPath));
    if(target->staticSettingsCheck(settings.getRequiredFeatures())){
      targets.push_back(target);
    }
  }
}

void Uploader::checkNextTarget(){
  std::promise<std::shared_ptr<Target>> nextTargetPromise;
  std::future<std::shared_ptr<Target>> future = nextTargetPromise.get_future();;
  checkNextTarget(nextTargetPromise);
  std::shared_ptr<Target> nextTarget = future.get();
  checkedTargets.push_back(nextTarget);
}

void Uploader::checkNextTarget(std::promise<std::shared_ptr<Target>>& promise){
  if(targets.size() == 0){
    quit::failedToUpload("There is no reachable target matching your selection");
  }
  std::shared_ptr<Target> nextTarget = targets.front();
  targets.pop_front();
  nextTarget->dynamicSettingsCheck(settings.getRequiredFeatures(), [this, &promise, &nextTarget](){
    try{
      promise.set_value(nextTarget);
    }catch(std::exception e){
      quit::unexpectedFailure("Failed to set promise value. If your are compiling this code yourself, you probably forgot to enable threads . Try '-pthread'.");
    }
  },[this, &promise](std::string message){
    checkNextTarget(promise);
  }, 200);
}
