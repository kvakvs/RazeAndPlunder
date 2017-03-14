#pragma once

#include "../UnitAgent.h"

/** 
 * The LurkerAgent handles abilities for Zerg Lurker units.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class LurkerAgent : public UnitAgent {
public:
  explicit LurkerAgent(Unit mUnit) : UnitAgent(mUnit) {
    agentType = "LurkerAgent";
  }

  bool useAbilities() override;
};
