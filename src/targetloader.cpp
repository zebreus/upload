#include "targetloader.hpp"

#ifdef STATIC_LOADER
#include "statictargetloader.hpp"
#else
#include "dynamictargetloader.hpp"
#endif
