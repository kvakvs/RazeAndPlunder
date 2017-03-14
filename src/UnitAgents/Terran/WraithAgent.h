#pragma once

#include "../UnitAgent.h"

/** 
 * The WraithAgent handles abilities for Terran Wraith flying units.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class WraithAgent : public UnitAgent {
public:
  explicit WraithAgent(BWAPI::Unit mUnit) : UnitAgent(mUnit) {
    agentType = "WraithAgent";
  }

  bool useAbilities() override;
};
