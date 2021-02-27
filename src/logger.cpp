#include "logger.hpp"

Logger logger;

Logger::Logger() {
  topicState[Topic::Fatal] = true;
  topicState[Topic::Print] = true;
  topicState[Topic::Url] = true;
  topicState[Topic::Info] = false;
  topicState[Topic::Debug] = false;
}

bool Logger::getTopicState(Topic topic) {
  return topicState[topic];
}

void Logger::setTopicState(Topic topic, bool state) {
  topicState[topic] = state;
}

void Logger::log(Topic topic, const std::string& message) {
  log(topic) << message;
}

std::ostream& Logger::log(Topic topic) {
  static std::ofstream nullstream;
  static bool markedBad = false;
  if(!markedBad) {
    nullstream.setstate(std::ios_base::badbit);
    markedBad = true;
  }

  if(getTopicState(topic)) {
    switch(topic) {
      case Topic::Fatal:
        return std::clog;
      case Topic::Print:
        return std::cout;
      case Topic::Debug:
        return std::clog;
      case Topic::Info:
        return std::clog;
      case Topic::Url:
        return std::cout;
      default:
        return nullstream;
    }
  } else {
    return nullstream;
  }
}

std::ostream& Logger::debug() {
  return log(Topic::Debug);
}
