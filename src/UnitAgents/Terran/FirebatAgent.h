#pragma once

#include "../UnitAgent.h"

/** 
 * The FirebatAgent handles abilities for Terran Firebat units.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class FirebatAgent : public UnitAgent {
public:
  explicit FirebatAgent(Unit mUnit) : UnitAgent(mUnit) {
    agentType = "FirebatAgent";
  }

  bool useAbilities() override;
};
