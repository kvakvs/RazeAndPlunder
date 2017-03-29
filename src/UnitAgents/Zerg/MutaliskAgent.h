#pragma once

#include "../UnitAgent.h"

/** 
 * The MutaliskAgent handles abilities for Zerg Mutalisk units.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class MutaliskAgent : public UnitAgent {
public:
  explicit MutaliskAgent(BWAPI::Unit mUnit) : UnitAgent(mUnit) {
    agent_type_ = "MutaliskAgent";
  }

  bool use_abilities() override;
};
