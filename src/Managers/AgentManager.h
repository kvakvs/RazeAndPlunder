#pragma once

#include "MainAgents/BaseAgent.h"

#include "Utils/Sets.h"
//#include <BWAPI/UnitCommand.h>
//#include <Actors/Scheduler.h>
//#include "MainAgents/AgentFactory.h"
#include "Managers/AgentManagerMsg.h"

/** The AgentManager class is a container that holds a list of all active agents in the game. Each unit, worker, building or
 * or addon is assigned to an agent. See the MainAgents, StructureAgents and UnitAgents folders for detailed information
 * about each specific type of agent.
 *
 * The AgentManager is implemented as a singleton class. Each class that needs to access AgentManager can request an instance,
 * and all classes shares the same AgentManager instance.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */

class AgentManager: public act::Actor {
private:
//  BaseAgent::Map agents_; // moved to actor/scheduler
//  BaseAgent::Map dead_agents_;
  int last_call_frame_ = 0;

public:
  AgentManager();
  ~AgentManager();

  uint32_t ac_flavour() const override {
    return static_cast<uint32_t>(ActorFlavour::Singleton);
  }

  // Adds an agent to the container. Is called each time a new
  // unit is built. 
  void add_agent(BWAPI::Unit unit) const;

  // Called when a Zerg Drone is morphed into another unit. 
  void on_drone_morphed(BWAPI::Unit unit);

  // Called each update to issue commands from all active agents. 
  void tick() override;

  // Returns the current number of active worker units. 
  size_t get_workers_count() const;

  // Returns the current number of active workers gathering minerals. 
  size_t get_mining_workers_count() const;

  // Returns the closest free worker from the specified position, or nullptr if not found. 
  const BaseAgent* 
    find_closest_free_worker(const BWAPI::TilePosition& pos) const;

  // Checks if any agent has the task to repair this specified agent. 
  bool is_any_agent_repairing_this_agent(const BaseAgent* repairedAgent) const;

  // Checks if we have a completed building of the specified type 
  bool have_a_completed_building(BWAPI::UnitType type) const;

  // Returns the number of own units of a specific type. Includes units
  // currently being constructed. 
  size_t get_units_of_type_count(BWAPI::UnitType type) const;

  // Returns the number of own units of a specific type. Does not include units
  // in construction. 
  size_t get_finished_units_count(BWAPI::UnitType type) const;

  // Returns the number of units or buildings of the specified type
  // that currently is in production. 
  size_t get_in_production_count(BWAPI::UnitType type) const;

  // Returns the number of bases the player has. 
  size_t get_bases_count() const;

  // Returns a list of all agents in the container. 
//  const BaseAgent::Map& get_alive() const {
//    return agents_;
//  }
//  const BaseAgent::Map& get_dead() const {
//    return dead_agents_;
//  }
//  void for_each_alive(std::function<void(const BaseAgent*)> fn) {
//    act::for_each_alive<BaseAgent>(fn);
//  }

  // Returns a reference to the agent associated with a specific unit,
  // or nullptr if the unit doesn't exist. 
  const BaseAgent* get_agent(int unitID) const;

  // Returns the closest agent in the list of the specified type, or nullptr if not found. 
  const BaseAgent* get_closest_agent(BWAPI::TilePosition pos, BWAPI::UnitType type) const;

  // Returns the closest base agent (Terran Command Center, Protoss Nexus), in the list,
  // or nullptr if not found. 
  const BaseAgent* get_closest_base(const BWAPI::TilePosition& pos) const;

  // Checks if a worker is targeting the specified target. 
  bool is_worker_targeting_unit(BWAPI::Unit target) const;

  // Removes inactive agents from the container. Shouldn't be called too often. 
  //void cleanup();

  // Called each time an unit is destroyed
  void on_unit_destroyed(BWAPI::Unit dead);

  //
  // Actor stuff
  //

  void handle_message(act::Message* incoming) override;
private:
};

