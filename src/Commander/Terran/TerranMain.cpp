#include "TerranMain.h"
#include "Managers/AgentManager.h"
#include "../ExplorationSquad.h"
#include "../RushSquad.h"
#include "Glob.h"
#include "Commander/SquadMsg.h"
#include "Utils/Profiler.h"

using namespace BWAPI;

TerranMain::TerranMain()
: main_sq_(), secondary_sq_(), rush_sq_(), scout2_sq_(), backup1_sq_()
    , backup2_sq_()
{
  build_plan_.add(UnitTypes::Terran_Supply_Depot, 9);
  build_plan_.add(UnitTypes::Terran_Barracks, 9);
  build_plan_.add(UnitTypes::Terran_Refinery, 12);
  build_plan_.add(UnitTypes::Terran_Bunker, 14);
  build_plan_.add(UnitTypes::Terran_Bunker, 14);

  build_plan_.add(UnitTypes::Terran_Supply_Depot, 16);
  build_plan_.add(UnitTypes::Terran_Bunker, 16);
  build_plan_.add(UnitTypes::Terran_Factory, 18);
  build_plan_.add(UnitTypes::Terran_Academy, 20);

  build_plan_.add(UnitTypes::Terran_Supply_Depot, 23);
  build_plan_.add(TechTypes::Tank_Siege_Mode, 22);
  build_plan_.add(TechTypes::Stim_Packs, 29);
  build_plan_.add(UpgradeTypes::U_238_Shells, 30);
  build_plan_.add(UnitTypes::Terran_Supply_Depot, 31);

  main_sq_ = Squad::spawn(Squad::SquadType::OFFENSIVE, "main", 10);
  msg::squad::add_setup(main_sq_, UnitTypes::Terran_Marine, 10);
  msg::squad::required(main_sq_, true);
  msg::squad::buildup(main_sq_, true);
  squads_.insert(main_sq_);

  secondary_sq_ = Squad::spawn(Squad::SquadType::OFFENSIVE, "scndry", 10);
  msg::squad::required(secondary_sq_, false);
  msg::squad::buildup(secondary_sq_, true);
  squads_.insert(secondary_sq_);

  rush_sq_ = RushSquad::spawn("rush1", 11);
  msg::squad::required(rush_sq_, false);
  squads_.insert(rush_sq_);

  scout2_sq_ = ExplorationSquad::spawn("scout2", 11);
  msg::squad::add_setup(scout2_sq_, UnitTypes::Terran_SCV, 1);
  msg::squad::required(scout2_sq_, false);
  squads_.insert(scout2_sq_);

  backup1_sq_ = Squad::spawn(Squad::SquadType::OFFENSIVE, "bckup1", 11);
  msg::squad::required(backup1_sq_, false);
  msg::squad::buildup(backup1_sq_, true);
  squads_.insert(backup1_sq_);

  backup2_sq_ = Squad::spawn(Squad::SquadType::OFFENSIVE, "bckup2", 12);
  msg::squad::required(backup2_sq_, false);
  msg::squad::buildup(backup2_sq_, true);
  squads_.insert(backup2_sq_);

  workers_num_ = 16;
  workers_per_refinery_ = 2;
}

TerranMain::~TerranMain() {
}

