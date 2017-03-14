#pragma once

#include "../UnitAgent.h"

/** 
 * The ReaverAgent handles abilities for Protoss Reaver units.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class ReaverAgent : public UnitAgent {

private:

public:
  explicit ReaverAgent(BWAPI::Unit mUnit) : UnitAgent(mUnit) {
    agentType = "ReaverAgent";
  }

  bool useAbilities() override;
};
