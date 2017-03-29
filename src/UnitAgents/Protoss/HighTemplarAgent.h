#pragma once

#include "../UnitAgent.h"

/** 
 * The HighTemplarAgent handles abilities for Protoss High Templar units.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class HighTemplarAgent : public UnitAgent {

private:
  bool has_cast_transform_ = false;

  BWAPI::Unit find_psi_storm_target();

  const BaseAgent* find_hallucination_target() const;

  const BaseAgent* find_archon_target() const;

  // Returns the number of own units that are within maxRange of the specified tile. 
  static int get_friendly_units_within_range(BWAPI::TilePosition tilePos, int maxRange);

public:
  explicit HighTemplarAgent(BWAPI::Unit mUnit) : UnitAgent(mUnit) {
    agent_type_ = "HighTemplarAgent";
  }

  bool use_abilities() override;
};
