#pragma once

#include "../UnitAgent.h"

/** 
 * The CorsairAgent handles abilities for Protoss Corsair flying units.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class CorsairAgent : public UnitAgent {
  int lastUseFrame = 0;
  Unit getClosestEnemyAirDefense(int maxRange);

public:
  explicit CorsairAgent(Unit mUnit) : UnitAgent(mUnit) {
    agentType = "CorsairAgent";
  }

  bool useAbilities() override;
};
