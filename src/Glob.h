#pragma once
#include "Actors/Actor.h"
#include <spdlog/spdlog.h>
#include "BWAPI/UnitType.h"

namespace rnp {
  class Army;
} // rnp

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
class Statistics;
class StrategySelector;
class Upgrader;

namespace rnp {

enum class MatchResult { Win, Loss, Draw };

// Globally visible functions which access main class singleton
const AgentManager* agent_manager();
const act::ActorId& agent_manager_id();

const rnp::Army* army();
const act::ActorId& army_id();

const Commander* commander();
const act::ActorId& commander_id();

const CommanderStrategy* strategy();
const act::ActorId& strategy_id();

const ExplorationManager* exploration();
const act::ActorId& exploration_id();

const Constructor* constructor();
const act::ActorId& constructor_id();

// TODO: Replace singletons with premade actor ids
BuildingPlacer* building_placer();
MapManager* map_manager();
NavigationAgent* navigation();
Pathfinder* pathfinder();
Profiler* profiler();
ResourceManager* resources();
Statistics* statistics();
StrategySelector* strategy_selector();
Upgrader* upgrader();

void start_logging();
spdlog::logger* log();

// Removes the race from a string, Terran Marine = Marine. 
std::string remove_race(const std::string& str);
inline std::string remove_race(BWAPI::UnitType type) {
  std::string name = type.getName();
  return remove_race(name);
}

} // ns rnp
