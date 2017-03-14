#pragma once

#include "../MainAgents/BaseAgent.h"

using namespace BWAPI;



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
  bool repairing;

  bool canBuildUnit(UnitType type);
  bool canEvolveUnit(UnitType type);

  std::vector<TilePosition> hasScanned;

  /** Checks if the specified unit/building can be constructed */
  bool canBuild(UnitType type);

public:
  StructureAgent();
  explicit StructureAgent(Unit mUnit);
  virtual ~StructureAgent();

  /** Called each update to issue orders. */
  void computeActions() override;

  /** Used in debug modes to show a line to the agents' goal. */
  void debug_showGoal() override;

  /** Checks if the agent can morph into the specified type. Zerg only. */
  bool canMorphInto(UnitType type);

  /** Sends a number of workers to a newly constructed base. */
  void sendWorkers();

  /** Used to print info about this agent to the screen. */
  void printInfo() override;
};
