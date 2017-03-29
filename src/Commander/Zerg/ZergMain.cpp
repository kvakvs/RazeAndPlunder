#if 0

#include "ZergMain.h"
#include "Managers/BuildplanEntry.h"
#include "Managers/AgentManager.h"
#include "Managers/ExplorationManager.h"
#include "Glob.h"
#include "Commander/ExplorationSquad.h"
#include "Commander/RushSquad.h"

using namespace BWAPI;

ZergMain::ZergMain() {
  plan(UnitTypes::Zerg_Spawning_Pool, 5);
  plan(UnitTypes::Zerg_Extractor, 5);
  plan(UnitTypes::Zerg_Hydralisk_Den, 8);
  plan(UnitTypes::Zerg_Hatchery, 14);

  mainSquad = std::make_shared<Squad>(1, Squad::SquadType::OFFENSIVE, "MainSquad", 10);
  mainSquad->add_setup(UnitTypes::Zerg_Zergling, 16);
  mainSquad->add_setup(UnitTypes::Zerg_Hydralisk, 10);
  mainSquad->set_required(true);
  mainSquad->set_buildup(true);
  squads_.push_back(mainSquad);

  scout_sq_ = std::make_shared<ExplorationSquad>(4, "ScoutingSquad", 8);
  scout_sq_->add_setup(UnitTypes::Zerg_Overlord, 1);
  scout_sq_->set_required(false);
  scout_sq_->set_buildup(false);
  scout_sq_->set_active_priority(10);
  squads_.push_back(scout_sq_);

  rush_sq_ = std::make_shared<RushSquad>(5, "RushSquad", 7);
  rush_sq_->add_setup(UnitTypes::Zerg_Zergling, 2);
  rush_sq_->set_required(false);
  rush_sq_->set_buildup(false);
  rush_sq_->set_active_priority(1000);

  workers_num_ = 8;
  workers_per_refinery_ = 3;
}

ZergMain::~ZergMain() {
}

void ZergMain::on_frame() {
  on_frame_base();

  workers_num_ = rnp::agent_manager()->get_bases_count() * 6 + rnp::agent_manager()->get_units_of_type_count(UnitTypes::Zerg_Extractor) * 3;

  int cSupply = Broodwar->self()->supplyUsed() / 2;
  int min = Broodwar->self()->minerals();
  int gas = Broodwar->self()->gas();

  if (stage_ == 0 && rnp::agent_manager()->get_finished_units_count(UnitTypes::Zerg_Lair) > 0) {
    plan(UnitTypes::Zerg_Spire, cSupply);
    plan(UnitTypes::Zerg_Creep_Colony, cSupply);
    plan(UnitTypes::Zerg_Creep_Colony, cSupply);

    stage_++;
  }
  if (stage_ == 1 && rnp::agent_manager()->get_finished_units_count(UnitTypes::Zerg_Spire) > 0) {
    mainSquad->add_setup(UnitTypes::Zerg_Hydralisk, 14);
    mainSquad->add_setup(UnitTypes::Zerg_Mutalisk, 16);
    mainSquad->set_buildup(false);

    plan(UnitTypes::Zerg_Queens_Nest, cSupply);

    stage_++;
  }
  if (stage_ == 2 && min > 450) {
    plan(UnitTypes::Zerg_Hatchery, cSupply);

    stage_++;
  }
  if (stage_ == 3 && rnp::agent_manager()->get_finished_units_count(UnitTypes::Zerg_Hive) > 0) {
    plan(UnitTypes::Zerg_Defiler_Mound, cSupply);

    stage_++;
  }
  if (stage_ == 4 && rnp::agent_manager()->get_finished_units_count(UnitTypes::Zerg_Defiler_Mound) > 0) {
    mainSquad->add_setup(UnitTypes::Zerg_Defiler, 4);

    stage_++;
  }
}

#endif // 0
