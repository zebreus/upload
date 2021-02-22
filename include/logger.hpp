#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <map>
#include <string>
#include <iostream>
#include <fstream>

//TODO maybe rename to printer
class Logger{
public:
  // Fatal = exit message
  // Print = program output
  // Info = additional output
  // Debug = debug message
  // Url = result url
  enum Topic{
    Fatal,
    Print,
    Info,
    Debug,
    Url
  };
private:
  std::map<Topic,bool> topicState;

public:
  Logger();
  bool getTopicState(Topic topic);
  void setTopicState(Topic topic, bool state);
  void log(Topic topic, const std::string& message);
  std::ostream& log(Topic topic);
  std::ostream& debug();
};

//Global logger object
extern Logger logger;

#endif
