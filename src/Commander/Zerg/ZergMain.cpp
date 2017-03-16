#include "ZergMain.h"
#include "Managers/BuildplanEntry.h"
#include "Managers/AgentManager.h"
#include "Managers/ExplorationManager.h"
#include "Glob.h"
#include "Commander/ExplorationSquad.h"
#include "Commander/RushSquad.h"

using namespace BWAPI;

ZergMain::ZergMain() {
  buildplan_.push_back(BuildplanEntry(UnitTypes::Zerg_Spawning_Pool, 5));
  buildplan_.push_back(BuildplanEntry(UnitTypes::Zerg_Extractor, 5));
  buildplan_.push_back(BuildplanEntry(UnitTypes::Zerg_Hydralisk_Den, 8));
  buildplan_.push_back(BuildplanEntry(UnitTypes::Zerg_Hatchery, 14));

  mainSquad = std::make_shared<Squad>(1, Squad::SquadType::OFFENSIVE, "MainSquad", 10);
  mainSquad->addSetup(UnitTypes::Zerg_Zergling, 16);
  mainSquad->addSetup(UnitTypes::Zerg_Hydralisk, 10);
  mainSquad->setRequired(true);
  mainSquad->setBuildup(true);
  squads_.push_back(mainSquad);

  sc1 = std::make_shared<ExplorationSquad>(4, "ScoutingSquad", 8);
  sc1->addSetup(UnitTypes::Zerg_Overlord, 1);
  sc1->setRequired(false);
  sc1->setBuildup(false);
  sc1->setActivePriority(10);
  squads_.push_back(sc1);

  sc2 = std::make_shared<RushSquad>(5, "ScoutingSquad", 7);
  sc2->addSetup(UnitTypes::Zerg_Zergling, 2);
  sc2->setRequired(false);
  sc2->setBuildup(false);
  sc2->setActivePriority(1000);

  workers_num_ = 8;
  workers_per_refinery_ = 3;
}

ZergMain::~ZergMain() {
}

void ZergMain::on_frame() {
  on_frame_base();

  workers_num_ = rnp::agent_manager()->countNoBases() * 6 + rnp::agent_manager()->countNoUnits(UnitTypes::Zerg_Extractor) * 3;

  int cSupply = Broodwar->self()->supplyUsed() / 2;
  int min = Broodwar->self()->minerals();
  int gas = Broodwar->self()->gas();

  if (stage_ == 0 && rnp::agent_manager()->countNoFinishedUnits(UnitTypes::Zerg_Lair) > 0) {
    buildplan_.push_back(BuildplanEntry(UnitTypes::Zerg_Spire, cSupply));
    buildplan_.push_back(BuildplanEntry(UnitTypes::Zerg_Creep_Colony, cSupply));
    buildplan_.push_back(BuildplanEntry(UnitTypes::Zerg_Creep_Colony, cSupply));

    stage_++;
  }
  if (stage_ == 1 && rnp::agent_manager()->countNoFinishedUnits(UnitTypes::Zerg_Spire) > 0) {
    mainSquad->addSetup(UnitTypes::Zerg_Hydralisk, 14);
    mainSquad->addSetup(UnitTypes::Zerg_Mutalisk, 16);
    mainSquad->setBuildup(false);

    buildplan_.push_back(BuildplanEntry(UnitTypes::Zerg_Queens_Nest, cSupply));

    stage_++;
  }
  if (stage_ == 2 && min > 450) {
    buildplan_.push_back(BuildplanEntry(UnitTypes::Zerg_Hatchery, cSupply));

    stage_++;
  }
  if (stage_ == 3 && rnp::agent_manager()->countNoFinishedUnits(UnitTypes::Zerg_Hive) > 0) {
    buildplan_.push_back(BuildplanEntry(UnitTypes::Zerg_Defiler_Mound, cSupply));

    stage_++;
  }
  if (stage_ == 4 && rnp::agent_manager()->countNoFinishedUnits(UnitTypes::Zerg_Defiler_Mound) > 0) {
    mainSquad->addSetup(UnitTypes::Zerg_Defiler, 4);

    stage_++;
  }
}
