#pragma once

#include "../UnitAgent.h"

/** 
 * The DefilerAgent handles abilities for Zerg Defiler units.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class DefilerAgent : public UnitAgent {
  int dark_swarm_frame_ = 0;

public:
  explicit DefilerAgent(BWAPI::Unit mUnit)
      : UnitAgent(mUnit)
  {
    agent_type_ = "DefilerAgent";
  }

  bool use_abilities() override;
};
