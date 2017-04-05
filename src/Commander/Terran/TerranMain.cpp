#include "TerranMain.h"
#include "Managers/AgentManager.h"
#include "../ExplorationSquad.h"
#include "../RushSquad.h"
#include "Glob.h"
#include "Commander/SquadMsg.h"
#include "Utils/Profiler.h"

#define MODULE_PREFIX "(terran-main) "
using namespace BWAPI;

size_t TerranMain::workers_per_refinery() {
  return 2;
}

size_t TerranMain::adjust_workers_count(size_t workers_now) {
  auto am = rnp::agent_manager();
  size_t have_cmd_centers = am->get_finished_units_count(UnitTypes::Terran_Command_Center);
  size_t have_refineries = am->get_finished_units_count(UnitTypes::Terran_Refinery);

  auto x = std::min<size_t>(30, 12 * have_cmd_centers + 2 * have_refineries);
  return x;
}

TerranMain::TerranMain()
: FsmBaseClass(TerranStrategyState::NoStage)
, main_sq_(), secondary_sq_(), backup1_sq_(), backup2_sq_()
, rush_sq_(), scout2_sq_()
{
}

TerranMain::~TerranMain() {
}

void TerranMain::tick() {
  ProfilerAuto pa(*rnp::profiler(), "OnFrame_TerranMain");

  int c_supply = Broodwar->self()->supplyUsed() / 2;
  int minerals = Broodwar->self()->minerals();
//  int gas = Broodwar->self()->gas();

  switch (fsm_state()) {
  case TerranStrategyState::NoStage:
    return fsm_set_state(TerranStrategyState::Stage1);
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

  act::suspend(self(), 8); // do not revisit this logic too often
}

void TerranMain::on_enter_stage1() {
  auto fn1 =
      [this](Commander* c) {
        c->build_plan_.add(UnitTypes::Terran_Supply_Depot, 9);
        c->build_plan_.add(UnitTypes::Terran_Barracks, 9);
        c->build_plan_.add(UnitTypes::Terran_Refinery, 12);
        c->build_plan_.add(UnitTypes::Terran_Bunker, 14);
        c->build_plan_.add(UnitTypes::Terran_Bunker, 14);

        c->build_plan_.add(UnitTypes::Terran_Supply_Depot, 16);
        c->build_plan_.add(UnitTypes::Terran_Bunker, 16);
        c->build_plan_.add(UnitTypes::Terran_Factory, 18);
        c->build_plan_.add(UnitTypes::Terran_Academy, 20);

        c->build_plan_.add(UnitTypes::Terran_Supply_Depot, 23);
        c->build_plan_.add(TechTypes::Tank_Siege_Mode, 22);
        c->build_plan_.add(TechTypes::Stim_Packs, 29);
        c->build_plan_.add(UpgradeTypes::U_238_Shells, 30);
        c->build_plan_.add(UnitTypes::Terran_Supply_Depot, 31);

        main_sq_ = Squad::spawn(Squad::SquadType::OFFENSIVE, "main1", 10);
        msg::squad::add_setup(main_sq_, UnitTypes::Terran_Marine, 10);
        msg::squad::required(main_sq_, true);
        msg::squad::buildup(main_sq_, true);
        c->squads_.insert(main_sq_);

        secondary_sq_ = Squad::spawn(Squad::SquadType::OFFENSIVE, "main2", 10);
        msg::squad::required(secondary_sq_, false);
        msg::squad::buildup(secondary_sq_, true);
        c->squads_.insert(secondary_sq_);

        rush_sq_ = RushSquad::spawn("rush1", 11);
        msg::squad::required(rush_sq_, false);
        c->squads_.insert(rush_sq_);

        scout2_sq_ = ExplorationSquad::spawn("scout2", 11);
        msg::squad::add_setup(scout2_sq_, UnitTypes::Terran_SCV, 1);
        msg::squad::required(scout2_sq_, false);
        c->squads_.insert(scout2_sq_);

        backup1_sq_ = Squad::spawn(Squad::SquadType::OFFENSIVE, "bckup1", 11);
        msg::squad::required(backup1_sq_, false);
        msg::squad::buildup(backup1_sq_, true);
        c->squads_.insert(backup1_sq_);

        backup2_sq_ = Squad::spawn(Squad::SquadType::OFFENSIVE, "bckup2", 12);
        msg::squad::required(backup2_sq_, false);
        msg::squad::buildup(backup2_sq_, true);
        c->squads_.insert(backup2_sq_);
      };
  act::modify_actor<Commander>(rnp::commander_id(), fn1);
}

void TerranMain::on_enter_stage2() const {
  msg::squad::add_setup(main_sq_, UnitTypes::Terran_Siege_Tank_Tank_Mode, 4);
  msg::squad::add_setup(main_sq_, UnitTypes::Terran_SCV, 1);
  msg::squad::add_setup(main_sq_, UnitTypes::Terran_Marine, 6);
  msg::squad::add_setup(main_sq_, UnitTypes::Terran_Medic, 4);
}

void TerranMain::on_enter_stage3() {
  auto fn3 =
      [this](Commander* c) {
        int c_supply = Broodwar->self()->supplyUsed() / 2;
        c->build_plan_.add(UnitTypes::Terran_Factory, c_supply);
        c->build_plan_.add(UnitTypes::Terran_Armory, c_supply);
        c->build_plan_.add(UnitTypes::Terran_Engineering_Bay, c_supply);
      };
  act::modify_actor<Commander>(rnp::commander_id(), fn3);

  msg::squad::add_setup(main_sq_, UnitTypes::Terran_Goliath, 3);
  msg::squad::buildup(main_sq_, false);
}

void TerranMain::on_enter_stage4() {
  auto fn4 =
      [this](Commander* c) {
        int c_supply = Broodwar->self()->supplyUsed() / 2;
        c->build_plan_.add(UnitTypes::Terran_Barracks, c_supply);
        c->build_plan_.add(UnitTypes::Terran_Missile_Turret, c_supply);

        msg::squad::add_setup(rush_sq_, UnitTypes::Terran_Vulture, 1);
        msg::squad::priority(rush_sq_, 1);
        msg::squad::active_priority(rush_sq_, 1000);
        c->squads_.insert(rush_sq_);
      };
  act::modify_actor<Commander>(rnp::commander_id(), fn4);

  msg::squad::add_setup(secondary_sq_, UnitTypes::Terran_Siege_Tank_Tank_Mode, 2);
  msg::squad::add_setup(secondary_sq_, UnitTypes::Terran_Marine, 8);
  msg::squad::add_setup(secondary_sq_, UnitTypes::Terran_Goliath, 2);
  msg::squad::buildup(secondary_sq_, false);
}

void TerranMain::on_enter_stage5() {
  auto fn5 =
      [this](Commander* c) {
        int c_supply = Broodwar->self()->supplyUsed() / 2;
        c->build_plan_.add(UnitTypes::Terran_Command_Center, c_supply);
      };
  act::modify_actor<Commander>(rnp::commander_id(), fn5);

  msg::squad::add_setup(scout2_sq_, UnitTypes::Terran_Vulture, 2);
  msg::squad::buildup(scout2_sq_, false);
}

void TerranMain::on_enter_stage6() {
  auto fn6 =
      [this](Commander* c) {
        int c_supply = Broodwar->self()->supplyUsed() / 2;
        c->build_plan_.add(UnitTypes::Terran_Missile_Turret, c_supply);
        c->build_plan_.add(UpgradeTypes::Terran_Vehicle_Weapons, c_supply);
        c->build_plan_.add(UpgradeTypes::Terran_Infantry_Weapons, c_supply);
        c->build_plan_.add(UnitTypes::Terran_Starport, c_supply);
        c->build_plan_.add(UnitTypes::Terran_Science_Facility, c_supply);
      };
  act::modify_actor<Commander>(rnp::commander_id(), fn6);
}

void TerranMain::on_enter_stage7() {
  auto fn7 =
      [this](Commander* c) {
        int c_supply = Broodwar->self()->supplyUsed() / 2;
        c->build_plan_.add(UnitTypes::Terran_Missile_Turret, c_supply);
        c->build_plan_.add(TechTypes::Cloaking_Field, c_supply);
      };
  act::modify_actor<Commander>(rnp::commander_id(), fn7);

  msg::squad::add_setup(main_sq_, UnitTypes::Terran_Science_Vessel, 1);
  msg::squad::add_setup(backup1_sq_, UnitTypes::Terran_Wraith, 5);
  msg::squad::buildup(backup1_sq_, false);
}

void TerranMain::on_enter_stage8() {
  auto fn8 =
      [this](Commander* c) {
        int c_supply = Broodwar->self()->supplyUsed() / 2;
        c->build_plan_.add(TechTypes::EMP_Shockwave, c_supply);
        c->build_plan_.add(UpgradeTypes::Terran_Vehicle_Weapons, c_supply);
      };
  act::modify_actor<Commander>(rnp::commander_id(), fn8);
}

void TerranMain::on_enter_endgame() {
  auto fn_endgame =
    [this](Commander* c) {
    int c_supply = Broodwar->self()->supplyUsed() / 2;
    c->build_plan_.add(TechTypes::Yamato_Gun, c_supply);
  };
  act::modify_actor<Commander>(rnp::commander_id(), fn_endgame);

  msg::squad::add_setup(backup2_sq_, UnitTypes::Terran_Battlecruiser, 2);
  msg::squad::buildup(backup2_sq_, false);
}

void TerranMain::fsm_on_transition(TerranStrategyState old_st, 
                                   TerranStrategyState new_st) {
  rnp::log()->debug(MODULE_PREFIX "strategy stage #{}", 
                    static_cast<int>(new_st));

  switch (new_st) {
  case TerranStrategyState::Stage1: return on_enter_stage1();
  case TerranStrategyState::Stage2: return on_enter_stage2();
  case TerranStrategyState::Stage3: return on_enter_stage3(); 
  case TerranStrategyState::Stage4: return on_enter_stage4();
  case TerranStrategyState::Stage5: return on_enter_stage5();
  case TerranStrategyState::Stage6: return on_enter_stage6();
  case TerranStrategyState::Stage7: return on_enter_stage7();
  case TerranStrategyState::Stage8: return on_enter_stage8();
  case TerranStrategyState::EndGame: return on_enter_endgame();
  default: ;
  }
}

#undef MODULE_PREFIX
