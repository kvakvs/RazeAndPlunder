#include "RnpBot.h"
#include "Managers/Constructor.h"
#include "Managers/ExplorationManager.h"
#include "Influencemap/MapManager.h"
#include "Managers/BuildingPlacer.h"
#include "Managers/Upgrader.h"
#include "Managers/ResourceManager.h"
#include "Utils/Profiler.h"
#include "Pathfinding/NavigationAgent.h"
#include "Utils/Config.h"
#include "Commander/StrategySelector.h"

#include "Managers/AgentManager.h"

#include "Glob.h"
#include "RnpConst.h"

using namespace BWAPI;

RnpBot* RnpBot::singleton_ = nullptr;

RnpBot::RnpBot()
    : BWAPI::AIModule(), bwem_(&BWEM::Map::Instance())
    , commander_id_(), agent_manager_id_(), exploration_id_(), constructor_id_()
    , building_placer_(), map_manager_(), navigation_(), pathfinder_()
    , profiler_(), resource_manager_(), statistics_(), strategy_selector_()
    , upgrader_()
{
  singleton_ = this;
}

RnpBot::~RnpBot() {
}

void RnpBot::init_early_singletons() {
  profiler_ = std::make_unique<Profiler>();
  statistics_ = std::make_unique<Statistics>();
  strategy_selector_ = std::make_unique<StrategySelector>();
}

void RnpBot::init_singletons() {
  agent_manager_ptr_ = act::spawn_get_ptr<AgentManager>(ActorFlavour::Singleton);
  agent_manager_id_ = agent_manager_ptr_->self();

  commander_ptr_ = strategy_selector_->get_strategy();
  commander_id_ = commander_ptr_->self();

  exploration_ptr_ = act::spawn_get_ptr<ExplorationManager>(ActorFlavour::Singleton);
  exploration_id_ = exploration_ptr_->self();

  constructor_ptr_ = act::spawn_get_ptr<Constructor>(ActorFlavour::Singleton);
  constructor_id_ = constructor_ptr_->self();

  building_placer_ = std::make_unique<BuildingPlacer>();
  upgrader_ = std::make_unique<Upgrader>();
  resource_manager_ = std::make_unique<ResourceManager>();
  pathfinder_ = std::make_unique<Pathfinder>();
  navigation_ = std::make_unique<NavigationAgent>();
}

void RnpBot::on_start_init_map() {
  rnp::log()->info("Map initialization: {}", Broodwar->mapFileName());

  if (bwem_->Initialized()) {
    bwem_->Destroy();
    bwem_ = &BWEM::Map::Instance();
  }
  bwem_->Initialize();
  bwem_->EnableAutomaticPathAnalysis();
    
  auto starting_locations_ok = bwem_->FindBasesForStartingLocations();
  assert(starting_locations_ok);

  rnp::log()->info("Map dimensions {0}x{1}",
                   Broodwar->mapWidth(), Broodwar->mapHeight());
}

void RnpBot::on_start_setup_game() {
  Broodwar->setCommandOptimizationLevel(2); //0--3

  msg::commander::modify_commander(
    [](Commander* c) {
      c->toggle_squads_debug();
      c->toggle_buildplan_debug();
    });

  //Set speed
  speed_ = 0;
  Broodwar->setLocalSpeed(speed_);
}

void RnpBot::onStart() {
  print_help();
  
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

    // Analyze map // using BWTA
    on_start_init_map();

//    BWEM::utils::MapPrinter::Initialize(&bwem_);
//    BWEM::utils::printMap(theMap);      // will print the map into the file <StarCraftFolder>bwapi-data/map.bmp
//    BWEM::utils::pathExample(theMap);   // add to the printed map a path between two starting locations

    profile_ = false;

    init_singletons();

    map_manager_ = std::make_unique<MapManager>();

    ai_loop_.register_initial_units();

    running_ = true;

    // Set debug options and game speed
    on_start_setup_game();

    profiler_->end("OnInit");
  }
  catch (const std::exception& e) {
    Broodwar << "EXCEPTION: " << e.what() << std::endl;
  }
}

void RnpBot::game_stopped() {
  profiler_->dump_to_file();
  running_ = false;
  game_finished_ = true;
}

void RnpBot::onEnd(bool isWinner) {
  if (Broodwar->elapsedTime() / 60 < 4) return;

  auto win = isWinner ? rnp::MatchResult::Win
                      : rnp::MatchResult::Loss;
  if (Broodwar->elapsedTime() / 60 >= 80) {
    win = rnp::MatchResult::Draw;
  }

  strategy_selector_->add_result(win);
  strategy_selector_->save_stats();
  statistics_->save_result(win);

  game_stopped();
}

