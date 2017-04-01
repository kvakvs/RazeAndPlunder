#pragma once

#include "Squad.h"
#include "RnpUtil.h"

/** This class handle squads used to explore the game world. Any unit type
 * can be used as explorer.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class ExplorationSquad : public Squad {
private:
  rnp::DelayCounter delay_respawn_;
  rnp::Memoize<BWAPI::TilePosition> m_next_explore_;

public:
  // Constructor. See Squad.h for more details. 
  ExplorationSquad(std::string name, int priority);

  /** Returns true if this Squad is active, or false if not.
   * A Squad is active when it first has been filled with agents.
   * A Squad with destroyed units are still considered Active. */
  bool is_active() const override {
    return active_;
  };

  // Called each update to issue orders. 
  void tick_inactive() override;
  void tick_active() override;

  // Orders this squad to defend a position. 
  void defend(BWAPI::TilePosition goal) override {
  }

  // Orders this squad to launch an attack at a position. 
  void attack(BWAPI::TilePosition goal) override {
  }

  // Orders this squad to assist units at a position. 
  void assist(BWAPI::TilePosition goal) override {
  }

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
    return act::spawn<ExplorationSquad>(ActorFlavour::Squad, 
                                        std::forward<Args>(args)...);
  }
};
