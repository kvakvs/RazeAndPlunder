#pragma once
#include <memory>

class AgentManager;
class BuildingPlacer;
class Commander;
class Constructor;
class ExplorationManager;
class MapManager;
class NavigationAgent;
class Pathfinder;
class Profiler;
class ResourceManager;
class Statistics;
class StrategySelector;
class Upgrader;

namespace rnp {

  // Globally visible functions which access main class singleton
  AgentManager* agent_manager();
  BuildingPlacer* building_placer();
  std::shared_ptr<Commander> commander();
  Constructor* constructor();
  ExplorationManager* exploration();
  MapManager* map_manager();
  NavigationAgent* navigation();
  Pathfinder* pathfinder();
  Profiler* profiler();
  ResourceManager* resources();
  Statistics* statistics();
  StrategySelector* strategy_selector();
  Upgrader* upgrader();

} // ns rnp