#pragma once

#include "UnitSetup.h"
#include "Pathfinding/Pathfinder.h"
#include <memory>
#include "RnpConst.h"
#include "RnpStateMachine.h"

enum class SquadState {
  NOT_SET, ATTACK, DEFEND, ASSIST,
};

/** The Squad class represents a squad of units with a shared goal, for example
 * attacking the enemy or defending the base. The Squad can be built up from
 * different combinations and numbers of UnitTypes. 
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class Squad : public act::Actor
            , public rnp::FiniteStateMachine<SquadState> {
public:
  using FsmBaseClass = rnp::FiniteStateMachine<SquadState>;

  uint32_t ac_flavour() const override {
    return static_cast<uint32_t>(ActorFlavour::Squad);
  }

  using Ptr = std::shared_ptr < Squad > ;
  enum class SquadType {
    UNDEFINED, OFFENSIVE, DEFENSIVE, EXPLORER, SUPPORT, BUNKER, SHUTTLE, KITE,
    RUSH, CHOKEHARASS
  };

  enum class MoveType {
    GROUND, AIR
  };

protected:
  act::ActorId::Set members_;

  std::vector<UnitSetup> setup_;
  BWAPI::UnitType morphs_ = BWAPI::UnitTypes::Unknown;

  //
  // Pathing
  //
  BWAPI::TilePosition goal_;
  rnp::PathAB::Ptr path_;
  rnp::PathAB::Iterator::Ptr path_iter_;

  int arrived_frame_ = -1;
  int bunker_id_ = -1;

  bool active_ = false;
  SquadType type_ = SquadType::UNDEFINED;
  int priority_ = 0;
  int active_priority_ = 0;
  MoveType move_type_ = MoveType::AIR;
  bool required_ = false;
  std::string name_;
  int goal_set_frame_ = 0;
  bool buildup_ = false;

  // Make all squad members want to move
  void set_member_goals(BWAPI::TilePosition cGoal);

public:
  virtual ~Squad() = default;

  /** Creates a squad with a unique id, a type (Offensive, Defensive, Exploration, Support),
   * a name (for example AirAttackSquad, MainGroundSquad).
   * Higher priority squads gets filled before lower prio squads. Lower prio value is considered
   * higher priority. A squad with priority of 1000 or more will not be built. This can be used
   * to create one-time squads that are only filled once.
   */
  Squad(SquadType mType, std::string mName, int mPriority);

  // Used from next_move_position to update the arrived frame
  void set_arrived_frame(int af) { arrived_frame_ = af;  }

  // Returns the name of this Squad. 
  const std::string& get_name() const {
    return name_;
  }

  // Returns the members of this squad. 
  const act::ActorId::Set& get_members() const {
    return members_;
  }

  // Returns the unit squad members shall morph to (for example Archons or Lurkers)
  // UnitType::Unknown if no morph is set. 
  BWAPI::UnitType morphs_to() const {
    return morphs_;
  }

  // Sets the unit squad members shall morph to (for example Archons or Lurkers) 
  void set_morph_to(BWAPI::UnitType type) {
    morphs_ = type;
  }

  // Checks if this Squad is required to be active before an attack is launched. 
  bool is_required_for_attack() const {
    return required_;
  }

  // Sets if this Squad is required or not. 
  void set_required(bool required) {
    required_ = required;
  }

  // Sets if the squad is during buildup. Buildup means it wont
  // be set to Active. 
  void set_buildup(bool buildup) {
    buildup_ = buildup;
  }

  // Returns the priority for this Squad. Prio 1 is the highest. 
  int get_priority() const {
    return priority_;
  }

  // Sets the priority for this Squad. Prio 1 is the highest. 
  void set_priority(int mPriority) {
    priority_ = mPriority;
  }

  // Sets the priority this Squad has once it has been active. Prio 1 is the highest. 
  void set_active_priority(int mPriority) {
    active_priority_ = mPriority;
  }

  // Adds a setup for this Squad. Setup is a type and amount of units
  // that shall be in this Squad. 
  void add_setup(BWAPI::UnitType type, int no);

  // Removes a setup for this squad. 
  void remove_setup(BWAPI::UnitType type, int no);

  /** Returns true if this Squad is active, or false if not.
   * A Squad is active when it first has been filled with agents.
   * A Squad with destroyed units are still considered Active. */
  virtual bool is_active() const {
    return active_;
  }

  // Forces the squad to be active. 
  void force_active() {
    active_priority_ = priority_;
    active_ = true;
  }

  // Returns true if this Squad is full, i.e. it has all the units
  // it shall have. 
  bool is_full() const;

  // Returns the current size (i.e. number of alive agents in the squad).
  size_t size() const {
    // Assumed that all listed squad members are alive, if they die we delete them
    // via monitor notification message
    return members_.size();
  }
  bool empty() const {
    return members_.empty();
  }

  // Returns the maximum size of this squad (i.e. size of a full squad).
  size_t get_max_size();

  // Called each update to issue orders. 
  void tick() override final;
  virtual void tick_inactive();
  virtual void tick_active();

  // Sets the goal for this Squad. 
  void set_goal(BWAPI::TilePosition m_goal);

  // Used in debug modes to show goal of squads. 
  void debug_show_goal() const;

  // Checks if the squad is attacking, i.e. if any member of the squad has targets within range. 
  bool is_attacking() const;

  // Returns true if this Squad is under attack. 
  bool is_under_attack();

  // Check if this Squad need units of the specified type. 
  bool need_unit(BWAPI::UnitType type) const;

  // Adds an agent to this Squad. 
  bool add_member(const act::ActorId& agent);

  // Removes an agent from this Squad. 
  void remove_member(const BaseAgent* agent);

  // Removes an agent of the specified type from this Squad,
  // and returns the reference to the removed agent. 
  act::ActorId remove_member(BWAPI::UnitType type);

  // Disbands this squad. 
  void disband();

  // Orders this squad to defend a position. 
  virtual void defend(BWAPI::TilePosition m_goal);

  // Orders this squad to launch an attack at a position. 
  virtual void attack(BWAPI::TilePosition m_goal);

  // Orders this squad to assist units at a position. 
  virtual void assist(BWAPI::TilePosition m_goal);

  // Clears the goal for this Squad, i.e. sets the goal
  // to TilePosition(-1,-1). 
  virtual void clear_goal();

  // Returns the current goal of this Squad. 
  virtual BWAPI::TilePosition get_goal() const {
    return goal_;
  }

  // Returns the position to be moved to for units following
  // a squad. 
  BWAPI::TilePosition next_follow_move_position() const;

  // Returns the next TilePosition to move to. 
  BWAPI::TilePosition next_move_position() const;

  // Returns true if this squad has an assigned goal. 
  virtual bool has_goal() const;

  // Returns the center position of this Squad, i.e. the
  // average x and y position of its members. 
  BWAPI::TilePosition get_center() const;

  // Returns true if this is an Offensive Squad. 
  bool is_offensive_squad() const;

  // Returns true if this is a Defensive Squad. 
  bool is_defensive_squad() const;

  // Returns true if this is an Explorer Squad. 
  bool is_explorer_squad() const;

  // Returns true if this is a Bunker Defense Squad. 
  bool is_bunker_defend_squad() const;

  // Sets the bunker id for this squad (if any). 
  void set_bunker_id(int unitID) {
    bunker_id_ = unitID;
  }

  // Returns the bunker id for this squad (if any). 
  int get_bunker_id() const {
    return bunker_id_;
  }

  // Returns true if this is a Shuttle Squad. 
  bool is_shuttle_squad() const;

  // Returns true if this is a Kite Squad. 
  bool is_kite_squad() const;

  // Returns true if this is a Rush Squad. 
  bool is_rush_squad() const;

  // Returns true if this is a Support Squad (for
  // example Transports). 
  bool is_support_squad() const;

  // Returns true if this squad travels by ground. 
  bool is_ground() const;

  // Returns true if this squad travels by air. 
  bool is_air() const;

  // Returns the size of the Squad, i.e. the number
  // if agents currently in it. 
  int get_squad_size() const;

  // Returns the total number of units in the squad when it is full. 
  int get_total_units() const;

  // Returns the current strength of the Squad, i.e.
  // the sum of the destroyScore() for all Squad members. 
  int get_strength() const;

  // Used to print some info to the screen. 
  virtual void debug_print_info() const;

  // Returns true if this Squad has the number of the specified
  // unit types in it. 
  bool has_units(BWAPI::UnitType type, int no);

  std::string string() const {
    return "Sq" + self().string() + " " + name_;
  }

  // Reported by agents when they believe they can't move, or they can move
  // but don't get any closer to the goal in 10 sec.
  void on_squad_member_stuck(const act::ActorId& who_id);

  void fsm_on_transition(SquadState old_st, SquadState new_st) override;

  //
  // Actor stuff
  //
  static void modify(const act::ActorId& id, std::function<void(Squad*)> f) {
    act::modify_actor<Squad>(id, f);
  }

  template <typename... Args>
  static act::ActorId spawn(Args&& ...args) {
    return act::spawn<Squad>(ActorFlavour::Squad, std::forward<Args>(args)...);
  }

  void handle_message(act::Message* incoming) override;

protected:
  void fill_with_free_workers();
};
