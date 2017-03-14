#pragma once

#include "Squad.h"

/** This squad rushes to each start location until the enemy has been located. 
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class RushSquad : public Squad {

private:
  Unit findWorkerTarget();

public:
  /** Constructor. See Squad.h for more details. */
  RushSquad(int mId, std::string mName, int mPriority);

  /** Returns true if this Squad is active, or false if not.
   * A Squad is active when it first has been filled with agents.
   * A Squad with destroyed units are still considered Active. */
  bool isActive() override;

  /** Called each update to issue orders. */
  void computeActions() override;

  /** Orders this squad to defend a position. */
  void defend(TilePosition mGoal) override;

  /** Orders this squad to launch an attack at a position. */
  void attack(TilePosition mGoal) override;

  /** Orders this squad to assist units at a position. */
  void assist(TilePosition mGoal) override;

  /** Clears the goal for this Squad, i.e. sets the goal
   * to TilePosition(-1,-1). */
  void clearGoal() override;

  /** Returns the current goal of this Squad. */
  TilePosition getGoal() override;

  /** Returns true if this squad has an assigned goal. */
  bool hasGoal() override;
};

