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
  explicit CarrierAgent(Unit mUnit) : UnitAgent(mUnit) {
    agentType = "CarrierAgent";
  }

  bool useAbilities();
};
