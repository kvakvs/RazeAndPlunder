#pragma once

#include "BaseAgent.h"
#include <BWAPI.h>

/** The agent system is built from a single base agent where all specific agents extends the base agent indirectly or directly.
 * The AgentFactory class is a factory that creates the correct BaseAgent instance for a specific unit. This class shall always
 * be used when a new agent is requested.
 *
 * The AgentFactory is implemented as a singleton class. Each class that needs to access AgentFactory can request an instance,
 * and all classes shares the same AgentFactory instance.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class AgentFactory {

private:
  AgentFactory();
  static AgentFactory* instance_;
  static bool instance_flag_;

  act::ActorId create_terran_agent(BWAPI::Unit unit) const;
  act::ActorId create_protoss_agent(BWAPI::Unit unit) const;
  act::ActorId create_zerg_agent(BWAPI::Unit unit) const;

public:
  ~AgentFactory();

  // Returns the instance to the class. 
  static AgentFactory* get_instance();

  // Creates the BaseAgent 
  act::ActorId create_agent(BWAPI::Unit unit);

  // Returns true if the unit is of the specified type. 
  static bool is_of_type(BWAPI::Unit unit, BWAPI::UnitType type);

};
