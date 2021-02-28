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
    try {
      logger.log(Logger::Url) << uploader.uploadFile(file) << std::endl;
    } catch(const std::runtime_error& error) {
      logger.log(Logger::Info) << error.what() << '\n';
    }
  }

  // Exit, because there is no need to wait for other threads anymore.
  quit::success();
}
