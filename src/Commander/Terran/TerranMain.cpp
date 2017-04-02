#include "TerranMain.h"
#include "Managers/AgentManager.h"
#include "../ExplorationSquad.h"
#include "../RushSquad.h"
#include "Glob.h"
#include "Commander/SquadMsg.h"
#include "Utils/Profiler.h"

#define MODULE_PREFIX "(terran-main) "
using namespace BWAPI;

TerranMain::TerranMain()
: FsmBaseClass(TerranStrategyState::Stage1)
, main_sq_(), secondary_sq_(), backup1_sq_(), backup2_sq_()
, rush_sq_(), scout2_sq_()
{
  workers_num_ = 16;
  workers_per_refinery_ = 2;
}

TerranMain::~TerranMain() {
}

void TerranMain::tick() {
  ProfilerAuto pa(*rnp::profiler(), "OnFrame_TerranMain");
  tick_base_commander();

  auto agent_manager = rnp::agent_manager();
  auto have_cmd_centers = agent_manager->get_finished_units_count(UnitTypes::Terran_Command_Center);
  auto have_refineries = agent_manager->get_finished_units_count(UnitTypes::Terran_Refinery);

  workers_num_ = 12 * have_cmd_centers + 2 * have_refineries;
  if (workers_num_ > 30) workers_num_ = 30;

  int c_supply = Broodwar->self()->supplyUsed() / 2;
  int minerals = Broodwar->self()->minerals();
//  int gas = Broodwar->self()->gas();

  switch (fsm_state()) {
  case TerranStrategyState::Stage1:
    if (c_supply >= 20) {
      fsm_set_state(TerranStrategyState::Stage2);
    }
    break;
  case TerranStrategyState::Stage2:
    if (c_supply >= 30 && minerals > 200) {
      fsm_set_state(TerranStrategyState::Stage3);
    }
    break;
  case TerranStrategyState::Stage3:
    if (c_supply >= 45 && minerals > 200) {
      fsm_set_state(TerranStrategyState::Stage4);
    }
    break;
  case TerranStrategyState::Stage4:
    if (rnp::commander()->get_squad(main_sq_)->is_active()) {
      fsm_set_state(TerranStrategyState::Stage5);
    }
    break;
  case TerranStrategyState::Stage5: {
    auto ncomcenters = rnp::agent_manager()->get_units_of_type_count(
      UnitTypes::Terran_Command_Center);
    if (ncomcenters >= 2) {
      fsm_set_state(TerranStrategyState::Stage6);
    }
  }
    break;
  case TerranStrategyState::Stage6: {
    auto nscience = rnp::agent_manager()->get_finished_units_count(
      UnitTypes::Terran_Science_Facility);
    if (nscience > 0) {
      fsm_set_state(TerranStrategyState::Stage7);
    }
  }
    break;
  case TerranStrategyState::Stage7: {
    auto nvessels = rnp::agent_manager()->get_finished_units_count(
      UnitTypes::Terran_Science_Vessel);
    if (nvessels > 0) {
      fsm_set_state(TerranStrategyState::Stage8);
    }
  }
    break;
  case TerranStrategyState::Stage8: {
    auto nlabs = rnp::agent_manager()->get_finished_units_count(
      UnitTypes::Terran_Physics_Lab);
    if (nlabs > 0) {
      fsm_set_state(TerranStrategyState::EndGame);
    }
  } 
    break;
  case TerranStrategyState::EndGame: break;
  default: ;
  }
}

void TerranMain::fsm_on_transition(TerranStrategyState old_st, 
                                   TerranStrategyState new_st) {
  rnp::log()->debug(MODULE_PREFIX "strategy stage #{}", 
                    static_cast<int>(new_st) + 1);

  switch (new_st) {
  case TerranStrategyState::Stage1: {
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

    main_sq_ = Squad::spawn(Squad::SquadType::OFFENSIVE, "main1", 10);
    msg::squad::add_setup(main_sq_, UnitTypes::Terran_Marine, 10);
    msg::squad::required(main_sq_, true);
    msg::squad::buildup(main_sq_, true);
    squads_.insert(main_sq_);

    secondary_sq_ = Squad::spawn(Squad::SquadType::OFFENSIVE, "main2", 10);
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
  }
    break;

  case TerranStrategyState::Stage2:
    msg::squad::add_setup(main_sq_, UnitTypes::Terran_Siege_Tank_Tank_Mode, 4);
    msg::squad::add_setup(main_sq_, UnitTypes::Terran_SCV, 1);
    msg::squad::add_setup(main_sq_, UnitTypes::Terran_Marine, 6);
    msg::squad::add_setup(main_sq_, UnitTypes::Terran_Medic, 4);
    break;

  case TerranStrategyState::Stage3: {
    int c_supply = Broodwar->self()->supplyUsed() / 2;
    build_plan_.add(UnitTypes::Terran_Factory, c_supply);
    build_plan_.add(UnitTypes::Terran_Armory, c_supply);
    build_plan_.add(UnitTypes::Terran_Engineering_Bay, c_supply);

    msg::squad::add_setup(main_sq_, UnitTypes::Terran_Goliath, 3);
    msg::squad::buildup(main_sq_, false);
  }
    break;
  case TerranStrategyState::Stage4: {
    int c_supply = Broodwar->self()->supplyUsed() / 2;
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
  }
    break;
  case TerranStrategyState::Stage5: {
    int c_supply = Broodwar->self()->supplyUsed() / 2;
    build_plan_.add(UnitTypes::Terran_Command_Center, c_supply);

    msg::squad::add_setup(scout2_sq_, UnitTypes::Terran_Vulture, 2);
    msg::squad::buildup(scout2_sq_, false);
  }
    break;
  case TerranStrategyState::Stage6: {
    int c_supply = Broodwar->self()->supplyUsed() / 2;
    build_plan_.add(UnitTypes::Terran_Missile_Turret, c_supply);
    build_plan_.add(UpgradeTypes::Terran_Vehicle_Weapons, c_supply);
    build_plan_.add(UpgradeTypes::Terran_Infantry_Weapons, c_supply);
    build_plan_.add(UnitTypes::Terran_Starport, c_supply);
    build_plan_.add(UnitTypes::Terran_Science_Facility, c_supply);
  }
    break;
  case TerranStrategyState::Stage7: {
    int c_supply = Broodwar->self()->supplyUsed() / 2;
    build_plan_.add(UnitTypes::Terran_Missile_Turret, c_supply);
    msg::squad::add_setup(main_sq_, UnitTypes::Terran_Science_Vessel, 1);
    build_plan_.add(TechTypes::Cloaking_Field, c_supply);
    msg::squad::add_setup(backup1_sq_, UnitTypes::Terran_Wraith, 5);
    msg::squad::buildup(backup1_sq_, false);
  }
    break;
  case TerranStrategyState::Stage8: {
    int c_supply = Broodwar->self()->supplyUsed() / 2;
    build_plan_.add(TechTypes::EMP_Shockwave, c_supply);
    build_plan_.add(UpgradeTypes::Terran_Vehicle_Weapons, c_supply);
  }
    break;
  case TerranStrategyState::EndGame: {
    int c_supply = Broodwar->self()->supplyUsed() / 2;
    msg::squad::add_setup(backup2_sq_, UnitTypes::Terran_Battlecruiser, 2);
    msg::squad::buildup(backup2_sq_, false);
    build_plan_.add(TechTypes::Yamato_Gun, c_supply);
  }
    break;
  default: ;
  }
}

#undef MODULE_PREFIX
