#include "quit.hpp"

[[noreturn]] void quit::success() {
  exit(0);
}

[[noreturn]] void quit::failedToUpload() {
  exit(3);
}

[[noreturn]] void quit::noBackendSelected() {
  exit(4);
}

[[noreturn]] void quit::invalidCliUsage() {
  exit(64);
}

[[noreturn]] void quit::failedReadingFiles() {
  exit(65);
}

[[noreturn]] void quit::unexpectedFailure() {
  exit(1);
}
