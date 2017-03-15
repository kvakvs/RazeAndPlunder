#pragma once

//#include "MainAgents/BaseAgent.h"
#include "bwem.h"

/** This class handles the main AI loop that is executed each frame. It is 
 * separated from the RnpBot::onFrame().
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class BotAILoop {

private:
  bool debug_;
  bool debug_unit_;
  bool debug_pf_;
  bool debug_bp_;
  int debug_sq_;
  BWEM::Map& bwem_;

  void drawTerrainData();

public:
  // Constructor 
  BotAILoop();

  // Destructor 
  ~BotAILoop();

  // call this before game started but after commander and bot singleton have been created
  void register_initial_units(); 

  // Call this each AI frame. 
  void computeActions();

  // Toggles debug info on/off. 
  void toggleDebug();

  // Toggles the unit debug info on/off. 
  void toggleUnitDebug();

  // Toggles the potential field debug info on/off. 
  void togglePFDebug();

  // Toggles the building placer debug info on/off. 
  void toggleBPDebug();

  // Shows debug info for the specified squad. Use -1
  // to disable info. 
  void setDebugSQ(int squadID);

  // Show debug info. 
  void show_debug();

  // Called when a new unit is added to the game. 
  void addUnit(BWAPI::Unit unit);

  // Called when a unit is destroyed in the game. 
  void unitDestroyed(BWAPI::Unit unit);

  // Called when a unit is morphed in in the game. 
  void morphUnit(BWAPI::Unit unit);
};
