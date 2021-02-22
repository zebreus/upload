#ifndef TARGET_LOADER_HPP
#define TARGET_LOADER_HPP

#include <vector>
#include <memory>
#include "target.hpp"

std::vector<std::shared_ptr<Target>> loadTargets();

#endif
