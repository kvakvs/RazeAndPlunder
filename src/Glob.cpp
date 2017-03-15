#include "Glob.h"
#include "RnpBot.h"
#include "Utils/Profiler.h"

namespace rnp {

  std::shared_ptr<Commander> commander() {
    return RnpBot::singleton()->commander_;
  }

  Profiler* profiler() {
    return RnpBot::singleton()->profiler_.get();
  }

  AgentManager* agent_manager() {
    return RnpBot::singleton()->agent_manager_.get();
  }

  Statistics* statistics() {
    return RnpBot::singleton()->statistics_.get();
  }

  StrategySelector* strategy_selector() {
    return RnpBot::singleton()->strategy_selector_.get();
  }

  BuildingPlacer* building_placer() {
    return RnpBot::singleton()->building_placer_.get();
  }

  ExplorationManager* exploration() {
    return RnpBot::singleton()->exploration_.get();
  }

  Constructor* constructor() {
    return RnpBot::singleton()->constructor_.get();
  }

  Upgrader* upgrader() {
    return RnpBot::singleton()->upgrader_.get();
  }

  ResourceManager* resources() {
    return RnpBot::singleton()->resource_manager_.get();
  }

  Pathfinder* pathfinder() {
    return RnpBot::singleton()->pathfinder_.get();
  }

  NavigationAgent* navigation() {
    return RnpBot::singleton()->navigation_.get();
  }

  MapManager* map_manager() {
    return RnpBot::singleton()->map_manager_.get();
  }

} // ns rnp
