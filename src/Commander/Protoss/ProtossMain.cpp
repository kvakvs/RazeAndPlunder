#if 0
#include "ProtossMain.h"
#include "Managers/BuildplanEntry.h"
#include "Managers/ExplorationManager.h"
#include "Managers/AgentManager.h"
#include "Glob.h"

using namespace BWAPI;

ProtossMain::ProtossMain()
    : main_sq_(), stealth_sq_(), detector_sq_()
{
  plan(UnitTypes::Protoss_Pylon, 8);
  plan(UnitTypes::Protoss_Forge, 9);
  plan(UnitTypes::Protoss_Gateway, 9);
  plan(UnitTypes::Protoss_Pylon, 8);
  plan(UnitTypes::Protoss_Assimilator, 10);
  plan(UnitTypes::Protoss_Gateway, 12);
  plan(UnitTypes::Protoss_Pylon, 14);
  plan(UnitTypes::Protoss_Cybernetics_Core, 15);
  plan(UnitTypes::Protoss_Stargate, 18);
  plan(UnitTypes::Protoss_Citadel_of_Adun, 20);
  plan(UnitTypes::Protoss_Pylon, 22);
  plan(UnitTypes::Protoss_Templar_Archives, 24);

  main_sq_ = std::make_shared<Squad>(1, SquadType::OFFENSIVE, "MainSquad", 10);
  main_sq_->add_setup(UnitTypes::Protoss_Dragoon, 10);
  main_sq_->set_buildup(true);
  main_sq_->set_required(true);
  squads_.push_back(main_sq_);

  stealth_sq_ = std::make_shared<Squad>(2, SquadType::OFFENSIVE, "StealthSquad", 11);
  stealth_sq_->set_required(false);
  stealth_sq_->set_buildup(true);
  squads_.push_back(stealth_sq_);

  workers_num_ = 16;
  workers_per_refinery_ = 3;
}

ProtossMain::~ProtossMain() {
}

void ProtossMain::on_frame() {
  auto agent_manager = rnp::agent_manager();
  
  on_frame_base();

  int cSupply = Broodwar->self()->supplyUsed() / 2;
  int min = Broodwar->self()->minerals();
  int gas = Broodwar->self()->gas();

  if (cSupply >= 17 && stage_ == 0) {
    plan(UnitTypes::Protoss_Photon_Cannon, cSupply);

    main_sq_->add_setup(UnitTypes::Protoss_Dragoon, 8);
    main_sq_->add_setup(UnitTypes::Protoss_Scout, 5);
    plan(UpgradeTypes::Singularity_Charge, cSupply);
    stage_++;
  }
  if (cSupply >= 30 && stage_ == 1 
    && agent_manager->get_finished_units_count(UnitTypes::Protoss_Templar_Archives) > 0) {
    main_sq_->add_setup(UnitTypes::Protoss_High_Templar, 4);
    main_sq_->set_buildup(false);

    stealth_sq_->add_setup(UnitTypes::Protoss_Dark_Templar, 6);
    stealth_sq_->set_buildup(false);

    plan(TechTypes::Psionic_Storm, cSupply);

    stage_++;
  }
  if (min > 400 && stage_ == 2) {
    plan(UnitTypes::Protoss_Nexus, cSupply);
    plan(UnitTypes::Protoss_Gateway, cSupply);
    plan(UnitTypes::Protoss_Stargate, cSupply);
    stage_++;
  }
  if (stage_ == 4 && min > 400 && gas > 150 
    && agent_manager->get_finished_units_count(UnitTypes::Protoss_Gateway) >= 3) {
    plan(UnitTypes::Protoss_Robotics_Facility, cSupply);
    plan(UnitTypes::Protoss_Observatory, cSupply);

    main_sq_->add_setup(UnitTypes::Protoss_Observer, 1);

    plan(UpgradeTypes::Protoss_Ground_Weapons, cSupply);
    plan(UpgradeTypes::Protoss_Plasma_Shields, cSupply);
    plan(UpgradeTypes::Protoss_Ground_Weapons, cSupply);
    stage_++;
  }
}
#endif // 0
