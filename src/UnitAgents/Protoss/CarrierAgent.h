#pragma once

#include "../UnitAgent.h"

/** 
 * The CarrierAgent handles abilities for Protoss Carrier flying units.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class CarrierAgent : public UnitAgent {

private:

public:
  explicit CarrierAgent(BWAPI::Unit mUnit) : UnitAgent(mUnit) {
    agent_type_ = "CarrierAgent";
  }

  bool useAbilities();
};
