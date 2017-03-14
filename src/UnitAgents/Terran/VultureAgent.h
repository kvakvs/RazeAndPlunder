#pragma once

#include "../UnitAgent.h"

/** 
 * The VultureAgent handles abilities for Terran Vulture units.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class VultureAgent : public UnitAgent {

private:
  int mineDropFrame;

public:
  explicit VultureAgent(BWAPI::Unit mUnit) : UnitAgent(mUnit) {
    agentType = "VultureAgent";
    mineDropFrame = 0;
  }

  bool useAbilities() override;
};
