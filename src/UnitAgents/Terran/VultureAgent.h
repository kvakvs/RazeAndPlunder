#pragma once

#include "../UnitAgent.h"

/** 
 * The VultureAgent handles abilities for Terran Vulture units.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class VultureAgent : public UnitAgent {

private:
  int mine_drop_frame_ = 0;

public:
  explicit VultureAgent(BWAPI::Unit mUnit) : UnitAgent(mUnit) {
    agent_type_ = "VultureAgent";
  }

  bool use_abilities() override;
};
