#include "ReaverAgent.h"

using namespace BWAPI;

bool ReaverAgent::useAbilities() {
  int maxLoad = 5;
  if (Broodwar->self()->getUpgradeLevel(UpgradeTypes::Reaver_Capacity) > 0) {
    maxLoad = 10;
  }

  if (unit_->getScarabCount() < maxLoad) {
    if (unit_->train(UnitTypes::Protoss_Scarab)) {
      return true;
    }
  }

  return false;
}
