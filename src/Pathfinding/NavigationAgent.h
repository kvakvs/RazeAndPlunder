#pragma once

#include "MainAgents/BaseAgent.h"
#include "BWEM/bwem.h"

// The bot uses two techniques for navigation: if no enemy units are close 
// units navigate using the built in pathfinder in Starcraft. If enemy units 
// are close, own units uses potential fields to engage and surround the enemy.
// The NavigationAgent class is the main class for the navigation system, and 
// it switches between built in pathfinder and potential fields when necessary.
//
// The NavigationAgent is implemented as a singleton class. Each class that 
// needs to access NavigationAgent can request an instance, and all classes 
// share the same NavigationAgent instance.
//
// Author: Johan Hagelback (johan.hagelback@gmail.com)
//
class NavigationAgent {
  BWEM::Map& bwem_;

  int check_range_ = 0;
  int map_w_ = 0;
  int map_h_ = 0;

  // Moves a unit to the specified goal using the pathfinder, and stops at a
  // distance where the potential field navigation system should be used instead
  bool move_to_goal(const BaseAgent* agent,
                    BWAPI::TilePosition checkpoint,
                    BWAPI::TilePosition goal);

  // Calculates the potential field values for an attacking unit. 
  float get_attacking_unit_p(const BaseAgent* agent,
                             BWAPI::WalkPosition wp) const;

  float get_defending_unit_p(const BaseAgent* agent,
                             BWAPI::WalkPosition wp) const;

  static int get_max_unit_size(BWAPI::UnitType type);

  static BWAPI::Color get_color(float p);

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
    HybridPotentialField = 2
  };
  static PFType pathfinding_version_;

  NavigationAgent();
  ~NavigationAgent();

  // Is used to compute and execute movement commands for attacking units using the potential field
  // navigation system. 
  bool compute_move(const BaseAgent* agent,
                    BWAPI::TilePosition goal);

  // Computes a pathfinding move (no enemy units in range) 
  bool compute_pathfinding_move(const BaseAgent* agent,
                                BWAPI::TilePosition goal);

  // Computes a PF move (enemy units within range) 
  bool compute_potential_field_move(const BaseAgent* agent) const;

  // Computes a boids move (enemy units within range) 
  bool compute_boids_move(const BaseAgent* agent) const;

  // Displays a debug view of the potential fields for an agent. 
  void debug_display_pf(const BaseAgent* agent) const;
};