void TerranMain::tick() {
  rnp::profiler()->start("OnFrame_TerranMain");
  tick_base_commander();

  auto agent_manager = rnp::agent_manager();
  auto have_cmd_centers = agent_manager->get_finished_units_count(UnitTypes::Terran_Command_Center);
  auto have_refineries = agent_manager->get_finished_units_count(UnitTypes::Terran_Refinery);

  workers_num_ = 12 * have_cmd_centers + 2 * have_refineries;
  if (workers_num_ > 30) workers_num_ = 30;

  int c_supply = Broodwar->self()->supplyUsed() / 2;
  int minerals = Broodwar->self()->minerals();
//  int gas = Broodwar->self()->gas();

  if (c_supply >= 20 && stage_ == 0) {
    msg::squad::add_setup(main_sq_, UnitTypes::Terran_Siege_Tank_Tank_Mode, 4);
    msg::squad::add_setup(main_sq_, UnitTypes::Terran_SCV, 1);
    msg::squad::add_setup(main_sq_, UnitTypes::Terran_Marine, 6);
    msg::squad::add_setup(main_sq_, UnitTypes::Terran_Medic, 4);

    stage_++;
  }
  if (c_supply >= 30 && minerals > 200 && stage_ == 1) {
    build_plan_.add(UnitTypes::Terran_Factory, c_supply);
    build_plan_.add(UnitTypes::Terran_Armory, c_supply);
    build_plan_.add(UnitTypes::Terran_Engineering_Bay, c_supply);

    msg::squad::add_setup(main_sq_, UnitTypes::Terran_Goliath, 3);
    msg::squad::buildup(main_sq_, false);

    stage_++;
  }
  if (c_supply >= 45 && minerals > 200 && stage_ == 2) {
    build_plan_.add(UnitTypes::Terran_Barracks, c_supply);
    build_plan_.add(UnitTypes::Terran_Missile_Turret, c_supply);

    msg::squad::add_setup(rush_sq_, UnitTypes::Terran_Vulture, 1);
    msg::squad::priority(rush_sq_, 1);
    msg::squad::active_priority(rush_sq_, 1000);
    squads_.insert(rush_sq_);

    msg::squad::add_setup(secondary_sq_, UnitTypes::Terran_Siege_Tank_Tank_Mode, 2);
    msg::squad::add_setup(secondary_sq_, UnitTypes::Terran_Marine, 8);
    msg::squad::add_setup(secondary_sq_, UnitTypes::Terran_Goliath, 2);
    msg::squad::buildup(secondary_sq_, false);

    stage_++;
  }

  if (rnp::commander()->get_squad(main_sq_)->is_active() && stage_ == 3) {
    build_plan_.add(UnitTypes::Terran_Command_Center, c_supply);

    msg::squad::add_setup(scout2_sq_, UnitTypes::Terran_Vulture, 2);
    msg::squad::buildup(scout2_sq_, false);

    stage_++;
  }

  auto ncomcenters = rnp::agent_manager()->get_units_of_type_count(
    UnitTypes::Terran_Command_Center);
  if (stage_ == 4 && ncomcenters >= 2) {
    build_plan_.add(UnitTypes::Terran_Missile_Turret, c_supply);
    build_plan_.add(UpgradeTypes::Terran_Vehicle_Weapons, c_supply);
    build_plan_.add(UpgradeTypes::Terran_Infantry_Weapons, c_supply);
    build_plan_.add(UnitTypes::Terran_Starport, c_supply);
    build_plan_.add(UnitTypes::Terran_Science_Facility, c_supply);

    stage_++;
  }
  auto nscience = rnp::agent_manager()->get_finished_units_count(
    UnitTypes::Terran_Science_Facility);
  if (stage_ == 5 && nscience > 0) {
    build_plan_.add(UnitTypes::Terran_Missile_Turret, c_supply);

    msg::squad::add_setup(main_sq_, UnitTypes::Terran_Science_Vessel, 1);

    build_plan_.add(TechTypes::Cloaking_Field, c_supply);

    msg::squad::add_setup(backup1_sq_, UnitTypes::Terran_Wraith, 5);
    msg::squad::buildup(backup1_sq_, false);

    stage_++;
  }
  auto nvessels = rnp::agent_manager()->get_finished_units_count(
    UnitTypes::Terran_Science_Vessel);
  if (stage_ == 6 && nvessels > 0) {
    build_plan_.add(TechTypes::EMP_Shockwave, c_supply);
    build_plan_.add(UpgradeTypes::Terran_Vehicle_Weapons, c_supply);

    stage_++;
  }
  auto nlabs = rnp::agent_manager()->get_finished_units_count(
    UnitTypes::Terran_Physics_Lab);
  if (stage_ == 7 && nlabs > 0) {
    msg::squad::add_setup(backup2_sq_, UnitTypes::Terran_Battlecruiser, 2);
    msg::squad::buildup(backup2_sq_, false);
    build_plan_.add(TechTypes::Yamato_Gun, c_supply);

    stage_++;
  }
  rnp::profiler()->end("OnFrame_TerranMain");
}
