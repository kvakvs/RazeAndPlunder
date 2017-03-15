#include "RnpBot.h"
#include "Managers/Constructor.h"
#include "Managers/ExplorationManager.h"
#include "Influencemap/MapManager.h"
#include "Managers/BuildingPlacer.h"
#include "Commander/Commander.h"
#include "Pathfinding/Pathfinder.h"
#include "Managers/Upgrader.h"
#include "Managers/ResourceManager.h"
#include "Utils/Profiler.h"
#include "Pathfinding/NavigationAgent.h"
#include "Utils/Config.h"
#include "Commander/StrategySelector.h"
#include "Utils/Statistics.h"

#include "Managers/AgentManager.h"

#include <shlwapi.h>
#include "mapPrinter.h"
#include "Glob.h"

using namespace BWAPI;

RnpBot* RnpBot::singleton_ = nullptr;

RnpBot::RnpBot() : BWAPI::AIModule(), bwem_(BWEM::Map::Instance()) {
  singleton_ = this;
}

void RnpBot::init_early_singletons() {
  profiler_ = std::make_unique<Profiler>();
  statistics_ = std::make_unique<Statistics>();
  strategy_selector_ = std::make_unique<StrategySelector>();
}

void RnpBot::init_singletons() {
  agent_manager_ = std::make_unique<AgentManager>();
  commander_ = strategy_selector_->getStrategy();
  building_placer_ = std::make_unique<BuildingPlacer>();
  exploration_ = std::make_unique<ExplorationManager>();
  constructor_ = std::make_unique<Constructor>();
  upgrader_ = std::make_unique<Upgrader>();
  resource_manager_ = std::make_unique<ResourceManager>();
  pathfinder_ = std::make_unique<Pathfinder>();
  navigation_ = std::make_unique<NavigationAgent>();

  ai_loop_.register_initial_units();
}

void RnpBot::onStart() {
  try {
    init_early_singletons();

    //Enable/disable file writing stuff 
    profiler_->disable();
    statistics_->disable();
    strategy_selector_->disable();

    profiler_->start("OnInit");

    //Needed for text commands to work
    Broodwar->enableFlag(Flag::UserInput);

    //Uncomment to enable complete map information
    //Broodwar->enableFlag(Flag::CompleteMapInformation);

    //
    // Analyze map // using BWTA
    // 
    Broodwar << "Map initialization..." << std::endl;

    bwem_.Initialize();
    bwem_.EnableAutomaticPathAnalysis();
    bool starting_locations_ok = bwem_.FindBasesForStartingLocations();
    assert(starting_locations_ok);

//    BWEM::utils::MapPrinter::Initialize(&bwem_);
//    BWEM::utils::printMap(theMap);      // will print the map into the file <StarCraftFolder>bwapi-data/map.bmp
//    BWEM::utils::pathExample(theMap);   // add to the printed map a path between two starting locations

    Broodwar << "gg" << std::endl;

    profile_ = false;

    init_singletons();

    //Fill pathfinder
    for (auto& area : bwem_.Areas()) {
      for (auto& base : area.Bases()) {
        auto pos = base.Center();
        pathfinder_->requestPath(
          Broodwar->self()->getStartLocation(), 
          TilePosition(pos));
      }
    }

    map_manager_ = std::make_unique<MapManager>();

    //Add the units we have from start to agent manager
    for (auto& u : Broodwar->self()->getUnits()) {
      rnp::agent_manager()->addAgent(u);
    }

    running_ = true;

    Broodwar->setCommandOptimizationLevel(2); //0--3

    //Debug mode. Active panels.
    commander_->toggleSquadsDebug();
    commander_->toggleBuildplanDebug();
    upgrader_->toggleDebug();
    ai_loop_.toggleUnitDebug();
    //loop.toggleBPDebug();
    //End Debug mode

    //Set speed
    speed_ = 0;
    Broodwar->setLocalSpeed(speed_);

    profiler_->end("OnInit");
  }
  catch (const std::exception& e) {
    Broodwar << "EXCEPTION: " << e.what() << std::endl;
  }
}

