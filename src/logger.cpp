#include "logger.hpp"

Logger logger;

Logger::Logger() {
  nullstream = new std::ofstream();
  nullstream->setstate(std::ios_base::badbit);

  topicStream[Topic::Fatal] = &std::cout;
  topicStream[Topic::Print] = &std::cout;
  topicStream[Topic::Url] = &std::cout;
  topicStream[Topic::Info] = nullptr;
  topicStream[Topic::Debug] = nullptr;
}

Logger::~Logger() {
  delete nullstream;
}

bool Logger::getTopicState(Topic topic) {
  return (topicStream[topic] != nullptr && topicStream[topic] != nullstream);
}

void Logger::setTopicStream(Topic topic, std::ostream* stream) {
  topicStream[topic] = stream;
}

void Logger::setTopicStream(Logger::Topic topic, Logger::Topic stream) {
  topicStream[topic] = topicStream[stream];
}

void Logger::log(Topic topic, const std::string& message) {
  log(topic) << message;
}

std::ostream& Logger::log(Topic topic) {
  if(topicStream[topic] == nullptr) {
    return *nullstream;
  }
  return *topicStream[topic];
}

std::ostream& Logger::debug() {
  return log(Topic::Debug);
}
