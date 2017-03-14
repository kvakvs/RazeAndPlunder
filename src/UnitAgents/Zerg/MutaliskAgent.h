#pragma once

#include "../UnitAgent.h"

/** 
 * The MutaliskAgent handles abilities for Zerg Mutalisk units.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class MutaliskAgent : public UnitAgent {
public:
  explicit MutaliskAgent(Unit mUnit) : UnitAgent(mUnit) {
    agentType = "MutaliskAgent";
  }

  bool useAbilities() override;
};
