#include "Glob.h"
#include "RnpBot.h"
#include "Utils/Profiler.h"

namespace rnp {

std::shared_ptr<spdlog::logger> rnp_global_logger;

const Commander* 
commander() { return RnpBot::singleton()->commander_ptr_; }
const act::ActorId&
commander_id() { return RnpBot::singleton()->commander_id_; }

const CommanderStrategy*
strategy() { return RnpBot::singleton()->strategy_ptr_; }
const act::ActorId&
strategy_id() { return RnpBot::singleton()->strategy_id_; }

Profiler* profiler() {
  return RnpBot::singleton()->profiler_.get();
}

Statistics* statistics() {
  return RnpBot::singleton()->statistics_.get();
}

StrategySelector* strategy_selector() {
  return RnpBot::singleton()->strategy_selector_.get();
}

const AgentManager* 
agent_manager() { return RnpBot::singleton()->agent_manager_ptr_; }
const act::ActorId& 
agent_manager_id() { return RnpBot::singleton()->agent_manager_id_; }

BuildingPlacer* building_placer() {
  return RnpBot::singleton()->building_placer_.get();
}

const ExplorationManager* 
exploration() { return RnpBot::singleton()->exploration_ptr_; }
const act::ActorId&
exploration_id() { return RnpBot::singleton()->exploration_id_; }

const Constructor* 
constructor() { return RnpBot::singleton()->constructor_ptr_; }
const act::ActorId&
constructor_id() { return RnpBot::singleton()->constructor_id_; }

Upgrader* upgrader() {
  return RnpBot::singleton()->upgrader_.get();
}

void start_logging() {
  try {
    std::vector<spdlog::sink_ptr> sinks;

    auto tty = std::make_shared<spdlog::sinks::wincolor_stdout_sink_st>();
    sinks.push_back(tty);

    auto file = std::make_shared<spdlog::sinks::rotating_file_sink_st>(
      "rnpbot.log",
      1048576 * 10,
      3);
    sinks.push_back(file);

    auto combined_logger = std::make_shared<spdlog::logger>("rnp",
                                                            begin(sinks),
                                                            end(sinks));
    spdlog::register_logger(combined_logger);
    rnp_global_logger = std::move(combined_logger);
  }
  catch (const spdlog::spdlog_ex& ex) {
    //std::cout << "Log initialization failed: " << ex.what() << std::endl;
  }
}

spdlog::logger* log() {
  return rnp_global_logger.get();
}

std::string remove_race(const std::string& str) {
  std::string res(str);

  std::string raceName = BWAPI::Broodwar->self()->getRace().getName();
  if (str.find(raceName) == 0) {
    int i = str.find("_");
    res = str.substr(i + 1, str.length());
  }

  if (res == "Siege Tank Tank Mode") res = "Siege Tank";

  return res;
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
