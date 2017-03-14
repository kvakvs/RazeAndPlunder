#pragma once

#include "../UnitAgent.h"

/** 
 * The MarineAgent handles abilities for Terran Marine units.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class MarineAgent : public UnitAgent {

private:

public:
  explicit MarineAgent(BWAPI::Unit mUnit) : UnitAgent(mUnit) {
    agentType = "MarineAgent";
  }

  bool useAbilities() override;
};
