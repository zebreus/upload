#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <fstream>
#include <iostream>
#include <map>
#include <string>

// TODO maybe rename to printer
class Logger {
 public:
  // Fatal = exit message
  // Print = program output
  // Info = additional output
  // Debug = debug message
  // Url = result url
  // LoadFatal = file loading failed, program might exit
  // UploadFatal = file uploading failed, program might exit
  enum Topic { Fatal, Print, Info, Debug, Url, LoadFatal, UploadFatal };

 private:
  std::map<Topic, std::ostream*> topicStream;
  std::ostream* nullstream;

 public:
  Logger();
  Logger(Logger&) = delete;
  ~Logger();
  bool getTopicState(Topic topic);
  void setTopicStream(Topic topic, std::ostream* stream);
  void setTopicStream(Topic topic, Topic stream);
  void log(Topic topic, const std::string& message);
  std::ostream& log(Topic topic);
  std::ostream& debug();
};

// Global logger object
extern Logger logger;

#endif
