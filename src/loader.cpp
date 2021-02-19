#include "loader.hpp"

std::vector<File> loadFiles(const Settings& settings){
    std::vector<File> result;
    switch(settings.getMode()){
      case Settings::Mode::List:
        return result;
      case Settings::Mode::Individual:
        for(const std::string& fileName: settings.getFiles()){
          auto path = loadPath(fileName);
          if(std::filesystem::is_directory(path)){
            result.push_back(createArchive(std::vector<std::filesystem::path>{path},path.filename(),false));
          }else{
            result.emplace_back(path);
          }
        }
        return result;
      case Settings::Mode::Archive:
        break;
    }
    return std::vector<File>{};
}

std::filesystem::path loadPath(const std::string& filePath){
  std::error_code error;
  std::filesystem::file_status fileStatus = std::filesystem::status(filePath, error);
  std::stringstream message;
  switch(fileStatus.type()){
    case std::filesystem::file_type::none:
      message << "Failed to get information about the file " << filePath << ". ";
      if(error){
        message << error.message();
      }
      quit::failedReadingFiles(message.str());
    case std::filesystem::file_type::not_found:
      message << filePath << " does not exist. Maybe check for typos.";
      quit::failedReadingFiles(message.str());
    case std::filesystem::file_type::symlink:
      message << "You tried to upload the file " << filePath << " , which is a symlink. Those are currently not supported.";
      quit::failedReadingFiles(message.str());
    case std::filesystem::file_type::block:
      message << "You tried to upload the blockdevice " << filePath << " . Currently only regular files and directories are supported.";
      quit::failedReadingFiles(message.str());
    case std::filesystem::file_type::character:
      message << "You tried to upload the characterdevice " << filePath << " . Currently only regular files and directories are supported.";
      quit::failedReadingFiles(message.str());
    case std::filesystem::file_type::fifo:
      message << "You tried to upload the pipe " << filePath << " . Currently only regular files and directories are supported. Pipes may be supported somewhere in the future.";
      quit::failedReadingFiles(message.str());
    case std::filesystem::file_type::socket:
      message << "You tried to upload the socket " << filePath << " . Currently only regular files and directories are supported.";
      quit::failedReadingFiles(message.str());
    case std::filesystem::file_type::unknown:
      message << "Failed to determine to filetype of " << filePath << " . You should check, that you have permissions to access that file. ";
      if(error){
        message << error.message();
      }
      quit::failedReadingFiles(message.str());
    default:
      message << "You tried to upload a strange filetype " << filePath << " . I do not know how you got here, but you probably know what you did wrong.";
      quit::failedReadingFiles(message.str());
    
    case std::filesystem::file_type::regular:
    case std::filesystem::file_type::directory:
      // Currently only checking access permissions on unix
#ifdef __unix__
      int accessResult = access(filePath.c_str(), R_OK);
      if(accessResult != 0){
        message << "You do not have the permission to access " << filePath << " . Contact your systemadmisitrator about that. You can give everyone read permissions to that file with 'sudo chmod -R a+r " << filePath << "'";
        quit::failedReadingFiles(message.str());
      }
#endif
      return std::filesystem::path(filePath);
  }
}

File createArchive(const std::vector<std::filesystem::path>& files, const std::string& name, bool createDirectoriesInRoot){
  //TODO replace placeholder
  return File("placeholder", "content");
}
