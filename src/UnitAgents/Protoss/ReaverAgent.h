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
    agent_type_ = "ReaverAgent";
  }

  bool useAbilities() override;
};
