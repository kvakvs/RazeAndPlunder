#pragma once

#include <BWAPI.h>
#include <vector>
#include <memory>

#include <Actors/Actor.h>
#include <Actors/Scheduler.h>
#include "MainAgents/BaseAgentMsg.h"
#include "RnpConst.h"
#include "MainAgents/RnpMovementMonitor.h"

/** The BaseAgent is the base agent class all agent classes directly or indirectly must extend. It contains some
 * common logic that is needed for several other agent implementations.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class BaseAgent: public act::Actor {
protected:
  BWAPI::Unit unit_;
  BWAPI::UnitType type_;

  // A thingy which monitors whether we are getting any closer to the goal
  rnp::MovementProgressMonitor movement_progress_;
  BWAPI::TilePosition goal_;

  int unit_id_ = 0;
  act::ActorId squad_id_;
  bool alive_ = false;
//  bool bBlock;
  std::string agent_type_;

  int info_update_time_ = 0;
  static int info_update_frame_;
  static int debug_tooltip_x_;
  static int debug_tooltip_y_;
  int last_order_frame_ = 0;
  std::vector<BWAPI::WalkPosition> trail_;

public:
  uint32_t ac_flavour() const override {
    return static_cast<uint32_t>(ActorFlavour::Unit);
  }

  using Ptr = std::unique_ptr< BaseAgent >;
  struct Hasher {
    size_t operator () (const Ptr &ba) const {
      return std::hash<int>()(ba->unit_->getID());
    }
  };
  // Owning set of smart pointers
  using Set = std::unordered_set < Ptr, Hasher >;
  // Non owning set of simple pointers
  using PointerSet = std::unordered_set < BaseAgent*, std::hash<void*> >;
  using Map = std::unordered_map < int, Ptr, std::hash<int> > ;

  BaseAgent();
  BaseAgent(BWAPI::Unit mUnit);
  virtual ~BaseAgent();

  BaseAgent(const BaseAgent&) = delete;
  BaseAgent& operator=(const BaseAgent&) = delete;

  // Returns the frame the last order for the unit was issued. 
  int get_last_order_frame() const;

  // Called each update to issue orders. 
  void tick() override {}

  // Used in debug modes to show a line to the agents' goal. 
  virtual void debug_show_goal() const {
  }

  // Checks if there are any enemy units within sight range. 
  bool any_enemy_units_visible() const;

  // Sets the goal for this unit. Goals are set from either the SquadCommander 
  // for attacking or defending units, or from ExplorationManager for explore units. 
  void set_goal(BWAPI::TilePosition goal);

  // Clears the goal for this unit. 
  void clear_goal();

  // Returns the current goal for this unit. 
  const BWAPI::TilePosition& get_goal() const;

  // Returns the unique type name for the agent type. 
  const std::string& get_type_name() const;

  // Used to print info about this agent to the screen. 
  virtual void debug_print_info() const {    
  }

  // Returns the unique id for this agent. Agent id is the same as the id of the unit
  // assigned to the agent. 
  int get_unit_id() const;

  // Returns the type for the unit handled by this agent. 
  BWAPI::UnitType unit_type() const;

  // Returns a reference to the unit assigned to this agent. 
  BWAPI::Unit get_unit() const;

  // Called when the unit assigned to this agent is destroyed. 
  virtual void destroyed();

  // Returns true if this agent is active, i.e. the unit is not destroyed. 
  bool is_alive() const;

  // Returns true if the specified unit is the same unit assigned to this agent. 
  bool matches(BWAPI::Unit mUnit) const;

  // Returns true if the agent is of the specified type. 
  bool is_of_type(BWAPI::UnitType type) const;

  // Returns true if mType is the same UnitType as toCheckType. 
  //static bool is_of_type(BWAPI::UnitType mType, BWAPI::UnitType toCheckType);

  // Checks if there are any enemy detector units withing range of the
  // specified position. True if there is, false if not. 
  static bool is_enemy_detector_within_range(BWAPI::TilePosition pos, int range);

  // Returns true if this agent is a building. 
  bool is_building() const;

  // Returns true if this agent is a worker. 
  bool is_worker() const;

  // Returns true if this agent is a free worker, i.e. is idle or is gathering minerals. 
  virtual bool is_available_worker() const {
    return false;
  }

  // Returns true if this agent is a combat unit. 
  bool is_unit() const;

  // Returns true if this agent is under attack, i.e. lost hitpoints since last check. 
  bool is_under_attack() const;

  // Returns true if this agent is damaged. 
  bool is_damaged() const;

  // Assigns this agent to the squad with the specified id. 
  void set_squad_id(const act::ActorId& id) {
    squad_id_ = id;
  }

  // Returns the squad this agent belongs to, or -1 if it doesnt
  // belong to any squad. 
  const act::ActorId& get_squad_id() const {
    return squad_id_;
  }

  // Adds a new trail entry to a PF pheromone trail. 
  void add_trail_position(BWAPI::WalkPosition wt);

  // Returns the PF pheromone trail. 
  const std::vector<BWAPI::WalkPosition>& get_trail() const;

  bool can_target_air() const;
  bool can_target_ground() const;

  //
  // Actor section
  //
  void handle_message(act::Message *m) override;
private:
  void handle_message_squad_join(const msg::unit::JoinedSquad* sq_join);
  void handle_message_attack_bwunit(const msg::unit::AttackBWUnit* atk) const;
  void handle_message_attack_actor(msg::unit::Attack* atk) const;
};
