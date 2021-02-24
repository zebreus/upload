#include "file.hpp"

#include <map>

#include "logger.hpp"
#include "quit.hpp"

File::File(const std::filesystem::path& path) {
  std::error_code error;
  if(!std::filesystem::is_regular_file(path, error)) {
    logger.log(Logger::Fatal) << "You tried to open " << path.string()
                              << ", but that is not a regular file. Actually you should not be able to get this error, because the paths "
                                 "are checked, before opening a file. If you see this error message, please open an issue on github.";
    quit::failedReadingFiles();
  }

  std::ifstream fileStream(path, std::ios::binary | std::ios::in);
  fileStream.seekg(0, std::ios::end);
  size_t size = fileStream.tellg();
  content.resize(size);
  fileStream.seekg(0);
  fileStream.read(&content[0], size);

  name = path.filename();
}

File::File(const std::string& name, const std::string& content): name(name), content(content) {}

std::string File::getName() const {
  return name;
}

std::string File::getContent() const {
  return content;
}

std::string File::getMimetype() const {
  static std::map<std::string, std::string> extensionMap = {{"", "application/octet-stream"},
                                                            {"he5", "application/x-hdf5"},
                                                            {"hdf5", "application/x-hdf5"},
                                                            {"h5", "application/x-hdf5"},
                                                            {"apk", "application/vnd.android.package-archive"},
                                                            {"jar", "application/java-archive"},
                                                            {"css", "text/css"},
                                                            {"csv", "text/csv"},
                                                            {"txt", "text/plain"},
                                                            {"vtt", "text/vtt"},
                                                            {"htm", "text/html"},
                                                            {"html", "text/html"},
                                                            {"apng", "image/apng"},
                                                            {"avif", "image/avif"},
                                                            {"bmp", "image/bmp"},
                                                            {"gif", "image/gif"},
                                                            {"png", "image/png"},
                                                            {"svg", "image/svg+xml"},
                                                            {"webp", "image/webp"},
                                                            {"ico", "image/x-icon"},
                                                            {"tif", "image/tiff"},
                                                            {"tiff", "image/tiff"},
                                                            {"jpg", "image/jpeg"},
                                                            {"jpeg", "image/jpeg"},
                                                            {"mp4", "video/mp4"},
                                                            {"mpeg", "video/mpeg"},
                                                            {"webm", "video/webm"},
                                                            {"mp3", "audio/mp3"},
                                                            {"mpga", "audio/mpeg"},
                                                            {"weba", "audio/webm"},
                                                            {"wav", "audio/wave"},
                                                            {"otf", "font/otf"},
                                                            {"ttf", "font/ttf"},
                                                            {"woff", "font/woff"},
                                                            {"woff2", "font/woff2"},
                                                            {"7z", "application/x-7z-compressed"},
                                                            {"atom", "application/atom+xml"},
                                                            {"pdf", "application/pdf"},
                                                            {"js", "application/javascript"},
                                                            {"mjs", "application/javascript"},
                                                            {"json", "application/json"},
                                                            {"rss", "application/rss+xml"},
                                                            {"tar", "application/x-tar"},
                                                            {"xht", "application/xhtml+xml"},
                                                            {"xhtml", "application/xhtml+xml"},
                                                            {"xslt", "application/xslt+xml"},
                                                            {"xml", "application/xml"},
                                                            {"gz", "application/gzip"},
                                                            {"zip", "application/zip"},
                                                            {"wasm", "application/wasm"},
                                                            {"class", "application/java-vm"}};

  std::string extension = std::filesystem::path(name).extension();
  if(extension.size() > 0 && extension[0] == '.') {
    extension = extension.substr(1);
  }

  if(extensionMap.contains(extension)) {
    return extensionMap[extension];
  } else {
    return "application/octet-stream";
  }
}
