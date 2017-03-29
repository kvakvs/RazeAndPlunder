#pragma once

#include "../StructureAgent.h"

/** The HatcheryAgent handles Zerg Hatchery/Lair/Hive buildings.
 *
 * Implemented abilities:
 * - Trains and keeps the number of workers up.
 * - Can morph to Lair and Hive.
 * 
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class HatcheryAgent : public StructureAgent {
  bool has_sent_workers_ = false;
  
  bool check_build_unit(BWAPI::UnitType type);

public:
  explicit HatcheryAgent(BWAPI::Unit mUnit);

  // Called each update to issue orders. 
  void tick() override;
};
