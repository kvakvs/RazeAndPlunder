#pragma once

//#include "MainAgents/BaseAgent.h"
#include "BWEM/bwem.h"
#include "BWAPI/UnitCommand.h"
#include "Actors/ActorId.h"

/** This class handles the main AI loop that is executed each frame. It is 
 * separated from the RnpBot::onFrame().
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class BotAILoop {

private:
  bool debug_ = true;
  bool debug_units_ = false;
  bool debug_potential_fields_ = false;
  bool debug_building_placement_ = false;
  bool debug_mapmanager_ = false;
  act::ActorId debug_squad_id_;
  BWEM::Map& bwem_;

  void draw_terrain_data();

public:
  BotAILoop();
  ~BotAILoop();

  // call this before game started but after commander and bot singleton have been created
  static void register_initial_units(); 

  // Call this each AI frame. 
  void on_frame();

  // General switch for debug everything
  void toggle_debug();

  void toggle_unit_debug();
  void toggle_potential_fields_debug();
  void toggle_building_placement_debug();
  void toggle_mapmanager_debug();

  // Shows debug info for the specified squad. Use -1
  // to disable info. 
  void set_debug_sq(int squadID);

  // Show debug info. 
  void show_debug();

  // Called when a new unit is added to the game. 
  static void on_unit_added(BWAPI::Unit unit);

  // Called when a unit is destroyed in the game. 
  static void on_unit_destroyed(BWAPI::Unit unit);

  // Called when a unit is morphed in in the game. 
  static void on_unit_morphed(BWAPI::Unit unit);
};
