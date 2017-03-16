#include "TerranMain.h"
#include "Managers/BuildplanEntry.h"
#include "Managers/AgentManager.h"
#include "../ExplorationSquad.h"
#include "../RushSquad.h"
#include "Glob.h"

using namespace BWAPI;

TerranMain::TerranMain() {
  buildplan_.push_back(BuildplanEntry(UnitTypes::Terran_Supply_Depot, 9));
  buildplan_.push_back(BuildplanEntry(UnitTypes::Terran_Barracks, 9));
  buildplan_.push_back(BuildplanEntry(UnitTypes::Terran_Refinery, 12));
  buildplan_.push_back(BuildplanEntry(UnitTypes::Terran_Bunker, 14));
  buildplan_.push_back(BuildplanEntry(UnitTypes::Terran_Supply_Depot, 16));
  buildplan_.push_back(BuildplanEntry(UnitTypes::Terran_Bunker, 16));
  buildplan_.push_back(BuildplanEntry(UnitTypes::Terran_Factory, 18));
  buildplan_.push_back(BuildplanEntry(UnitTypes::Terran_Academy, 20));
  buildplan_.push_back(BuildplanEntry(TechTypes::Tank_Siege_Mode, 22));
  buildplan_.push_back(BuildplanEntry(UnitTypes::Terran_Supply_Depot, 23));
  buildplan_.push_back(BuildplanEntry(TechTypes::Stim_Packs, 29));
  buildplan_.push_back(BuildplanEntry(UpgradeTypes::U_238_Shells, 30));
  buildplan_.push_back(BuildplanEntry(UnitTypes::Terran_Supply_Depot, 31));

  main_sq_ = std::make_shared<Squad>(1, Squad::SquadType::OFFENSIVE, "MainSquad", 10);
  main_sq_->addSetup(UnitTypes::Terran_Marine, 10);
  main_sq_->setRequired(true);
  main_sq_->setBuildup(true);
  squads_.push_back(main_sq_);

  secondary_sq_ = std::make_shared<Squad>(2, Squad::SquadType::OFFENSIVE, "SecondarySquad", 10);
  secondary_sq_->setRequired(false);
  secondary_sq_->setBuildup(true);
  squads_.push_back(secondary_sq_);

  scout1_sq_ = std::make_shared<RushSquad>(10, "RushSquad1", 11);
  scout1_sq_->setRequired(false);
  squads_.push_back(scout1_sq_);

  scout2_sc_ = std::make_shared<ExplorationSquad>(11, "ScoutingSquad2", 11);
  scout2_sc_->setRequired(false);
  squads_.push_back(scout2_sc_);

  backup1_sq_ = std::make_shared<Squad>(5, Squad::SquadType::OFFENSIVE, "BackupSquad1", 11);
  backup1_sq_->setRequired(false);
  backup1_sq_->setBuildup(true);
  squads_.push_back(backup1_sq_);

  backup2_sq_ = std::make_shared<Squad>(6, Squad::SquadType::OFFENSIVE, "BackupSquad2", 12);
  backup2_sq_->setRequired(false);
  backup2_sq_->setBuildup(true);
  squads_.push_back(backup2_sq_);

  workers_num_ = 16;
  workers_per_refinery_ = 2;
}

TerranMain::~TerranMain() {
}

