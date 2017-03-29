#pragma once

#include <BWAPI.h>
#include "BaseAgent.h"

/** This agent is used to find the best target to attack for a unit. 
 * It is possible to add several rules, for example always target workers first.
 *
 * Currently the most expensive target (highest destroyscore) in range is targeted.
 * Units and buildings that cannot attack back get reduced score.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class TargetingAgent {

private:
  static double get_target_modifier(BWAPI::UnitType attacker, BWAPI::UnitType target);
  static bool is_cloaking_unit(BWAPI::UnitType type);
  static void handle_cloaked_unit(BWAPI::Unit unit);
  static bool is_highprio_target(BWAPI::UnitType type);

public:
  // Returns the best target within seekrange for a unit agent, or nullptr if no target
  // was found. 
  static BWAPI::Unit find_target(const BaseAgent* agent);

  // Returns a high prio target, if any, for strong attacks such as Yamato Gun. 
  static BWAPI::Unit find_highprio_target(const BaseAgent* agent, 
                                          int maxDist, 
                                          bool targetsAir, 
                                          bool targetsGround);

  // Returns the number of enemy units within range to attack an agent. 
  static int get_no_attackers(const BaseAgent* agent);

  // Checks if the specified type is an attacking unit or building. 
  static bool can_attack(BWAPI::UnitType type);

  // Checks the current target for an agent. If the agent has a bad target (for example
  // attacking a building and ignoring attacking units) a target switch is made. 
  static bool check_target(const BaseAgent* agent);
};
