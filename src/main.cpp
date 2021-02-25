#include <vector>

#include "file.hpp"
#include "loader.hpp"
#include "logger.hpp"
#include "settings.hpp"
#include "uploader.hpp"

int main(int argc, char** argv) {
  Settings settings(argc, argv);

  Loader loader(settings);
  Uploader uploader(settings);
  
  for(const File& file : loader) {
    logger.log(Logger::Debug) << "Uploading " << file.getName() << '\n';
    logger.log(Logger::Url) << uploader.uploadFile(file) << std::endl;
    
  }
  
  return 0;
}