void RnpBot::onFrame() {
  if (not running_) {
    //Game over. Do nothing.
    return;
  }
  if (not Broodwar->isInGame()) {
    //Not in game. Do nothing.
    game_stopped();
    return;
  }
  if (Broodwar->isReplay()) {
    //Replay. Do nothing.
    return;
  }

  profiler_->start("OnFrame");
  if (Broodwar->elapsedTime() / 60 >= 81) {
    //Stalled game. Leave it.
    Broodwar->leaveGame();
    return;
  }

  ai_loop_.on_frame();
  ai_loop_.show_debug();

  Config::getInstance()->displayBotName();

  profiler_->end("OnFrame");
}

void RnpBot::print_help() {
  rnp::log()->info("Console commands: d - all debug, dpf - pot field debug, "
    "dbp - building debug, dm - map debug, spf# - set pathfinding, "
    "sq# - squad# debug, + ++ - -- speed setting, dup - upgrader debug, "
    "du - units debug, ds - toggle all squads debug, db - buildplan debug");
}

void RnpBot::onSendText(std::string text) {
  if (text == "a") {
    msg::commander::modify_commander(
      [](Commander* c) { c->force_begin_attack(); });
  }
  else if (text == "p") {
    profile_ = !profile_;
  }
  else if (text == "d") {
    ai_loop_.toggle_debug();
  }
  else if (text == "dpf"
           && NavigationAgent::pathfinding_version_
              == NavigationAgent::PFType::HybridPotentialField) {
    ai_loop_.toggle_potential_fields_debug();
  }
  else if (text == "dbp") {
    ai_loop_.toggle_building_placement_debug();
  }
  else if (text == "dm") {
    ai_loop_.toggle_mapmanager_debug();
  }
  else if (text == "spf") {
    int pfver = static_cast<int>(NavigationAgent::pathfinding_version_);
    pfver = (pfver + 1) % 3;
    NavigationAgent::pathfinding_version_
        = static_cast<NavigationAgent::PFType>(pfver);
  }
  else if (text.substr(0, 2) == "sq") {
    if (text == "sq") {
      ai_loop_.set_debug_sq(-1);
    }
    else {
      int id = atoi(&text[2]);
      ai_loop_.set_debug_sq(id);
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
  else if (text == "dup") {
    upgrader_->toggle_debug();
  }
  else if (text == "ds") {
    msg::commander::modify_commander(
      [](Commander* c) { c->toggle_squads_debug(); });
  }
  else if (text == "db") {
    msg::commander::modify_commander(
      [](Commander* c) { c->toggle_buildplan_debug(); });
  }
  else if (text == "du") {
    ai_loop_.toggle_unit_debug();
  }
  else {
    Broodwar << "Not a recognized command '" << text << "'" << std::endl;
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
    if (not unit->getPlayer()->isNeutral() 
        && not unit->getPlayer()->isAlly(Broodwar->self())) 
    {
      ExplorationManager::modify(
        [unit](ExplorationManager* e) { e->on_unit_spotted(unit); });
    }
  }
}

void RnpBot::onUnitEvade(BWAPI::Unit unit) {

}

void RnpBot::onUnitShow(BWAPI::Unit unit) {
  if (Broodwar->isReplay() || Broodwar->getFrameCount() <= 1) return;

  if (unit->getPlayer()->getID() != Broodwar->self()->getID()) {
    if (not unit->getPlayer()->isNeutral() 
        && not unit->getPlayer()->isAlly(Broodwar->self())) 
    {
      ExplorationManager::modify(
        [unit](ExplorationManager* e) { e->on_unit_spotted(unit); });
    }
  }
}

void RnpBot::onUnitHide(BWAPI::Unit unit) {

}

void RnpBot::onUnitCreate(BWAPI::Unit unit) {
  if (rnp::is_my_unit(unit)) {
    ai_loop_.on_unit_added(unit);
  }
}

void RnpBot::onUnitComplete(BWAPI::Unit unit) {

}

void RnpBot::onUnitDestroy(BWAPI::Unit unit) {
  if (Broodwar->isReplay() || Broodwar->getFrameCount() <= 1) return;

  try {
    if (unit->getType().isMineralField()) {
      bwem_->OnMineralDestroyed(unit);
    }
    else if (unit->getType().isSpecialBuilding()) {
      bwem_->OnStaticBuildingDestroyed(unit);
    }
  }
  catch (const std::exception& e) {
    Broodwar << "EXCEPTION: " << e.what() << std::endl;
  }

  ai_loop_.on_unit_destroyed(unit);
}

void RnpBot::onUnitMorph(BWAPI::Unit unit) {
  if (Broodwar->isReplay() || Broodwar->getFrameCount() <= 1) return;

  if (rnp::is_my_unit(unit)) {
    if (Constructor::is_zerg()) {
      ai_loop_.on_unit_morphed(unit);
    }
    else {
      ai_loop_.on_unit_added(unit);
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
