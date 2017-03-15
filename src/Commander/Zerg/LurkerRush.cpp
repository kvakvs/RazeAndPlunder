#include "LurkerRush.h"
#include "Managers/BuildplanEntry.h"
#include "Managers/AgentManager.h"
#include "Managers/ExplorationManager.h"
#include "Commander/RushSquad.h"
#include "Commander/ExplorationSquad.h"
#include "Glob.h"

using namespace BWAPI;

LurkerRush::LurkerRush() {
  buildplan_.push_back(BuildplanEntry(UnitTypes::Zerg_Spawning_Pool, 5));
  buildplan_.push_back(BuildplanEntry(UnitTypes::Zerg_Extractor, 5));
  buildplan_.push_back(BuildplanEntry(UnitTypes::Zerg_Hydralisk_Den, 8));
  buildplan_.push_back(BuildplanEntry(TechTypes::Lurker_Aspect, 13));

  mainSquad = std::make_shared<Squad>(1, Squad::SquadType::OFFENSIVE, "MainSquad", 10);
  mainSquad->setRequired(true);
  mainSquad->setBuildup(true);
  squads_.push_back(mainSquad);

  l1 = std::make_shared<RushSquad>(2, "LurkerSquad", 8);
  l1->addSetup(UnitTypes::Zerg_Hydralisk, 4);
  l1->setBuildup(false);
  l1->setRequired(false);
  l1->setActivePriority(11);
  l1->setMorphsTo(UnitTypes::Zerg_Lurker);
  squads_.push_back(l1);

  sc1 = std::make_shared<ExplorationSquad>(4, "ScoutingSquad", 8);
  sc1->addSetup(UnitTypes::Zerg_Overlord, 1);
  sc1->setRequired(false);
  sc1->setBuildup(false);
  sc1->setActivePriority(10);
  squads_.push_back(sc1);

  sc2 = std::make_shared<RushSquad>(5, "RushSquad", 7);
  sc2->addSetup(UnitTypes::Zerg_Zergling, 2);
  sc2->setRequired(false);
  sc2->setBuildup(false);
  sc2->setActivePriority(1000);

  workers_num_ = 8;
  workers_per_refinery_ = 3;
}

LurkerRush::~LurkerRush() {
}

void LurkerRush::computeActions() {
  computeActionsBase();

  workers_num_ = rnp::agent_manager()->countNoBases() * 6 + rnp::agent_manager()->countNoUnits(UnitTypes::Zerg_Extractor) * 3;

  int cSupply = Broodwar->self()->supplyUsed() / 2;

  if (stage_ == 0 && rnp::agent_manager()->countNoFinishedUnits(UnitTypes::Zerg_Lair) > 0) {
    //Check if we have spotted any enemy buildings. If not, send
    //out two Zerglings to rush base locations. Only needed for
    //2+ player maps.
    //This is needed to find out where the enemy is before we
    //send out the Lurkers.
    TilePosition tp = rnp::exploration()->getClosestSpottedBuilding(Broodwar->self()->getStartLocation());
    if (tp.x == -1 && Broodwar->getStartLocations().size() > 2) {
      squads_.push_back(sc2);
    }
    stage_++;
  }
  if (stage_ == 1 && rnp::agent_manager()->countNoFinishedUnits(UnitTypes::Zerg_Lurker) > 0) {
    buildplan_.push_back(BuildplanEntry(UnitTypes::Zerg_Spire, cSupply));
    buildplan_.push_back(BuildplanEntry(UnitTypes::Zerg_Hatchery, cSupply));

    mainSquad->addSetup(UnitTypes::Zerg_Mutalisk, 20);
    mainSquad->setBuildup(false);

    stage_++;
  }
  if (stage_ == 2 && rnp::agent_manager()->countNoBases() > 1) {
    buildplan_.push_back(BuildplanEntry(UnitTypes::Zerg_Creep_Colony, cSupply));
    buildplan_.push_back(BuildplanEntry(UnitTypes::Zerg_Creep_Colony, cSupply));

    stage_++;
  }
  if (stage_ == 3 && rnp::agent_manager()->countNoFinishedUnits(UnitTypes::Zerg_Sunken_Colony) > 0) {
    buildplan_.push_back(BuildplanEntry(UnitTypes::Zerg_Hatchery, cSupply));

    stage_++;
  }
}
