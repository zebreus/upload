#ifndef QUIT_HPP
#define QUIT_HPP

#include <iostream>
#include <string>

namespace quit{

[[ noreturn ]] void success();

[[ noreturn ]] void failedToUpload(const std::string& message = "");

[[ noreturn ]] void noTargetSelected(const std::string& message = "");

[[ noreturn ]] void invalidCliUsage(const std::string& message = "");

[[ noreturn ]] void failedReadingFiles(const std::string& message = "");

}

#endif
