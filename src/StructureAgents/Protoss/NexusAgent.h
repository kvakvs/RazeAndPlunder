#pragma once

#include "../StructureAgent.h"

/** The NexusAgent handles Protoss Nexus buildings.
 * 
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class NexusAgent : public StructureAgent {
  bool has_sent_workers_ = false;

public:
  explicit NexusAgent(BWAPI::Unit mUnit);

  // Called each update to issue orders. 
  void tick() override;
};
