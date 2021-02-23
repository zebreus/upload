#include "transfershtarget.hpp"

setTargetType(TransferShTarget)

TransferShTarget::TransferShTarget(bool useSSL, const std::string& url, const std::string& name): HttplibTarget(useSSL,url,name){
  capabilities.maxSize = (long long)10*1024*1024*1024;
  capabilities.preserveName.reset(new bool(true));
  capabilities.minRetention = (long long)14*24*60*60*1000;
  capabilities.maxRetention = (long long)14*24*60*60*1000;
}

bool TransferShTarget::staticFileCheck(BackendRequirements requirements, const File& file) const{
  if(!checkFile(file)){
    return false;
  }
  
  return true;
}

void TransferShTarget::uploadFile(BackendRequirements requirements, const File& file, std::function<void(std::string)> successCallback, std::function<void(std::string)> errorCallback){

  try{
    std::string response = putFile(file);
    std::vector<std::string> urls = findValidUrls(response);
    if(urls.size()>=1){
      successCallback(urls.front());
    }else{
      std::string message = "Response did not contain any urls";
      errorCallback(message);
    }
  }catch(std::runtime_error& error){
    errorCallback(error.what());
  }
}

std::vector<Target*> TransferShTarget::loadTargets(){
  std::vector<Target*> targets;
  
  try{
    Target* httpTarget = new TransferShTarget(false, "transfer.sh", "transfer.sh (HTTP)");
    targets.push_back(httpTarget);
  }catch(std::invalid_argument& e){
    logger.log(Logger::Info) << "Failed to load TransferShTarget (http):" << e.what() << "\n";
  }
  
  try{
    Target* httpsTarget = new TransferShTarget(true, "transfer.sh", "transfer.sh");
    targets.push_back(httpsTarget);
  }catch(std::invalid_argument& e){
    logger.log(Logger::Info) << "Failed to load TransferShTarget (https):" << e.what() << "\n";
  }
  
  return targets;
}