void RnpBot::gameStopped() {
  pathfinder_->stop();
  profiler_->dumpToFile();
  running_ = false;
}

void RnpBot::onEnd(bool isWinner) {
  if (Broodwar->elapsedTime() / 60 < 4) return;

  int win = 0;
  if (isWinner) win = 1;
  if (Broodwar->elapsedTime() / 60 >= 80) win = 2;

  strategy_selector_->addResult(win);
  strategy_selector_->saveStats();
  statistics_->saveResult(win);

  gameStopped();
}

void RnpBot::onFrame() {
  profiler_->start("OnFrame");

  if (not running_) {
    //Game over. Do nothing.
    return;
  }
  if (not Broodwar->isInGame()) {
    //Not in game. Do nothing.
    gameStopped();
    return;
  }
  if (Broodwar->isReplay()) {
    //Replay. Do nothing.
    return;
  }

  if (Broodwar->elapsedTime() / 60 >= 81) {
    //Stalled game. Leave it.
    Broodwar->leaveGame();
    return;
  }

  ai_loop_.computeActions();
  ai_loop_.show_debug();

  Config::getInstance()->displayBotName();

  profiler_->end("OnFrame");
}

void RnpBot::onSendText(std::string text) {
  if (text == "a") {
    rnp::commander()->forceAttack();
  }
  else if (text == "p") {
    profile_ = !profile_;
  }
  else if (text == "d") {
    ai_loop_.toggleDebug();
  }
  else if (text == "pf" && NavigationAgent::pathfinding_version_ == 2) {
    ai_loop_.togglePFDebug();
  }
  else if (text == "bp") {
    ai_loop_.toggleBPDebug();
  }
  else if (text == "spf") {
    NavigationAgent::pathfinding_version_++;
    if (NavigationAgent::pathfinding_version_ > 2) NavigationAgent::pathfinding_version_ = 0;
  }
  else if (text.substr(0, 2) == "sq") {
    if (text == "sq") {
      ai_loop_.setDebugSQ(-1);
    }
    else {
      int id = atoi(&text[2]);
      ai_loop_.setDebugSQ(id);
    }
  }
  else if (text == "+") {
    speed_ -= 4;
    if (speed_ < 0) {
      speed_ = 0;
    }
    Broodwar << "Changed game speed: " << speed_ << std::endl;
    Broodwar->setLocalSpeed(speed_);
  }
  else if (text == "++") {
    speed_ = 0;
    Broodwar << "Changed game speed: " << speed_ << std::endl;
    Broodwar->setLocalSpeed(speed_);
  }
  else if (text == "-") {
    speed_ += 4;
    Broodwar << "Changed game speed: " << speed_ << std::endl;
    Broodwar->setLocalSpeed(speed_);
  }
  else if (text == "--") {
    speed_ = 24;
    Broodwar << "Changed game speed: " << speed_ << std::endl;
    Broodwar->setLocalSpeed(speed_);
  }
  else if (text == "t") {
    upgrader_->toggleDebug();
  }
  else if (text == "s") {
    rnp::commander()->toggleSquadsDebug();
  }
  else if (text == "b") {
    rnp::commander()->toggleBuildplanDebug();
  }
  else if (text == "i") {
    ai_loop_.toggleUnitDebug();
  }
  else {
    Broodwar << "You typed '" << text << "'!" << std::endl;
  }
}

void RnpBot::onReceiveText(BWAPI::Player player, std::string text) {
  Broodwar << player->getName() << " said '" << text << "'" << std::endl;
}

void RnpBot::onPlayerLeft(BWAPI::Player player) {

}

void RnpBot::onNukeDetect(BWAPI::Position target) {
  if (target != Positions::Unknown) {
    TilePosition t(target);
    Broodwar << "Nuclear Launch Detected at (" << t.x << "," << t.y << ")" << std::endl;
  }
  else {
    Broodwar << "Nuclear Launch Detected" << std::endl;
  }
}

