#include "ProtossMain.h"
#include "Managers/BuildplanEntry.h"
#include "Managers/ExplorationManager.h"
#include "Managers/AgentManager.h"
#include "Glob.h"

using namespace BWAPI;

ProtossMain::ProtossMain()
    : main_sq_(), stealth_sq_(), detector_sq_()
{
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

  main_sq_ = std::make_shared<Squad>(1, Squad::SquadType::OFFENSIVE, "MainSquad", 10);
  main_sq_->addSetup(UnitTypes::Protoss_Dragoon, 10);
  main_sq_->setBuildup(true);
  main_sq_->setRequired(true);
  squads_.push_back(main_sq_);

  stealth_sq_ = std::make_shared<Squad>(2, Squad::SquadType::OFFENSIVE, "StealthSquad", 11);
  stealth_sq_->setRequired(false);
  stealth_sq_->setBuildup(true);
  squads_.push_back(stealth_sq_);

  workers_num_ = 16;
  workers_per_refinery_ = 3;
}

ProtossMain::~ProtossMain() {
}

void ProtossMain::on_frame() {
  on_frame_base();

  int cSupply = Broodwar->self()->supplyUsed() / 2;
  int min = Broodwar->self()->minerals();
  int gas = Broodwar->self()->gas();

  if (cSupply >= 17 && stage_ == 0) {
    buildplan_.push_back(BuildplanEntry(UnitTypes::Protoss_Photon_Cannon, cSupply));

    main_sq_->addSetup(UnitTypes::Protoss_Dragoon, 8);
    main_sq_->addSetup(UnitTypes::Protoss_Scout, 5);
    buildplan_.push_back(BuildplanEntry(UpgradeTypes::Singularity_Charge, cSupply));
    stage_++;
  }
  if (cSupply >= 30 && stage_ == 1 && rnp::agent_manager()->countNoFinishedUnits(UnitTypes::Protoss_Templar_Archives) > 0) {
    main_sq_->addSetup(UnitTypes::Protoss_High_Templar, 4);
    main_sq_->setBuildup(false);

    stealth_sq_->addSetup(UnitTypes::Protoss_Dark_Templar, 6);
    stealth_sq_->setBuildup(false);

    buildplan_.push_back(BuildplanEntry(TechTypes::Psionic_Storm, cSupply));

    stage_++;
  }
  if (min > 400 && stage_ == 2) {
    buildplan_.push_back(BuildplanEntry(UnitTypes::Protoss_Nexus, cSupply));
    buildplan_.push_back(BuildplanEntry(UnitTypes::Protoss_Gateway, cSupply));
    buildplan_.push_back(BuildplanEntry(UnitTypes::Protoss_Stargate, cSupply));
    stage_++;
  }
  if (stage_ == 4 && min > 400 && gas > 150 && rnp::agent_manager()->countNoFinishedUnits(UnitTypes::Protoss_Gateway) >= 3) {
    buildplan_.push_back(BuildplanEntry(UnitTypes::Protoss_Robotics_Facility, cSupply));
    buildplan_.push_back(BuildplanEntry(UnitTypes::Protoss_Observatory, cSupply));

    main_sq_->addSetup(UnitTypes::Protoss_Observer, 1);

    buildplan_.push_back(BuildplanEntry(UpgradeTypes::Protoss_Ground_Weapons, cSupply));
    buildplan_.push_back(BuildplanEntry(UpgradeTypes::Protoss_Plasma_Shields, cSupply));
    buildplan_.push_back(BuildplanEntry(UpgradeTypes::Protoss_Ground_Weapons, cSupply));
    stage_++;
  }
}
