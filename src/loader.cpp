#include "loader.hpp"
#include <zip_file.hpp>

std::vector<File> loadFiles(const Settings& settings){
    std::vector<File> result;
    switch(settings.getMode()){
      case Settings::Mode::List:
        break;
      case Settings::Mode::Individual:
        for(const std::string& fileName: settings.getFiles()){
          auto path = loadPath(fileName);
          if(std::filesystem::is_directory(path)){
            result.push_back(createArchive(std::vector<std::filesystem::path>{path},path.filename(),settings.getDirectoryArchive()));
          }else{
            result.emplace_back(path);
          }
        }
        break;
      case Settings::Mode::Archive:
        std::vector<std::filesystem::path> paths;
        for(const std::string& fileName: settings.getFiles()){
          paths.push_back(loadPath(fileName));
        }
        result.push_back(createArchive(paths,settings.getArchiveName(),settings.getDirectoryArchive()));
        break;
    }
    return result;
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
        message << "You do not have the permission to access " << filePath << " . Contact your system administrator about that, or something. You can give everyone read permissions to that file with 'sudo chmod -R a+r " << filePath << "'";
        quit::failedReadingFiles(message.str());
      }
#endif
      return std::filesystem::path(filePath);
  }
}

File createArchive(const std::vector<std::filesystem::path>& files, const std::string& name, bool directoryCreation){
  miniz_cpp::zip_file file;

  for(const std::filesystem::path& path : files){
    if(std::filesystem::is_directory(path)){
      std::error_code error;
      std::filesystem::path canonicalPath =  std::filesystem::canonical(path, error);
      if(error){
        std::stringstream message;
        message << "Failed to create canonical path of " << path << " . This should not happen.";
        quit::failedReadingFiles(message.str());
      }
      
      std::filesystem::path basePath = canonicalPath;
      if(directoryCreation){
        basePath.remove_filename();
      }
      
      for(auto& p: std::filesystem::recursive_directory_iterator(canonicalPath)){
        //Absolute path in filesystem
        std::filesystem::path realPath = loadPath(p.path());
        //Path in archive
        std::filesystem::path resultPath = std::filesystem::relative(realPath, basePath);
        
        if(std::filesystem::is_directory(realPath)){
          std::string resultDirPath = resultPath.string() + "/";
          file.writestr(resultDirPath, "");
        }else{
          File f(realPath);
          file.writestr(resultPath, f.getContent());
        }
      }
      
    }else{
      File f(path);
      file.writestr(f.getName(), f.getContent());
    }
  }
  
  std::stringstream archiveStream;
  file.save(archiveStream);
 
  return File(name, archiveStream.str());
  
}
