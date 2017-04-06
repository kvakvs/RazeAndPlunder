#pragma once

#include "BaseAgent.h"
#include "MainAgents/WorkerAgentMsg.h"
#include "RnpStateMachine.h"

enum class WorkerState {
  // Worker is gathering minerals. 
  GATHER_MINERALS,
  // Worker is gathering gas. 
  GATHER_GAS,
  // Worker is trying to find a buildspot for a requested building. 
  FIND_BUILDSPOT,
  // Worker is moving to a found buildspot. 
  MOVE_TO_SPOT,
  // Worker is constructing a building. 
  CONSTRUCT,
  // Worker is repairing a building (Terran only). 
  REPAIRING,
  // Worker is needed to attack an enemy intruder in a base. 
  ATTACKING,
};

/** The WorkerAgent class handles all tasks that a worker, for example a Terran SCV, can perform. The tasks
 * involves gathering minerals and gas, move to a selected buildspot and construct the specified building,
 * and if Terran SCV, repair a building or tank.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class WorkerAgent : public BaseAgent
                  , public rnp::FiniteStateMachine<WorkerState> {
private:
  using BaseFsmClass = rnp::FiniteStateMachine<WorkerState>;

  BWAPI::UnitType to_build_;
  BWAPI::TilePosition build_spot_;
  BWAPI::TilePosition start_spot_;
  int start_build_frame_ = 0;
  int last_frame_ = 0;
  
  // The command center which we belong to
  act::ActorId assigned_cc_;

  bool is_build_spot_explored() const;

  void fsm_on_transition(WorkerState old_st, WorkerState new_st) override;

  bool is_built() const;

  bool check_repair();

  void compute_squad_worker_actions();

public:
  // Constructor. 
  explicit WorkerAgent(BWAPI::Unit unit);

  // Called each update to issue orders. 
  void tick() override;

  // Returns true if this agent is a free worker, i.e. is idle or is gathering minerals. 
  bool is_available_worker() const override;

  // Used to print info about this agent to the screen. 
  void debug_print_info() const override;

  // Used in debug modes to show a line to the agents' goal. 
  void debug_show_goal() const override;

  // Returns true if the Worker agent can create units of the specified type. 
  bool can_build(BWAPI::UnitType type) const;

  // Assigns the unit to construct a building of the specified type. 
  bool assign_to_build(BWAPI::UnitType type);

  // Returns the state of the agent as text. Good for printouts. 
  std::string get_state_as_text() const;

  // Called when the unit assigned to this agent is destroyed. 
  void destroyed();

  // Resets a worker to gathering minerals. 
  void reset();

  // Returns true if this worker is in any of the build states, and is constructing
  // the specified building. 
  bool is_constructing(BWAPI::UnitType type) const;

  // Assigns this worker to finish an unfinished building. 
  bool assign_to_finish_build(BWAPI::Unit building);

  // Assigns this worker to repair a building. 
  bool assign_to_repair(BWAPI::Unit building);

  // Reassign worker to another command center
  void change_command_center(const act::ActorId& ccid, 
                             const BWAPI::Position& pos);

  //
  // Actor section
  //
  void handle_message(act::Message *m) override;

private:
  void tick_attacking();
  void tick_repairing();
  void tick_gather();
  void tick_find_build_spot();
  void tick_move_to_spot();
  void tick_construct();
  void tick_gather_gas();

  // Called by movement handler when the worker does not go anywhere closer to
  // the goal over X=10 sec
  void on_worker_stuck();
};
