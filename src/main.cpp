#include "settings.hpp"
#include <vector>
#include "file.hpp"
#include "loader.hpp"

int main(int argc , char** argv){
  Settings settings(argc, argv);
  
  std::vector<File> files = loadFiles(settings);
  
  for(const File& file : files){
    std::cout << file.getName() << std::endl;
  }
  return 0;
}
