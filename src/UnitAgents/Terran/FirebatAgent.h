#pragma once

#include "../UnitAgent.h"

/** 
 * The FirebatAgent handles abilities for Terran Firebat units.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class FirebatAgent : public UnitAgent {
public:
  explicit FirebatAgent(BWAPI::Unit mUnit) : UnitAgent(mUnit) {
    agent_type_ = "FirebatAgent";
  }

  bool use_abilities() override;
};
