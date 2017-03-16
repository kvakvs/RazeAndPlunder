#include "WraithAgent.h"
#include <iso646.h>

using namespace BWAPI;

bool WraithAgent::useAbilities() {
  //Cloaking
  TechType cloak = TechTypes::Cloaking_Field;
  if (Broodwar->self()->hasResearched(cloak) 
    && not unit_->isCloaked() 
    && unit_->getEnergy() >= 25 
    && not isDetectorWithinRange(unit_->getTilePosition(), 192)) 
  {
    int range = 10 * 32;

    //Check if enemy units are visible
    for (auto& u : Broodwar->enemy()->getUnits()) {
      if (u->exists()) {
        if (unit_->getDistance(u) <= range) {
          unit_->useTech(cloak);
          return true;
        }
      }
    }
  }

  return false;
}
