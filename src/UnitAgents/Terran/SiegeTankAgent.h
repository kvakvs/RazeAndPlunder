#pragma once

#include "../UnitAgent.h"

/** 
 * The SiegeTankAgent handles abilities for Terran Siege Tank units.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class SiegeTankAgent : public UnitAgent {

public:
  explicit SiegeTankAgent(BWAPI::Unit mUnit) : UnitAgent(mUnit) {
    agent_type_ = "SiegeTankAgent";
  }

  bool use_abilities() override;
};
