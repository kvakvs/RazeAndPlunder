#pragma once

#include "../StructureAgent.h"

/** The NexusAgent handles Protoss Nexus buildings.
 * 
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class NexusAgent : public StructureAgent {
  bool hasSentWorkers = false;

public:
  explicit NexusAgent(Unit mUnit);

  /** Called each update to issue orders. */
  void computeActions() override;
};
