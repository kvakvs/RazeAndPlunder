#include "ProtossMain.h"
#include "../../Managers/BuildplanEntry.h"
#include "../ExplorationSquad.h"
#include "../../Managers/ExplorationManager.h"
#include "../RushSquad.h"
#include "../../Managers/AgentManager.h"

using namespace BWAPI;

ProtossMain::ProtossMain() {
  buildplan_.push_back(BuildplanEntry(UnitTypes::Protoss_Pylon, 8));
  buildplan_.push_back(BuildplanEntry(UnitTypes::Protoss_Forge, 9));
  buildplan_.push_back(BuildplanEntry(UnitTypes::Protoss_Gateway, 9));
  buildplan_.push_back(BuildplanEntry(UnitTypes::Protoss_Pylon, 8));
  buildplan_.push_back(BuildplanEntry(UnitTypes::Protoss_Assimilator, 10));
  buildplan_.push_back(BuildplanEntry(UnitTypes::Protoss_Gateway, 12));
  buildplan_.push_back(BuildplanEntry(UnitTypes::Protoss_Pylon, 14));
  buildplan_.push_back(BuildplanEntry(UnitTypes::Protoss_Cybernetics_Core, 15));
  buildplan_.push_back(BuildplanEntry(UnitTypes::Protoss_Stargate, 18));
  buildplan_.push_back(BuildplanEntry(UnitTypes::Protoss_Citadel_of_Adun, 20));
  buildplan_.push_back(BuildplanEntry(UnitTypes::Protoss_Pylon, 22));
  buildplan_.push_back(BuildplanEntry(UnitTypes::Protoss_Templar_Archives, 24));

  mainSquad = std::make_shared<Squad>(1, Squad::SquadType::OFFENSIVE, "MainSquad", 10);
  mainSquad->addSetup(UnitTypes::Protoss_Dragoon, 10);
  mainSquad->setBuildup(true);
  mainSquad->setRequired(true);
  squads_.push_back(mainSquad);

  stealthSquad = std::make_shared<Squad>(2, Squad::SquadType::OFFENSIVE, "StealthSquad", 11);
  stealthSquad->setRequired(false);
  stealthSquad->setBuildup(true);
  squads_.push_back(stealthSquad);

  workers_num_ = 16;
  workers_per_refinery_ = 3;
}

ProtossMain::~ProtossMain() {
}

void ProtossMain::computeActions() {
  computeActionsBase();

  int cSupply = Broodwar->self()->supplyUsed() / 2;
  int min = Broodwar->self()->minerals();
  int gas = Broodwar->self()->gas();

  if (cSupply >= 17 && stage_ == 0) {
    buildplan_.push_back(BuildplanEntry(UnitTypes::Protoss_Photon_Cannon, cSupply));

    mainSquad->addSetup(UnitTypes::Protoss_Dragoon, 8);
    mainSquad->addSetup(UnitTypes::Protoss_Scout, 5);
    buildplan_.push_back(BuildplanEntry(UpgradeTypes::Singularity_Charge, cSupply));
    stage_++;
  }
  if (cSupply >= 30 && stage_ == 1 && AgentManager::getInstance()->countNoFinishedUnits(UnitTypes::Protoss_Templar_Archives) > 0) {
    mainSquad->addSetup(UnitTypes::Protoss_High_Templar, 4);
    mainSquad->setBuildup(false);

    stealthSquad->addSetup(UnitTypes::Protoss_Dark_Templar, 6);
    stealthSquad->setBuildup(false);

    buildplan_.push_back(BuildplanEntry(TechTypes::Psionic_Storm, cSupply));

    stage_++;
  }
  if (min > 400 && stage_ == 2) {
    buildplan_.push_back(BuildplanEntry(UnitTypes::Protoss_Nexus, cSupply));
    buildplan_.push_back(BuildplanEntry(UnitTypes::Protoss_Gateway, cSupply));
    buildplan_.push_back(BuildplanEntry(UnitTypes::Protoss_Stargate, cSupply));
    stage_++;
  }
  if (stage_ == 4 && min > 400 && gas > 150 && AgentManager::getInstance()->countNoFinishedUnits(UnitTypes::Protoss_Gateway) >= 3) {
    buildplan_.push_back(BuildplanEntry(UnitTypes::Protoss_Robotics_Facility, cSupply));
    buildplan_.push_back(BuildplanEntry(UnitTypes::Protoss_Observatory, cSupply));

    mainSquad->addSetup(UnitTypes::Protoss_Observer, 1);

    buildplan_.push_back(BuildplanEntry(UpgradeTypes::Protoss_Ground_Weapons, cSupply));
    buildplan_.push_back(BuildplanEntry(UpgradeTypes::Protoss_Plasma_Shields, cSupply));
    buildplan_.push_back(BuildplanEntry(UpgradeTypes::Protoss_Ground_Weapons, cSupply));
    stage_++;
  }
}
