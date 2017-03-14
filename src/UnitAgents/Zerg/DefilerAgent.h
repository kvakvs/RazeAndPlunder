#pragma once

#include "../UnitAgent.h"

/** 
 * The DefilerAgent handles abilities for Zerg Defiler units.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class DefilerAgent : public UnitAgent {
  int darkSwarmFrame;

public:
  explicit DefilerAgent(BWAPI::Unit mUnit) : UnitAgent(mUnit) {
    agentType = "DefilerAgent";
    darkSwarmFrame = 0;
  }

  bool useAbilities() override;
};
