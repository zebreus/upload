#ifndef LOADER_HPP
#define LOADER_HPP

#include <vector>
#include <string>
#include "file.hpp"
#include "settings.hpp"


#ifdef __unix__
#include <unistd.h>
#endif

std::vector<File> loadFiles(const Settings& settings);

//Converts a string to a path and ensures, that it is readable and a regular file or a directory
std::filesystem::path loadPath(const std::string& filePath);

bool isDirectory(const std::filesystem::path& path);

File createArchive(const std::vector<std::filesystem::path>& files, const std::string& name, bool directoryCreation);

#endif
