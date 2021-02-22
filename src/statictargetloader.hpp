#include "targetloader.hpp"

#include <nullpointertarget.hpp>
#include <localtarget.hpp>

std::vector<std::shared_ptr<Target>> loadTargets(){
  
  std::vector<std::shared_ptr<Target>> targets;
  
  for(Target* target: NullPointerTarget::loadTargets()){
    targets.push_back(std::shared_ptr<Target>(target));
  }
  
  for(Target* target: LocalTarget::loadTargets()){
    targets.push_back(std::shared_ptr<Target>(target));
  }

  return targets;
}
