#pragma once

#include "../StructureAgent.h"

/** The ComsatAgent handles Terran Comsat Station buildings.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class ComsatAgent : public StructureAgent {
  int last_sweep_frame_ = 0;
  BWAPI::TilePosition last_sweep_pos_;

  int friendlyUnitsWithinRange(BWAPI::Position pos);
  bool anyHasSweeped(BWAPI::TilePosition pos);

public:
  explicit ComsatAgent(BWAPI::Unit mUnit);

  // Called each update to issue orders. 
  void tick() override;

  // Checks if this Comsat has sweeped the specified position within the
  // previous 100 frames.
  bool hasSweeped(BWAPI::TilePosition pos) const;
};
