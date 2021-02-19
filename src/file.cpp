#include "file.hpp"

File::File(const std::filesystem::path& path){
  std::error_code error;
  if( !std::filesystem::is_regular_file(path, error)){
    quit::failedReadingFiles("You tried to open something, that is not a regular file. If you see this error message, please open an issue on github.");
  }
  
  std::ifstream fileStream(path, std::ios::binary | std::ios::in);
  fileStream.seekg(0, std::ios::end);
  size_t size = fileStream.tellg();
  content.resize(size);
  fileStream.seekg(0);
  fileStream.read(&content[0], size);
  
  name = path.filename();
}

File::File(const std::string& name, const std::string& content): name(name), content(content){
  
}

std::string File::getName() const{
  return name;
}

std::string File::getContent() const{
  return content;
}
