#pragma once

#include "../UnitAgent.h"

/** 
 * The HydraliskAgent handles abilities for Zerg Hydralisk units.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class HydraliskAgent : public UnitAgent {
public:
  explicit HydraliskAgent(Unit mUnit) : UnitAgent(mUnit) {
    agentType = "HydraliskAgent";
  }

  bool useAbilities() override;
};
