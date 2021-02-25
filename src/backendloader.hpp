#ifndef BACKEND_LOADER_HPP
#define BACKEND_LOADER_HPP

#include <memory>
#include <vector>

#include "backend.hpp"

std::vector<std::shared_ptr<Backend>> loadBackends();

#endif
