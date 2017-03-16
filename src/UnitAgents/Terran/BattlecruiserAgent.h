#pragma once

#include "../UnitAgent.h"

/** 
 * The BattlecruiserAgent handles abilities for Terran Battlecruiser flying units.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class BattlecruiserAgent : public UnitAgent {

private:
  int lastUseFrame = 0;

public:
  explicit BattlecruiserAgent(BWAPI::Unit mUnit) : UnitAgent(mUnit) {
    agent_type_ = "BattlecruiserAgent";
  }

  bool useAbilities() override;
};
