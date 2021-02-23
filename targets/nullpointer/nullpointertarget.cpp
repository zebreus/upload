#include "nullpointertarget.hpp"
#include <httplib.h>

setTargetType(NullPointerTarget)

NullPointerTarget::NullPointerTarget(bool useSSL, const std::string& url = "0x0.st", const std::string& name = "THE NULL POINTER"):  name(name), url(url), useSSL(useSSL){
  if(useSSL){
    capabilities.http = false;
    capabilities.https = true;
  }else{
    capabilities.http = true;
    capabilities.https = false;
  }
  capabilities.maxSize = 512*1024*1024;
  capabilities.preserveName.reset(new bool(false));
  capabilities.minRetention = (long long)30*24*60*60*1000;
  capabilities.maxRetention = (long long)365*24*60*60*1000;
  
  initializeClient();
}

NullPointerTarget::~NullPointerTarget(){
  delete client;
}

std::string NullPointerTarget::getName() const{
  return name;
}

bool NullPointerTarget::staticSettingsCheck(BackendRequirements requirements) const{
  if(!capabilities.meetsRequirements(requirements)){
    logger.log(Logger::Topic::Debug) << "Not all requiredFeatures are supported\n";
    return false;
  }
  return true;
}

bool NullPointerTarget::staticFileCheck(BackendRequirements requirements, const File& file) const{
  std::string filename = file.getName();
  if( filename == "" ){
    logger.log(Logger::Topic::Info) << "You have specified an empty filename." << '\n';
    return false;
  }
  if(!checkFile(file)){
    return false;
  }
  return true;
}

void NullPointerTarget::dynamicSettingsCheck(BackendRequirements requirements, std::function<void()> successCallback, std::function<void(std::string)> errorCallback, int timeoutMillis){
  std::string errorMessage;
  if(isReachable(errorMessage)){
    successCallback();
  }else {
    errorCallback(errorMessage);
  }
}

void NullPointerTarget::uploadFile(BackendRequirements requirements, const File& file, std::function<void(std::string)> successCallback, std::function<void(std::string)> errorCallback){

  
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
    logger.log(Logger::Topic::Debug) << "Received response from " << name << " (" << res->status << "): " << res->body << '\n';
    if(res->status != 200){
      std::stringstream message;
      message << "Request failed, responsecode " << httplib::detail::status_message(res->status) << "(" << res->status << ")." ;
      errorCallback(message.str());
      return;
    }
    
    std::string content = res->body;
    //TODO improve expression
    std::regex urlExpression("http[-\\]_.~!*'();:@&=+$,/?%#[A-z0-9]+", std::regex::icase | std::regex::ECMAScript);
    std::smatch results;
    std::regex_search(content,results,urlExpression);
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
    errorCallback(message.str());
    return;
  }
}

bool NullPointerTarget::isReachable(std::string& errorMessage){
  if(auto result = client.Post("/")){
    //client.set_follow_location(true);
    logger.log(Logger::Topic::Debug) << "Received response from " << name << " (" << result->status << "): " << result->body << '\n';
    if(result->status == 200 || result->status == 400){
      return true;
    }else{
      errorMessage = "Received an unexpected reply. This should not happen.";
      return false;
    }
  }else{
    std::stringstream message;
    switch(result.error()){
      case httplib::Connection:
      case httplib::BindIPAddress:
      message << name << " is not online. Check your internet connection.";
      case httplib::Read:
      case httplib::Write:
      case httplib::Canceled:
      message << name << " seems to be online, but something went wrong.";
      case httplib::SSLConnection:
      case httplib::SSLLoadingCerts:
      case httplib::SSLServerVerification:
      message << "Some kind of SSL error happened.";
      case httplib::ExceedRedirectCount:
      message << "Too many redirects. This is probably not your fault.";
      default:
      message << "Failed to establish connection. Maybe try again later.";
    }
    errorMessage = message.str();
    return false;
  }
}

bool NullPointerTarget::checkFile(const File& file) const{

  
  if(
    mimetype == "application/x-dosexec" ||
    mimetype == "application/x-executable" ||
    mimetype == "application/x-hdf5" ||
    mimetype == "application/vnd.android.package-archive" ||
    mimetype == "application/java-archive" ||
    mimetype == "application/java-vm"
  ){
    logger.log(Logger::Topic::Info) << " does not allow the upload of " << mimetype << " files." << '\n';
    return false;
  }
  
  if(file.getContent().size() > 536870912){
    logger.log(Logger::Topic::Info) << name << " has a size limit of 512 MiB per file." << '\n';
    return false;
  }
  return true;
}

std::vector<Target*> NullPointerTarget::loadTargets(){
  Target* myTarget = new NullPointerTarget();
  return std::vector<Target*>{myTarget};
}

/*
std::string NullPointerTarget::generateUrl(){
  std::string httpUrl
  if(useSSL){
    httpUrl = "https://";
  }else{
    httpUrl = "http://";
  }
  httpUrl.append(url);
  return url;
}
* */

void NullPointerTarget::initializeClient(){
  if(client == nullptr){
    if(useSSL){
      httplib::SSLClient* httpsClient = new httplib::SSLClient(url);
      httpsClient.set_ca_cert_path("libs/cpp-httplib/example/ca-bundle.crt");
      client = httpsClient;
    }else{
      httplib::Client* httpClient = new httplib::SSLClient(url);
      client = httpClient;
    }
    
    httplib::Headers headers = {
      { "Accept", "*/*" },
      { "User-Agent", userAgent }
    };
    client.set_default_headers(headers);
  }
}
