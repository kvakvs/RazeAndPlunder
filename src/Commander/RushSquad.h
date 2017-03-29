#pragma once

#include "Squad.h"

/** This squad rushes to each start location until the enemy has been located. 
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class RushSquad : public Squad {

private:
  act::ActorId find_worker_target() const;

public:
  //  Constructor. See Squad.h for more details. 
  RushSquad(std::string mName, int mPriority);

  /** Returns true if this Squad is active, or false if not.
   * A Squad is active when it first has been filled with agents.
   * A Squad with destroyed units are still considered Active. */
  bool is_active() const override;

  // Called each update to issue orders. 
  void tick() override;

  // Orders this squad to defend a position. 
  void defend(BWAPI::TilePosition mGoal) override;

  // Orders this squad to launch an attack at a position. 
  void attack(BWAPI::TilePosition mGoal) override;

  // Orders this squad to assist units at a position. 
  void assist(BWAPI::TilePosition mGoal) override;

  // Clears the goal for this Squad, i.e. sets the goal
  // to TilePosition(-1,-1). 
  void clear_goal() override;

  // Returns the current goal of this Squad. 
  BWAPI::TilePosition get_goal() const override {
    return goal_;
  }

  // Returns true if this squad has an assigned goal. 
  bool has_goal() const override;

  //
  // Actor stuff
  //
  template <typename... Args>
  static act::ActorId spawn(Args&& ...args) {
    return act::spawn<RushSquad>(ActorFlavour::Squad, std::forward<Args>(args)...);
  }
private:
  void prepare_rush_squad();
  void tick_active_rush_squad();
};

