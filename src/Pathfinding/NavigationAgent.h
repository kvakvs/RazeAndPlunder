#pragma once

#include "MainAgents/BaseAgent.h"
#include "BWEM/bwem.h"

/** The bot uses two techniques for navigation: if no enemy units are close units navigate using the built in pathfinder in
 * Starcraft. If enemy units are close, own units uses potential fields to engage and surround the enemy.
 * The NavigationAgent class is the main class for the navigation system, and it switches between built in pathfinder and
 * potential fields when necessary.
 *
 * The NavigationAgent is implemented as a singleton class. Each class that needs to access NavigationAgent can request an instance,
 * and all classes shares the same NavigationAgent instance.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class NavigationAgent {
  BWEM::Map& bwem_;

  int check_range_;
  int map_w_;
  int map_h_;

  // Moves a unit to the specified goal using the pathfinder, and stops at a distance where the
  // potential field navigation system should be used instead. 
  bool moveToGoal(BaseAgent* agent, BWAPI::TilePosition checkpoint, BWAPI::TilePosition goal);

  // Calculates the potential field values for an attacking unit. 
  float getAttackingUnitP(BaseAgent* agent, BWAPI::WalkPosition wp) const;
  float getDefendingUnitP(BaseAgent* agent, BWAPI::WalkPosition wp) const;

  static int getMaxUnitSize(BWAPI::UnitType type);

  static BWAPI::Color getColor(float p);

public:
  /** 
   * Used to switch between the three pathfinding systems:
   * - Hybrid pathfinding with potential fields
   * - Hybrid pathfinding with boids
   * - Non-hybrid pathfinding
  */
  enum class PFType {
    Builtin = 0,
    HybridBoids = 1,
    HybridPF = 2
  };
  static PFType pathfinding_version_;

  NavigationAgent();
  ~NavigationAgent();

  // Is used to compute and execute movement commands for attacking units using the potential field
  // navigation system. 
  bool computeMove(BaseAgent* agent, BWAPI::TilePosition goal);

  // Computes a pathfinding move (no enemy units in range) 
  bool computePathfindingMove(BaseAgent* agent, BWAPI::TilePosition goal);

  // Computes a PF move (enemy units within range) 
  bool computePotentialFieldMove(BaseAgent* agent);

  // Computes a boids move (enemy units within range) 
  bool computeBoidsMove(BaseAgent* agent) const;

  // Displays a debug view of the potential fields for an agent. 
  void displayPF(BaseAgent* agent) const;
};
