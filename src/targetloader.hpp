#ifndef TARGET_LOADER_HPP
#define TARGET_LOADER_HPP

#include <memory>
#include <vector>

#include "target.hpp"

std::vector<std::shared_ptr<Target>> loadTargets();

#endif
