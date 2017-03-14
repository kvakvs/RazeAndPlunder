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
  bool hasSentWorkers = false;
  bool checkBuildUnit(BWAPI::UnitType type);

public:
  explicit HatcheryAgent(BWAPI::Unit mUnit);

  // Called each update to issue orders. 
  void computeActions() override;
};
