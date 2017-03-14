#pragma once

#include "../StructureAgent.h"
using namespace BWAPI;


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
  bool checkBuildUnit(UnitType type);

public:
  explicit HatcheryAgent(Unit mUnit);

  /** Called each update to issue orders. */
  void computeActions() override;
};