void RnpBot::onUnitDiscover(BWAPI::Unit unit) {
  if (Broodwar->isReplay() || Broodwar->getFrameCount() <= 1) return;

  if (unit->getPlayer()->getID() != Broodwar->self()->getID()) {
    if (not unit->getPlayer()->isNeutral() && !unit->getPlayer()->isAlly(Broodwar->self())) {
      exploration_->addSpottedUnit(unit);
    }
  }
}

void RnpBot::onUnitEvade(BWAPI::Unit unit) {

}

void RnpBot::onUnitShow(BWAPI::Unit unit) {
  if (Broodwar->isReplay() || Broodwar->getFrameCount() <= 1) return;

  if (unit->getPlayer()->getID() != Broodwar->self()->getID()) {
    if (not unit->getPlayer()->isNeutral() && !unit->getPlayer()->isAlly(Broodwar->self())) {
      exploration_->addSpottedUnit(unit);
    }
  }
}

void RnpBot::onUnitHide(BWAPI::Unit unit) {

}

void RnpBot::onUnitCreate(BWAPI::Unit unit) {
  if (unit->getPlayer()->getID() == Broodwar->self()->getID()) {
    ai_loop_.addUnit(unit);
  }
}

void RnpBot::onUnitComplete(BWAPI::Unit unit) {

}

void RnpBot::onUnitDestroy(BWAPI::Unit unit) {
  if (Broodwar->isReplay() || Broodwar->getFrameCount() <= 1) return;

  try {
    if (unit->getType().isMineralField()) {
      bwem_.OnMineralDestroyed(unit);
    }
    else if (unit->getType().isSpecialBuilding()) {
      bwem_.OnStaticBuildingDestroyed(unit);
    }
  }
  catch (const std::exception& e) {
    Broodwar << "EXCEPTION: " << e.what() << std::endl;
  }

  ai_loop_.unitDestroyed(unit);
}

void RnpBot::onUnitMorph(BWAPI::Unit unit) {
  if (Broodwar->isReplay() || Broodwar->getFrameCount() <= 1) return;

  if (unit->getPlayer()->getID() == Broodwar->self()->getID()) {
    if (Constructor::isZerg()) {
      ai_loop_.morphUnit(unit);
    }
    else {
      ai_loop_.addUnit(unit);
    }
  }
}

void RnpBot::onUnitRenegade(BWAPI::Unit unit) {

}

void RnpBot::onSaveGame(std::string gameName) {
  Broodwar << "The game was saved to '" << gameName << "'" << std::endl;
}

bool BotTournamentModule::onAction(int actionType, void* parameter) {
  switch (actionType) {
  case Tournament::SendText:
  case Tournament::Printf:
    return true;
  case Tournament::EnableFlag:
    switch (*static_cast<int*>(parameter)) {
    case Flag::CompleteMapInformation:
    case Flag::UserInput:
      // Disallow these two flags
      return false;
    default: 
      return true;
    }
  case Tournament::LeaveGame:
  case Tournament::PauseGame:
  case Tournament::ResumeGame:
  case Tournament::SetFrameSkip:
  case Tournament::SetGUI:
  case Tournament::SetLocalSpeed:
  case Tournament::SetMap:
    return false; // Disallow these actions
  case Tournament::SetLatCom:
  case Tournament::SetTextSize:
    return true; // Allow these actions
  case Tournament::SetCommandOptimizationLevel:
    return *(int*)parameter > MINIMUM_COMMAND_OPTIMIZATION; // Set a minimum command optimization level 
    // to reduce APM with no action loss
  default:
    break;
  }
  return true;
}

void BotTournamentModule::onFirstAdvertisement() {
  Broodwar->sendText("Welcome to " TOURNAMENT_NAME "!");
  Broodwar->sendText("Brought to you by " SPONSORS ".");
}
