#pragma once

#include "../UnitAgent.h"

/** 
 * The CorsairAgent handles abilities for Protoss Corsair flying units.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class CorsairAgent : public UnitAgent {
  int lastUseFrame = 0;
  BWAPI::Unit getClosestEnemyAirDefense(int maxRange);

public:
  explicit CorsairAgent(BWAPI::Unit mUnit) : UnitAgent(mUnit) {
    agent_type_ = "CorsairAgent";
  }

  bool use_abilities() override;
};
