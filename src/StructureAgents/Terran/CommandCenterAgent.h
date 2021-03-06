#pragma once

#include "../StructureAgent.h"

/** The CommandCenterAgent handles Terran Command Center buildings.
 *
 * Implemented abilities:
 * - Trains and keeps the number of SCVs (workers) up. Is implemented in levels
 * where the preferred number of SCVs are higher at higher levels, i.e. later in
 * the game.
 * 
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class CommandCenterAgent : public StructureAgent {

private:
  bool has_sent_workers_ = false;

public:
  explicit CommandCenterAgent(BWAPI::Unit mUnit);

  // Called each update to issue orders. 
  void tick() override;
};
