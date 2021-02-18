#include "settings.hpp"

Settings::Settings(int argc, char** argv){
  parseOptions(argc,argv);
}

Settings::Mode Settings::getMode() const{
  return mode;
}

Settings::VerbosityLevel Settings::getVerbosity() const{
  return verbosity;
}

Settings::ArchiveType Settings::getArchiveType() const{
  return archiveType;
}

Settings::HttpsSetting Settings::getHttpsSetting() const{
  return httpsSetting;
}

bool Settings::getPreserveName() const{
  return preserveName;
}

std::string Settings::getArchiveName() const{
  return archiveName;
}

std::vector<std::string> Settings::getRequestedTargets() const{
  return requestedTargets;
}

std::vector<std::string> Settings::getFiles() const{
  return files;
}

cxxopts::Options Settings::generateParser(){
  cxxopts::Options options("upload", "Upload files to the internet");
  options.add_options()
  ("h,help", "Displays this help screen")
  ("version", "Displays version information")
  ("v", "Increase verbosity")
  ("a,archive", "Pack all files and directories into an archive")
  ("i,individual", "Upload all files or directory individually")
  ("l,list", "List all available upload targets for the current request, ordered by preference")
  ("archive-type", "Sets the archive type", cxxopts::value<std::string>())
  ("d,directory-in-archive", "Put the contents in the root of the archive", cxxopts::value<std::string>()->implicit_value("true"))
  ("t,target", "Add a specific target. If this option is used, the default order is discarded.", cxxopts::value<std::vector<std::string>>())
  ("p,preserve-name", "Ensure that the filenames are preserved.")
  ("s,ssl", "Ensure the use of https.")
  ("no-ssl", "Ensure the use of http.")
  ("n,name", "The name of the created archive in archive mode.", cxxopts::value<std::string>()->default_value(generateArchiveName()))
  ("file", "The files that will be uploaded.", cxxopts::value<std::vector<std::string>>())
  ;
  options.parse_positional({"file"});
  return options;
}

void Settings::parseOptions(int argc, char** argv){
  try{
  cxxopts::Options options = generateParser();
  auto result = options.parse(argc,argv);
  
  if (result.count("help")){
    std::cout << options.help() << std::endl;
    exit(0);
  }
  
  mode = parseMode(result);
  files = parseFiles(result, mode);
  archiveType = parseArchiveType(result);
  httpsSetting = parseHttpsSetting(result);
  verbosity = parseVerbosity(result);
  if(result.count("target")){
    requestedTargets = result["file"].template as<std::vector<std::string>>();
  }
  archiveName = result["name"].template as<std::string>();
  preserveName = result.count("preserve-name");
  }catch(std::exception e){
    quit::invalidCliUsage(e.what());
  }
}

std::string Settings::generateArchiveName(){
  char name[11];
  for(int x = 0; x < 10; x++){
    name[x] = (rand()%26)+97;
  }
  return name;
}

Settings::Mode Settings::parseMode(const auto& parseResult){
  if (parseResult.count("archive") && parseResult.count("individual")){
    quit::invalidCliUsage("You cannot set archive mode and individual mode");
  }
  
  // Test for flags
  if (parseResult.count("list")){
    return Mode::List;
  }
  if (parseResult.count("archive")){
    return Mode::Archive;
  }
  if (parseResult.count("individual")){
    return Mode::Individual;
  }

  // Default mode depends on the number of files
  if (parseResult.count("file") <= 1){
    return Mode::Individual;
  }else{
    return Mode::Archive;
  }
}

Settings::VerbosityLevel Settings::parseVerbosity(const auto& parseResult){
  switch (parseResult.count("v")){
    case 0:
      return VerbosityLevel::Default;
    case 1:
      return VerbosityLevel::Verbose;
    case 2:
    default:
      return VerbosityLevel::Debug;
  }
}

Settings::ArchiveType Settings::parseArchiveType(const auto& parseResult){
  if(parseResult.count("archive-type")){
    std::string archiveType = parseResult["archive-type"].template as<std::string>();
    
    if(archiveType == "zip"){
      return ArchiveType::Zip;
    }else{
      quit::invalidCliUsage("You specified an invalid archive-type. The only possible value is 'zip'");
    }
  }
  return defaultArchiveType;
}

Settings::HttpsSetting Settings::parseHttpsSetting(const auto& parseResult){
  if(parseResult.count("ssl") && parseResult.count("no-ssl")){
    quit::invalidCliUsage("You cannot have ssl and no-ssl, try leaving one away");
  }
  
  if(parseResult.count("ssl")){
    return HttpsSetting::Force;
  }
  
  if(parseResult.count("no-ssl")){
    return HttpsSetting::Forbid;
  }
  
  return HttpsSetting::Allow;
}

std::vector<std::string> Settings::parseFiles(const auto& parseResult, Settings::Mode mode){
  bool filesRequired = ( mode == Mode::Archive || mode == Mode::Individual );
  if(parseResult.count("file") == 0){
    if(filesRequired){
      quit::invalidCliUsage("You have to specify some files to upload. Just write them like 'upload FILE1 FILE2'");
    }else{
      return {};
    }
  }
  return parseResult["file"].template as<std::vector<std::string>>();
}
