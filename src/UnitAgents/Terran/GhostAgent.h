#pragma once

#include "../UnitAgent.h"

/** 
 * The GhostAgent handles abilities for Terran Ghost units.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class GhostAgent : public UnitAgent {
  Unit findLockdownTarget();
  /** Returns the number of own units that are within maxRange of the agent. */
  int friendlyUnitsWithinRange(int maxRange);

public:
  explicit GhostAgent(Unit mUnit) : UnitAgent(mUnit) {
    agentType = "GhostAgent";
  }

  bool useAbilities() override;
};
