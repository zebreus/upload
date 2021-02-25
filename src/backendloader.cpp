#include "backendloader.hpp"

#ifdef STATIC_LOADER
#include "staticbackendloader.hpp"
#else
#include "dynamicbackendloader.hpp"
#endif
