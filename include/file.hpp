#ifndef FILE_HPP
#define FILE_HPP

#include <string>
#include <fstream>
#include <filesystem>

class File{
  std::string name;
  std::string content;
public:
  File(const std::filesystem::path& path);
  File(const std::string& name, const std::string& content);
  std::string getName() const;
  std::string getContent() const;
  std::string getMimetype() const;
};

#endif
