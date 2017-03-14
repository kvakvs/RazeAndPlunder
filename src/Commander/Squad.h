#pragma once

#include "UnitSetup.h"
#include "../Utils/Sets.h"
#include "Pathfinding/Pathfinder.h"
#include <memory>

/** The Squad class represents a squad of units with a shared goal, for example
 * attacking the enemy or defending the base. The Squad can be built up from
 * different combinations and numbers of UnitTypes. 
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class Squad {
public:
  using Ptr = std::shared_ptr < Squad > ;
  enum class SquadType {
    // Offensive Squad 
    OFFENSIVE = 0,
    // Defensive Squad 
    DEFENSIVE = 1,
    // Explorer Squad 
    EXPLORER = 2,
    // Support Squad 
    SUPPORT = 3,
    // Bunker Defense Squad 
    BUNKER = 4,
    // Shuttle Squad 
    SHUTTLE = 5,
    // Kite Squad 
    KITE = 6,
    // Rush Squad 
    RUSH = 7,
    // ChokeHarass Squad 
    CHOKEHARASS = 8
  };

  enum class MoveType {
    // Ground Squad 
    GROUND = 0,
    // Air Squad 
    AIR = 1
  };

  enum class State {
    // Squad is attacking 
    ATTACK = 1,
    // Squad is defending 
    DEFEND = 2,
    // Squad is assisting 
    ASSIST = 3,
    // No state is set 
    NOT_SET = 4
  };

protected:
  Agentset agents_;
  std::vector<UnitSetup> setup_;
  BWAPI::UnitType morphs_ = BWAPI::UnitTypes::Unknown;

  BWAPI::TilePosition goal_;
  BWEM::CPPath path_;
  int path_index_ = 0;
  int arrived_frame_ = -1;
  int bunker_id_ = -1;
  int id_;
  bool active_ = false;
  SquadType type_;
  int priority_;
  int active_priority_;
  MoveType move_type_ = MoveType::AIR;
  bool required_ = false;
  std::string name_;
  int goal_set_frame_ = 0;
  State current_state_ = State::DEFEND;
  bool buildup_ = false;

  void setMemberGoals(BWAPI::TilePosition cGoal);
  void removeDestroyed();

public:
  virtual ~Squad() = default;

  /** Creates a squad with a unique id, a type (Offensive, Defensive, Exploration, Support),
   * a name (for example AirAttackSquad, MainGroundSquad).
   * Higher priority squads gets filled before lower prio squads. Lower prio value is considered
   * higher priority. A squad with priority of 1000 or more will not be built. This can be used
   * to create one-time squads that are only filled once.
   */
  Squad(int mId, SquadType mType, std::string mName, int mPriority);

  // Returns the id for this Squad. 
  int getID() const;

  // Returns the name of this Squad. 
  std::string getName() const;

  // Returns the members of this squad. 
  const Agentset& getMembers() const;

  // Returns the unit squad members shall morph to (for example Archons or Lurkers)
  // UnitType::Unknown if no morph is set. 
  BWAPI::UnitType morphsTo() const;

  // Sets the unit squad members shall morph to (for example Archons or Lurkers) 
  void setMorphsTo(BWAPI::UnitType type);

  // Checks if this Squad is required to be active before an attack is launched. 
  bool isRequired() const;

  // Sets if this Squad is required or not. 
  void setRequired(bool mRequired);

  // Sets if the squad is during buildup. Buildup means it wont
  // be set to Active. 
  void setBuildup(bool mBuildup);

  // Returns the priority for this Squad. Prio 1 is the highest. 
  int getPriority() const;

  // Sets the priority for this Squad. Prio 1 is the highest. 
  void setPriority(int mPriority);

  // Sets the priority this Squad has once it has been active. Prio 1 is the highest. 
  void setActivePriority(int mPriority);

  // Adds a setup for this Squad. Setup is a type and amount of units
  // that shall be in this Squad. 
  void addSetup(BWAPI::UnitType type, int no);

  // Removes a setup for this squad. 
  void removeSetup(BWAPI::UnitType type, int no);

  /** Returns true if this Squad is active, or false if not.
   * A Squad is active when it first has been filled with agents.
   * A Squad with destroyed units are still considered Active. */
  virtual bool isActive();

  // Forces the squad to be active. 
  void forceActive();

  // Returns true if this Squad is full, i.e. it has all the units
  // it shall have. 
  bool isFull();

  // Returns the current size (i.e. number of alive agents in the squad). 
  int size();

  // Returns the maximum size of this squad (i.e. size of a full squad). 
  int maxSize();

  // Called each update to issue orders. 
  virtual void computeActions();

  // Sets the goal for this Squad. 
  void setGoal(BWAPI::TilePosition mGoal);

  // Used in debug modes to show goal of squads. 
  void debug_showGoal();

  // Checks if the squad is attacking, i.e. if any member of the squad has targets within range. 
  bool isAttacking();

  // Returns true if this Squad is under attack. 
  bool isUnderAttack();

  // Check if this Squad need units of the specified type. 
  bool needUnit(BWAPI::UnitType type);

  // Adds an agent to this Squad. 
  bool addMember(BaseAgent* agent);

  // Removes an agent from this Squad. 
  void removeMember(BaseAgent* agent);

  // Removes an agent of the specified type from this Squad,
  // and returns the reference to the removed agent. 
  BaseAgent* removeMember(BWAPI::UnitType type);

  // Disbands this squad. 
  void disband();

  // Orders this squad to defend a position. 
  virtual void defend(BWAPI::TilePosition mGoal);

  // Orders this squad to launch an attack at a position. 
  virtual void attack(BWAPI::TilePosition mGoal);

  // Orders this squad to assist units at a position. 
  virtual void assist(BWAPI::TilePosition mGoal);

  // Clears the goal for this Squad, i.e. sets the goal
  // to TilePosition(-1,-1). 
  virtual void clearGoal();

  // Returns the current goal of this Squad. 
  virtual BWAPI::TilePosition getGoal();

  // Returns the position to be moved to for units following
  // a squad. 
  BWAPI::TilePosition nextFollowMovePosition();

  // Returns the next TilePosition to move to. 
  BWAPI::TilePosition nextMovePosition();

  // Returns true if this squad has an assigned goal. 
  virtual bool hasGoal();

  // Returns the center position of this Squad, i.e. the
  // average x and y position of its members. 
  BWAPI::TilePosition getCenter();

  // Returns true if this is an Offensive Squad. 
  bool isOffensive() const;

  // Returns true if this is a Defensive Squad. 
  bool isDefensive() const;

  // Returns true if this is an Explorer Squad. 
  bool isExplorer() const;

  // Returns true if this is a Bunker Defense Squad. 
  bool isBunkerDefend() const;

  // Sets the bunker id for this squad (if any). 
  void setBunkerID(int unitID);

  // Returns the bunker id for this squad (if any). 
  int getBunkerID() const;

  // Returns true if this is a Shuttle Squad. 
  bool isShuttle() const;

  // Returns true if this is a Kite Squad. 
  bool isKite() const;

  // Returns true if this is a Rush Squad. 
  bool isRush() const;

  // Returns true if this is a Support Squad (for
  // example Transports). 
  bool isSupport() const;

  // Returns true if this squad travels by ground. 
  bool isGround() const;

  // Returns true if this squad travels by air. 
  bool isAir() const;

  // Returns the size of the Squad, i.e. the number
  // if agents currently in it. 
  int getSize();

  // Returns the total number of units in the squad when it is full. 
  int getTotalUnits();

  // Returns the current strength of the Squad, i.e.
  // the sum of the destroyScore() for all Squad members. 
  int getStrength();

  // Used to print some info to the screen. 
  virtual void printInfo();

  // Returns true if this Squad has the number of the specified
  // unit types in it. 
  bool hasUnits(BWAPI::UnitType type, int no);
};
