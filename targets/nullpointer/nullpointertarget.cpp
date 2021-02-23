#include "nullpointertarget.hpp"

setTargetType(NullPointerTarget)

NullPointerTarget::NullPointerTarget(bool useSSL, const std::string& url, const std::string& name):  name(name), url(url), useSSL(useSSL), client(nullptr){
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
  
  httplib::MultipartFormDataItems items = {
    { "file", file.getContent(), file.getName(), "application/octet-stream" }
  };
  
  if(auto result = client->Post("/", items)){
    //cli.set_follow_location(true);
    logger.log(Logger::Topic::Debug) << "Received response from " << name << " (" << result->status << "): " << result->body << '\n';
    if(result->status != 200){
      std::stringstream message;
      message << "Request failed, responsecode " << httplib::detail::status_message(result->status) << "(" << result->status << ")." ;
      errorCallback(message.str());
      return;
    }
    
    std::string content = result->body;
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
    message << "Request failed, responsecode " << result.error() << "." ;
    errorCallback(message.str());
    return;
  }
}

bool NullPointerTarget::isReachable(std::string& errorMessage){
  if(auto result = client->Post("/")){
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
  std::string mimetype = file.getMimetype();
  
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
  
  if(file.getContent().size() > capabilities.maxSize){
    logger.log(Logger::Topic::Info) << name << " has a size limit of 512 MiB per file." << '\n';
    return false;
  }
  return true;
}

void NullPointerTarget::initializeClient(){
  if(client == nullptr){
    if(useSSL){
      std::string httpsUrl = "https://";
      httpsUrl.append(url);
      client = new httplib::Client(httpsUrl.c_str());
      const char* path = "libs/cpp-httplib/example/ca-bundle.crt";
      auto setCerts = []<typename T>(T client, const char* certs) {
        if constexpr(requires{
          client->set_ca_cert_path(certs);
        }){
          client->set_ca_cert_path(certs);
          logger.log(Logger::Topic::Debug) << "HTTPS is supported\n";
        }else{
          logger.log(Logger::Topic::Debug) << "HTTPS is not supported\n";
          throw std::invalid_argument("https is disabled");
        }
      };
      setCerts(client, path);
    }else{
      std::string httpUrl = "http://";
      httpUrl.append(url);
      client = new httplib::Client(httpUrl.c_str());
    }
    
    httplib::Headers headers = {
      { "Accept", "*/*" },
      { "User-Agent", userAgent }
    };
    client->set_default_headers(headers);
  }
}

long long NullPointerTarget::calculateRetentionPeriod(const File& f) const{
  long long min_age = capabilities.minRetention;
  long long max_age = capabilities.maxRetention;
  long long max_size = capabilities.maxSize;
  long long file_size = f.getContent().size();
  long long retention = min_age + (-max_age + min_age) * pow((file_size / max_size - 1), 3);
  if(retention < min_age){
    return min_age;
  }else if(retention > max_age){
    return max_age;
  }else{
    return retention;
  }
}

std::vector<Target*> NullPointerTarget::loadTargets(){
  std::vector<Target*> targets;
  
  try{
    Target* httpTarget = new NullPointerTarget(false, "0x0.st", "THE NULL POINTER (HTTP)");
    targets.push_back(httpTarget);
  }catch(std::invalid_argument& e){
    logger.log(Logger::Info) << "Failed to load nullpointertarget (http):" << e.what() << "\n";
  }
  
  try{
    Target* httpsTarget = new NullPointerTarget(true, "0x0.st", "THE NULL POINTER");
    targets.push_back(httpsTarget);
  }catch(std::invalid_argument& e){
    logger.log(Logger::Info) << "Failed to load nullpointertarget (https):" << e.what() << "\n";
  }
  
  return targets;
}
