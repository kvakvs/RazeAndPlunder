#pragma once

#include <BWAPI.h>
#include "BotAILoop.h"

#include "bwem.h"
#include "Utils/Statistics.h"

#define TOURNAMENT_NAME "SSCAIT 2017"
#define SPONSORS "the Sponsors!"
#define MINIMUM_COMMAND_OPTIMIZATION 1

// Remember not to use "Broodwar" in any global class constructor!

class BotTournamentModule : public BWAPI::TournamentModule {
  virtual bool onAction(int actionType, void* parameter = nullptr);
  virtual void onFirstAdvertisement();
};

// singletons forward def
class Commander;
class Profiler;
class AgentManager;
class StrategySelector;
class BuildingPlacer;
class ExplorationManager;
class Constructor;
class Upgrader;
class ResourceManager;
class Pathfinder;
class NavigationAgent;
class MapManager;

/** This class contains the main game loop and all events that is broadcasted from the Starcraft engine
* using BWAPI. See the BWAPI documentation for more info.
*
*/
class RnpBot : public BWAPI::AIModule {
  bool running_ = false;
  bool profile_ = false;
  
  // The singleton
  static RnpBot* singleton_;

  int speed_ = 1;
  BotAILoop ai_loop_;

public:
  // Globally visible resources via the singleton
  BWEM::Map& bwem_;
  std::shared_ptr<Commander> commander_;
  std::unique_ptr<Profiler> profiler_;
  std::unique_ptr<AgentManager> agent_manager_;
  std::unique_ptr<Statistics> statistics_;
  std::unique_ptr<StrategySelector> strategy_selector_;
  std::unique_ptr<BuildingPlacer> building_placer_;
  std::unique_ptr<ExplorationManager> exploration_;
  std::unique_ptr<Constructor> constructor_;
  std::unique_ptr<Upgrader> upgrader_;
  std::unique_ptr<ResourceManager> resource_manager_;
  std::unique_ptr<Pathfinder> pathfinder_;
  std::unique_ptr<NavigationAgent> navigation_;
  std::unique_ptr<MapManager> map_manager_;
  
public:
  RnpBot();

  static RnpBot* singleton() {
    bwem_assert(singleton_);
    return singleton_;
  }
  static RnpBot* setup() {
    return new RnpBot();
  }

  // Virtual functions for callbacks, leave these as they are.
  void onStart() override;
  void onEnd(bool isWinner) override;
  void onFrame() override;
  void onSendText(std::string text) override;
  void onReceiveText(BWAPI::Player player, std::string text) override;
  void onPlayerLeft(BWAPI::Player player) override;
  void onNukeDetect(BWAPI::Position target) override;
  void onUnitDiscover(BWAPI::Unit unit) override;
  void onUnitEvade(BWAPI::Unit unit) override;
  void onUnitShow(BWAPI::Unit unit) override;
  void onUnitHide(BWAPI::Unit unit) override;
  void onUnitCreate(BWAPI::Unit unit) override;
  void onUnitDestroy(BWAPI::Unit unit) override;
  void onUnitMorph(BWAPI::Unit unit) override;
  void onUnitRenegade(BWAPI::Unit unit) override;
  void onSaveGame(std::string gameName) override;
  void onUnitComplete(BWAPI::Unit unit) override;

private:
  void gameStopped();
  void init_early_singletons(); // called first onStart
  void init_singletons(); // called in the middle of onStart
};