void TerranMain::on_frame() {
  on_frame_base();

  workers_num_ = 12 * rnp::agent_manager()->countNoFinishedUnits(UnitTypes::Terran_Command_Center) + 2 * rnp::agent_manager()->countNoFinishedUnits(UnitTypes::Terran_Refinery);
  if (workers_num_ > 30) workers_num_ = 30;

  int cSupply = Broodwar->self()->supplyUsed() / 2;
  int min = Broodwar->self()->minerals();
//  int gas = Broodwar->self()->gas();

  if (cSupply >= 20 && stage_ == 0) {
    main_sq_->addSetup(UnitTypes::Terran_Siege_Tank_Tank_Mode, 4);
    main_sq_->addSetup(UnitTypes::Terran_SCV, 1);
    main_sq_->addSetup(UnitTypes::Terran_Marine, 6);
    main_sq_->addSetup(UnitTypes::Terran_Medic, 4);

    stage_++;
  }
  if (cSupply >= 30 && min > 200 && stage_ == 1) {
    buildplan_.push_back(BuildplanEntry(UnitTypes::Terran_Factory, cSupply));
    buildplan_.push_back(BuildplanEntry(UnitTypes::Terran_Armory, cSupply));
    buildplan_.push_back(BuildplanEntry(UnitTypes::Terran_Engineering_Bay, cSupply));

    main_sq_->addSetup(UnitTypes::Terran_Goliath, 3);
    main_sq_->setBuildup(false);

    stage_++;
  }
  if (cSupply >= 45 && min > 200 && stage_ == 2) {
    buildplan_.push_back(BuildplanEntry(UnitTypes::Terran_Barracks, cSupply));
    buildplan_.push_back(BuildplanEntry(UnitTypes::Terran_Missile_Turret, cSupply));

    scout1_sq_->addSetup(UnitTypes::Terran_Vulture, 1);
    scout1_sq_->setPriority(1);
    scout1_sq_->setActivePriority(1000);
    squads_.push_back(scout1_sq_);

    secondary_sq_->addSetup(UnitTypes::Terran_Siege_Tank_Tank_Mode, 2);
    secondary_sq_->addSetup(UnitTypes::Terran_Marine, 8);
    secondary_sq_->addSetup(UnitTypes::Terran_Goliath, 2);
    secondary_sq_->setBuildup(false);

    stage_++;
  }
  if (rnp::commander()->getSquad(1)->isActive() && stage_ == 3) {
    buildplan_.push_back(BuildplanEntry(UnitTypes::Terran_Command_Center, cSupply));

    scout2_sc_->addSetup(UnitTypes::Terran_Vulture, 2);
    scout2_sc_->setBuildup(false);

    stage_++;
  }

  if (stage_ == 4 && rnp::agent_manager()->countNoUnits(UnitTypes::Terran_Command_Center) >= 2) {
    buildplan_.push_back(BuildplanEntry(UnitTypes::Terran_Missile_Turret, cSupply));
    buildplan_.push_back(BuildplanEntry(UpgradeTypes::Terran_Vehicle_Weapons, cSupply));
    buildplan_.push_back(BuildplanEntry(UpgradeTypes::Terran_Infantry_Weapons, cSupply));
    buildplan_.push_back(BuildplanEntry(UnitTypes::Terran_Starport, cSupply));
    buildplan_.push_back(BuildplanEntry(UnitTypes::Terran_Science_Facility, cSupply));

    stage_++;
  }
  if (stage_ == 5 && rnp::agent_manager()->countNoFinishedUnits(UnitTypes::Terran_Science_Facility) > 0) {
    buildplan_.push_back(BuildplanEntry(UnitTypes::Terran_Missile_Turret, cSupply));

    main_sq_->addSetup(UnitTypes::Terran_Science_Vessel, 1);

    buildplan_.push_back(BuildplanEntry(TechTypes::Cloaking_Field, cSupply));

    backup1_sq_->addSetup(UnitTypes::Terran_Wraith, 5);
    backup1_sq_->setBuildup(false);

    stage_++;
  }
  if (stage_ == 6 && rnp::agent_manager()->countNoFinishedUnits(UnitTypes::Terran_Science_Vessel) > 0) {
    buildplan_.push_back(BuildplanEntry(TechTypes::EMP_Shockwave, cSupply));
    buildplan_.push_back(BuildplanEntry(UpgradeTypes::Terran_Vehicle_Weapons, cSupply));

    stage_++;
  }
  if (stage_ == 7 && rnp::agent_manager()->countNoFinishedUnits(UnitTypes::Terran_Physics_Lab) > 0) {
    backup2_sq_->addSetup(UnitTypes::Terran_Battlecruiser, 2);
    backup2_sq_->setBuildup(false);
    buildplan_.push_back(BuildplanEntry(TechTypes::Yamato_Gun, cSupply));

    stage_++;
  }
}
