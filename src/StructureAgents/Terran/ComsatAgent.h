#pragma once

#include "../StructureAgent.h"

/** The ComsatAgent handles Terran Comsat Station buildings.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class ComsatAgent : public StructureAgent {
  int friendlyUnitsWithinRange(BWAPI::Position pos);
  int lastSweepFrame;
  BWAPI::TilePosition lastSweepPos;
  bool anyHasSweeped(BWAPI::TilePosition pos);

public:
  explicit ComsatAgent(BWAPI::Unit mUnit);

  // Called each update to issue orders. 
  void computeActions() override;

  // Checks if this Comsat has sweeped the specified position within the previos 100 frames. 
  bool hasSweeped(BWAPI::TilePosition pos);
};
