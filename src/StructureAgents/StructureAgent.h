#pragma once

#include "MainAgents/BaseAgent.h"

/** The StructureAgent is the base agent class for all agents handling buildings. If a building is created and no
 * specific agent for that type is found, the building is assigned to a StructureAgent. StructureAgents are typically
 * agents without logic, for example supply depots. To add logic to a building, for example Terran Academy researching
 * stim packs, an agent implementation for that unit type must be created.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class StructureAgent : public BaseAgent {

private:

protected:
  bool repairing_ = false;

  bool can_build_unit(BWAPI::UnitType type) const;
  bool can_evolve_unit(BWAPI::UnitType type) const;

  //std::vector<BWAPI::TilePosition> hasScanned;

  // Checks if the specified unit/building can be constructed 
  bool can_build(BWAPI::UnitType type) const;

public:
  StructureAgent();
  explicit StructureAgent(BWAPI::Unit mUnit);
  virtual ~StructureAgent();

  // Called each update to issue orders. 
  void tick() override;

  // Used in debug modes to show a line to the agents' goal. 
  void debug_show_goal() const override;

  // Checks if the agent can morph into the specified type. Zerg only. 
  bool can_morph_into(BWAPI::UnitType type) const;

  // Sends a number of workers to a newly constructed base. 
  void send_workers();

  // Used to print info about this agent to the screen. 
  void debug_print_info() const override;
};
