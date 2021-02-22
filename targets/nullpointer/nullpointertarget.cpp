#include "nullpointertarget.hpp"

setTargetType(NullPointerTarget)

NullPointerTarget::NullPointerTarget(){
}

NullPointerTarget::~NullPointerTarget(){
}

std::string NullPointerTarget::getName() const{
  return name;
}

BackendFeatures NullPointerTarget::getSupportedFeatures() const{
  return supportedFeatures;
}

bool NullPointerTarget::staticSettingsCheck(BackendFeatures requiredFeatures) const{
  if((requiredFeatures & supportedFeatures) != requiredFeatures){
    //std::clog << "Not all requiredFeatures are supported" << requiredFeatures << " " << supportedFeatures << "\n";
    return false;
  }

  return true;
}

bool NullPointerTarget::staticFileCheck(BackendFeatures requiredFeatures, const File& file) const{
  std::string filename = file.getName();
  if( filename == "" ){
    //std::clog << "Filename has to be set\n";
    return false;
  }
  if(!checkFile(file)){
    return false;
  }
  return true;
}

void NullPointerTarget::dynamicSettingsCheck(BackendFeatures requiredFeatures, std::function<void()> successCallback, std::function<void(std::string)> errorCallback, int timeoutMillis){  
  if(isReachable()){
    successCallback();
  }else {
    errorCallback("Failed to connect");
  }
}

void NullPointerTarget::uploadFile(BackendFeatures requiredFeatures, const File& file, std::function<void(std::string)> successCallback, std::function<void(std::string)> errorCallback){
  std::string httpUrl = "http://";
  httpUrl.append(url);
  httplib::Client cli(httpUrl.c_str());
  //cli.set_ca_cert_path("libs/cpp-httplib/example/ca-bundle.crt");
  
  httplib::Headers headers = {
    { "Accept", "*/*" },
    { "User-Agent", userAgent }
  };
  
  httplib::MultipartFormDataItems items = {
    { "file", file.getContent(), file.getName(), "application/octet-stream" }
  };
  
  if(auto res = cli.Post("/", headers, items)){
    //cli.set_follow_location(true);
    std::cout << res->status;
    std::cout << res->body;
    if(res->status != 200){
      std::stringstream message;
      message << "Request failed, responsecode " << httplib::detail::status_message(res->status) << "(" << res->status << ")." ;
      std::cout << res.error();
      errorCallback(message.str());
      return;
    }
    
    std::string content = res->body;
    //TODO improve expression
    std::regex urlExpression("http[^ ]*", std::regex::icase | std::regex::ECMAScript);
    std::smatch results;
    std::regex_match(content,results,urlExpression);
    if(results.size()==1){
      std::string url = results[0];
      successCallback(url);
      return;
    }else if(results.size()==0){
      std::string message = "Response did not contain any urls";
      errorCallback(message);
      return;
    }else{
      std::stringstream message;
      message << "Response contained to many urls. ";
      for(int x = 0; x<results.size();x++){
        message << "URL " << x << ": " << results[x] << ". "; 
      }
      errorCallback(message.str());
      return;
    }
  }else{
    std::stringstream message;
    message << "Request failed, responsecode " << res.error() << "." ;
    std::cout << res.error();
    errorCallback(message.str());
    return;
  }
}

bool NullPointerTarget::isReachable(){
  std::string httpUrl = "http://";
  httpUrl.append(url);
  httplib::Client cli(httpUrl.c_str());
  //cli.set_ca_cert_path("libs/cpp-httplib/example/ca-bundle.crt");
  
  httplib::Headers headers = {
    { "Accept", "*/*" },
    { "User-Agent", userAgent }
  };
  cli.set_default_headers(headers);
  if(auto res = cli.Post("/")){
    //cli.set_follow_location(true);
    std::cout << res->status;
    std::cout << res->body;
    if(res->status == 200 || res->status == 400){
      return true;
    }else{
      return false;
    }
  }else{
    std::stringstream message;
    message << "Reachability check failed, code " << res.error() ;
    std::cout << message.str();
    return false;
  }
}

bool NullPointerTarget::checkFile(const File& f) const{
  return true;
}

std::vector<Target*> NullPointerTarget::loadTargets(){
  Target* myTarget = new NullPointerTarget();
  return std::vector<Target*>{myTarget};
}
