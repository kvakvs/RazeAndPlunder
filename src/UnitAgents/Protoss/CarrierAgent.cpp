#include "CarrierAgent.h"

using namespace BWAPI;

bool CarrierAgent::use_abilities() {
  if (Broodwar->canMake(UnitTypes::Protoss_Interceptor, unit_)) {
    if (unit_->train(UnitTypes::Protoss_Interceptor)) {
      return true;
    }
  }

  return false;
}
