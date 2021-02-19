#include "quit.hpp"

[[ noreturn ]] void quit::success(){
  exit(0);
}

[[ noreturn ]] void quit::failedToUpload(const std::string& message){
  std::cerr << message << '\n';
  exit(3);
}

[[ noreturn ]] void quit::noTargetSelected(const std::string& message){
  std::cerr << message << '\n';
  exit(4);
}

[[ noreturn ]] void quit::invalidCliUsage(const std::string& message){
  std::cerr << message << '\n';
  exit(64);
}

[[ noreturn ]] void quit::failedReadingFiles(const std::string& message){
  std::cerr << message << '\n';
  exit(65);
}

[[ noreturn ]] void quit::unexpectedFailure(const std::string& message){
  std::cerr << message << '\n';
  exit(1);
}
