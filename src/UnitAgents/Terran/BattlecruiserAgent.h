#pragma once

#include "../UnitAgent.h"


/** 
 * The BattlecruiserAgent handles abilities for Terran Battlecruiser flying units.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class BattlecruiserAgent : public UnitAgent {

private:
  int lastUseFrame;

public:
  BattlecruiserAgent(Unit mUnit) : UnitAgent(mUnit) {
    agentType = "BattlecruiserAgent";
  }

  bool useAbilities();
};
