#pragma once

#include "../UnitAgent.h"

/** 
 * The QueenAgent handles abilities for Zerg Queen units.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class QueenAgent : public UnitAgent {

private:

public:
  explicit QueenAgent(BWAPI::Unit mUnit) : UnitAgent(mUnit) {
    agent_type_ = "QueenAgent";
  }

  bool use_abilities() override;
};
