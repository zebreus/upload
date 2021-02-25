#ifndef QUIT_HPP
#define QUIT_HPP

#include <iostream>
#include <string>

namespace quit {

[[noreturn]] void success();

[[noreturn]] void failedToUpload();

[[noreturn]] void noBackendSelected();

[[noreturn]] void invalidCliUsage();

[[noreturn]] void failedReadingFiles();

[[noreturn]] void unexpectedFailure();

}  // namespace quit

#endif
