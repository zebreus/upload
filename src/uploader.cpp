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
    }catch(std::runtime_error e){
      logger.log(Logger::Info) << "Failed to upload " << file.getName() << " to " << target->getName() << ". " << e.what() << '\n';
    }catch(...){
      logger.log(Logger::Info) << "Unexpected error while uploading " << file.getName() << " to " << target->getName() << "." << '\n';
    }
  }
  
  while(true){
    checkNextTarget();
    std::shared_ptr<Target> target = checkedTargets.back();
    try{
      return uploadFile(file, target);
    }catch(std::runtime_error e){
      logger.log(Logger::Info) << "Failed to upload " << file.getName() << " to " << target->getName() << ". " << e.what() << '\n';
    }catch(...){
      logger.log(Logger::Info) << "Unexpected error while uploading " << file.getName() << " to " << target->getName() << "." << '\n';
    }
  }
}

std::string Uploader::uploadFile(const File& file, std::shared_ptr<Target> target){
  std::promise<std::string> urlPromise;
  target->uploadFile(settings.getBackendRequirements(), file, [this, &urlPromise](std::string url){
    try{
      urlPromise.set_value(url);
    }catch(std::exception e){
      logger.log(Logger::Fatal) << "Failed to set urlPromise value. If your are compiling this code yourself, you probably forgot to enable threads . Try linking with '-pthread'." << '\n';
      quit::unexpectedFailure();
    }
  },[&urlPromise](std::string message){
    urlPromise.set_exception(std::make_exception_ptr(std::runtime_error(message)));
  });
  
  return urlPromise.get_future().get();
}

void Uploader::printAvailableTargets(){
  for(const std::shared_ptr<Target>& target: checkedTargets){
    logger.log(Logger::Print) << target->getName() << '\n';
  }
  for(const std::shared_ptr<Target>& target: targets){
    logger.log(Logger::Print) << target->getName() << '\n';
  }
}

void Uploader::initializeTargets(const Settings& settings){
  std::vector<std::shared_ptr<Target>> loadedTargets = loadTargets();
  
  //Find all targets with a requested name, possibly multiple with the same name, but none twice
  if(settings.getRequestedTargets().size() > 0){
    std::vector<std::shared_ptr<Target>> unmatchedTargets = loadedTargets;
    std::vector<std::shared_ptr<Target>> orderedTargets;
    std::vector<std::shared_ptr<Target>> nextTargets;
    for(const std::string& targetName : settings.getRequestedTargets()){
      bool found = false;
      for(const std::shared_ptr<Target>& target : unmatchedTargets){
        if(target->getName() == targetName){
          orderedTargets.push_back(target);
          found = true;
        }else{
          nextTargets.push_back(target);
        }
      }
      if(!found){
        logger.log(Logger::Topic::Fatal) << "Unable to find requested target '" << targetName << "'. Maybe check for a typo in its name.";
        quit::invalidCliUsage();
      }
      unmatchedTargets = nextTargets;
    }
    loadedTargets = orderedTargets;
  }
  
  for(std::shared_ptr<Target> target: loadedTargets){
    if(target->staticSettingsCheck(settings.getBackendRequirements())){
      logger.log(Logger::Debug) << target->getName() << " has all required features." << '\n';
      targets.push_back(target);
    }else{
      logger.log(Logger::Debug) << target->getName() << " does not have all required features." << '\n';
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
    logger.log(Logger::Fatal) << "There is no target matching your requirements" << '\n';
    quit::failedToUpload();
  }
  std::shared_ptr<Target> nextTarget = targets.front();
  targets.pop_front();
  nextTarget->dynamicSettingsCheck(settings.getBackendRequirements(), [this, &promise, &nextTarget](){
    try{
      promise.set_value(nextTarget);
    }catch(std::exception e){
      logger.log(Logger::Fatal) << "Failed to set promise value. If your are compiling this code yourself, you probably forgot to enable threads . Try '-pthread'." << '\n';
      quit::unexpectedFailure();
    }
  },[this, &promise](std::string message){
    checkNextTarget(promise);
  }, 200);
}
