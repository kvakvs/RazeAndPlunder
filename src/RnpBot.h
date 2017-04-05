#pragma once

#include <BWAPI.h>
#include "BotAILoop.h"

#include "BWEM/bwem.h"
#include "Utils/Statistics.h"
#include "Actors/Actor.h"

// Remember not to use "Broodwar" in any global class constructor!

class BotTournamentModule : public BWAPI::TournamentModule {
  virtual bool onAction(int actionType, void* parameter = nullptr);
  virtual void onFirstAdvertisement();
};

// singletons forward def
class AgentManager;
class BuildingPlacer;
class Commander;
class CommanderStrategy;
class Constructor;
class ExplorationManager;
class MapManager;
class NavigationAgent;
class Pathfinder;
class Profiler;
class ResourceManager;
class StrategySelector;
class Upgrader;

/** This class contains the main game loop and all events that is broadcasted from the Starcraft engine
* using BWAPI. See the BWAPI documentation for more info.
*
*/
class RnpBot : public BWAPI::AIModule {
  bool running_ = false;
  bool profile_ = false;
  bool game_finished_ = false;
  
  // The singleton
  static RnpBot* singleton_;

  int speed_ = 1;
  BotAILoop ai_loop_;

public:
  // Globally visible resources via the singleton
  BWEM::Map* bwem_;

//  template <class T> struct ActorAndId {
//    const T* ptr_ = nullptr;
//    act::ActorId id_;
//  };

  const Commander* commander_ptr_ = nullptr;
  act::ActorId commander_id_;

  const CommanderStrategy* strategy_ptr_ = nullptr;
  act::ActorId strategy_id_;

  const AgentManager* agent_manager_ptr_ = nullptr;
  act::ActorId agent_manager_id_;

  const ExplorationManager* exploration_ptr_ = nullptr;
  act::ActorId exploration_id_;

  const Constructor* constructor_ptr_ = nullptr;
  act::ActorId constructor_id_;

  std::unique_ptr<BuildingPlacer> building_placer_;
  std::unique_ptr<MapManager> map_manager_;
  std::unique_ptr<NavigationAgent> navigation_;
  std::unique_ptr<Pathfinder> pathfinder_;
  std::unique_ptr<Profiler> profiler_;
  std::unique_ptr<ResourceManager> resource_manager_;
  std::unique_ptr<Statistics> statistics_;
  std::unique_ptr<StrategySelector> strategy_selector_;
  std::unique_ptr<Upgrader> upgrader_;
  
public:
  RnpBot();
  ~RnpBot();
  RnpBot(const RnpBot& other) = delete;
  RnpBot& operator = (const RnpBot& other) = delete;

  static RnpBot* singleton() {
    bwem_assert(singleton_);
    return singleton_;
  }
  static RnpBot* setup() {
    return new RnpBot();
  }

  bool is_game_finished() const { return game_finished_;  }

  // Virtual functions for callbacks, leave these as they are.
  void onEnd(bool isWinner) override;
  void onFrame() override;
  void print_help();
  void onNukeDetect(BWAPI::Position target) override;
  void onPlayerLeft(BWAPI::Player player) override;
  void onReceiveText(BWAPI::Player player, std::string text) override;
  void onSaveGame(std::string gameName) override;
  void onSendText(std::string text) override;
  void onStart() override;
  void onUnitComplete(BWAPI::Unit unit) override;
  void onUnitCreate(BWAPI::Unit unit) override;
  void onUnitDestroy(BWAPI::Unit unit) override;
  void onUnitDiscover(BWAPI::Unit unit) override;
  void onUnitEvade(BWAPI::Unit unit) override;
  void onUnitHide(BWAPI::Unit unit) override;
  void onUnitMorph(BWAPI::Unit unit) override;
  void onUnitRenegade(BWAPI::Unit unit) override;
  void onUnitShow(BWAPI::Unit unit) override;

private:
  void game_stopped();
  
  // called before everything in onStart
  void init_early_singletons(); 
  
  // called in the middle of onStart
  void init_singletons(); 

  void on_start_init_map();
  
  // Set debug options and game speed
  void on_start_setup_game();
};
