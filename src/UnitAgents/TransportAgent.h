#pragma once

#include "UnitAgent.h"

/** The TransportAgent handles transport units (Terran Dropship and Protoss Shuttle).
 *
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class TransportAgent : public UnitAgent {

private:
  int max_load_ = 0;
  int current_load_ = 0;

  int get_current_load();

  bool is_valid_load_unit(const BaseAgent* a) const;

  const BaseAgent* find_unit_to_load(int spaceLimit);

public:
  explicit TransportAgent(BWAPI::Unit mUnit);

  // Called each update to issue orders. 
  void tick() override;
};
