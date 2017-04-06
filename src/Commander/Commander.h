#pragma once

#include "Squad.h"
#include "MainAgents/BaseAgent.h"
#include "Managers/BuildplanEntry.h"

#include "BWEM/bwem.h"
#include "RnpUtil.h"
#include "BWAPI/Position.h"
#include "Commander/CommanderMsg.h"

class CommanderStrategy: public act::Actor {
public:
  virtual ~CommanderStrategy() {}
  virtual size_t workers_per_refinery() const = 0;
  virtual size_t adjust_workers_count(size_t workers_now) const = 0;

  uint32_t ac_flavour() const override {
    return static_cast<uint32_t>(ActorFlavour::Singleton);
  }

  // Strategy takes no messages, but if it really wants, can override this
  void handle_message(act::Message* m) override {}
};

enum class CommanderAttackState {
  INITIALIZE,
  DEFEND,
  ATTACK
};

/** The Commander class is the base class for commanders. The Commander 
 * classes are responsible for which and when buildings to construct, when to 
 * do upgrades/techs, and which squads to build. It is also responsible for 
 * finding defensive positions, launch attacks and where to launch an 
 * attack.
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class Commander: public act::Actor
               , public rnp::FiniteStateMachine<CommanderAttackState> {
public:
  using AttackDefendFsm = rnp::FiniteStateMachine<CommanderAttackState>;
  uint32_t ac_flavour() const override {
    return static_cast<uint32_t>(ActorFlavour::Singleton);
  }

private:
  bool removal_done_ = false;

  rnp::Memoize<bool> m_time_to_engage_;

private:
  // NYI bool chokePointFortified(BWAPI::TilePosition center);
//  act::ActorId::Vector sort_squad_list();

  static BWAPI::TilePosition find_defense_pos(const BWEM::ChokePoint* choke);

  void check_no_squad_units();

  void assign_unit(const BaseAgent* agent);

public:
  Buildplan build_plan_;
  act::ActorId::Set squads_;

protected:
  bool debug_bp_ = false;
  bool debug_sq_ = false;

  size_t workers_num_ = 0;
  size_t workers_per_refinery_ = 0;

  // Checks the Commander buildplan, and add buildings,
  // techs and upgrades to the planners. 
  void check_buildplan();

  // Stops the production of workers. 
  void cut_workers_production();

public:
  Commander();
  virtual ~Commander();

  // Executes basic code for a commander. 
  void tick_base_commander();

  // Returns the instance of the class. 
//  static Commander* getInstance();

  // Switch on/off buildplan debug info printing to screen. 
  void toggle_buildplan_debug();

  // Switch on/off squads debug info printing to screen. 
  void toggle_squads_debug();

  // Called each update to issue orders. 
  void tick() override;

  // Returns the number of preferred workers, i.e. the
  // number of workers should be built. 
  size_t get_preferred_workers_count() const;

  // Returns the preferred number of workers for a refinery. 
  size_t get_workers_per_refinery() const;

  // Used in debug modes to show goal of squads. 
  virtual void debug_show_goal() const;

  // Checks if it is time to engage the enemy. This happens when all Required squads
  // are active. 
  bool is_time_to_engage();

  // Updates the goals for all squads. 
  void update_squad_goals();

  // Removes a squad. 
  void remove_squad(const act::ActorId& id);

  // Adds a new squad. 
  void add_squad(const act::ActorId& sq);

  // Returns the Squad with the specified id, or nullptr if not found. 
  const Squad* get_squad(const act::ActorId& a) const;

  // Returns the position where to launch an attack at. 
  BWAPI::TilePosition find_attack_position();

  // Checks if workers needs to attack. Happens if base is under attack
  // and no offensive units are available.
  bool check_workers_attack(const BaseAgent* base) const;

  // Checks if we need to assist a building. 
  bool assist_building();

  // Checks if we need to assist a worker that is under attack. 
  bool assist_worker();

  // Checks if there are any removable obstacles nearby, i.e. minerals with 
  // less than 20 resources left. 
  void check_removable_obstacles();

  // Forces an attack, even if some squads are not full. 
  void force_begin_attack();

  // Shows some info on the screen. 
  void debug_print_info() const;

  // Searches for and returns a good chokepoint position to defend the territory. 
  static BWAPI::TilePosition find_chokepoint();

  // Checks if there are any unfinished buildings that does not have an SCV working on them. Terran only. 
  static bool check_damaged_buildings();

  // Assigns a worker to finish constructing an interrupted building. Terran only. 
  static void assist_unfinished_construction(const BaseAgent* agent);

  // Adds a bunker squad when a Terran Bunker has been created. Returns
  // the squadID of the bunker squad. 
  static act::ActorId add_bunker_squad();

  // Removes a bunker squad. Used when the bunker is destroyed
  // with units inside. 
  bool remove_bunker_squad(int unitID);

  void handle_message(act::Message* incoming) override;

  // Commander does not react on defend/attack switch, but it potentially can
  void fsm_on_transition(CommanderAttackState old_st, 
                         CommanderAttackState new_st) override {}
private:
  // Called each time a unit is created. The unit is then
  // placed in a Squad. 
  void on_unit_created(const act::ActorId& agentid);

  // Called each time a unit is destroyed. The unit is then
  // removed from its Squad. 
  void on_unit_destroyed(const act::ActorId& agent);
  void tick_base_commander_attack_maybe_defend();
  void tick_base_commander_defend();
  void tick_base_commander_attack();
};
