#pragma once

#include "../UnitAgent.h"

/** 
 * The LurkerAgent handles abilities for Zerg Lurker units.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class LurkerAgent : public UnitAgent {
public:
  explicit LurkerAgent(BWAPI::Unit mUnit) : UnitAgent(mUnit) {
    agent_type_ = "LurkerAgent";
  }

  bool use_abilities() override;
};
