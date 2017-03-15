#pragma once
#include <memory>

class Pathfinder;
class ResourceManager;
class ExplorationManager;
class Statistics;
class Commander;
class Profiler;
class AgentManager;
class StrategySelector;
class BuildingPlacer;
class Constructor;
class Upgrader;
class NavigationAgent;
class MapManager;

namespace rnp {

  // Globally visible functions which access main class singleton
  std::shared_ptr<Commander> commander();
  Profiler* profiler();
  AgentManager* agent_manager();
  Statistics* statistics();
  StrategySelector* strategy_selector();
  BuildingPlacer* building_placer();
  ExplorationManager* exploration();
  Constructor* constructor();
  Upgrader* upgrader();
  ResourceManager* resources();
  Pathfinder* pathfinder();
  NavigationAgent* navigation();
  MapManager* map_manager();

} // ns rnp