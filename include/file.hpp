#ifndef FILE_HPP
#define FILE_HPP

#include <filesystem>
#include <fstream>
#include <string>

class File {
  std::string name;
  std::string content;

 public:
  explicit File(const std::filesystem::path& path);
  File(std::string name, std::string content);
  [[nodiscard]] std::string getName() const;
  [[nodiscard]] std::string getContent() const;
  [[nodiscard]] std::string getMimetype() const;
};

#endif
