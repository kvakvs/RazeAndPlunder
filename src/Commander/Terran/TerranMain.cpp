#include "TerranMain.h"
#include "Commander/ExplorationSquad.h"
#include "Commander/RnpArmy.h"
#include "Commander/RushSquad.h"
#include "Commander/SquadMsg.h"
#include "Glob.h"
#include "Managers/AgentManager.h"
#include "Utils/Profiler.h"

#define MODULE_PREFIX "(terran-main) "
using namespace BWAPI;

size_t TerranMain::workers_per_refinery() const {
  return 2;
}

size_t TerranMain::adjust_workers_count(size_t workers_now) const {
  auto am = rnp::agent_manager();
  size_t have_cmd_centers = am->get_finished_units_count(UnitTypes::Terran_Command_Center);
  size_t have_refineries = am->get_finished_units_count(UnitTypes::Terran_Refinery);

  auto x = std::min<size_t>(30, 12 * have_cmd_centers + 2 * have_refineries);
  return x;
}

TerranMain::TerranMain()
: FsmBaseClass(TerranStrategyState::NoStage)
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
    //if (rnp::commander()->get_squad(main_sq_)->is_active()) {
    rnp::log()->warn(MODULE_PREFIX "skip stage4, TODO check army is ready");
      fsm_set_state(TerranStrategyState::Stage5);
    //}
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

        msg::army::add_setup(UnitTypes::Terran_Marine, 10);
        //msg::army::add_setup(UnitTypes::Terran_SCV, 1);
        // TODO: pick a random SCV and give it scouting orders/temporary scout squad
      };
  act::modify_actor<Commander>(rnp::commander_id(), fn1);
}

void TerranMain::on_enter_stage2() const {
  msg::army::add_setup(UnitTypes::Terran_Siege_Tank_Tank_Mode, 4);
  msg::army::add_setup(UnitTypes::Terran_SCV, 1);
  msg::army::add_setup(UnitTypes::Terran_Marine, 6);
  msg::army::add_setup(UnitTypes::Terran_Medic, 4);
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
  msg::army::add_setup(UnitTypes::Terran_Goliath, 3);
}

void TerranMain::on_enter_stage4() {
  auto fn4 =
      [this](Commander* c) {
        int c_supply = Broodwar->self()->supplyUsed() / 2;
        c->build_plan_.add(UnitTypes::Terran_Barracks, c_supply);
        c->build_plan_.add(UnitTypes::Terran_Missile_Turret, c_supply);
        msg::army::add_setup(UnitTypes::Terran_Vulture, 1);
      };
  act::modify_actor<Commander>(rnp::commander_id(), fn4);

  msg::army::add_setup(UnitTypes::Terran_Siege_Tank_Tank_Mode, 2);
  msg::army::add_setup(UnitTypes::Terran_Marine, 8);
  msg::army::add_setup(UnitTypes::Terran_Goliath, 2);
}

void TerranMain::on_enter_stage5() {
  auto fn5 =
      [this](Commander* c) {
        int c_supply = Broodwar->self()->supplyUsed() / 2;
        c->build_plan_.add(UnitTypes::Terran_Command_Center, c_supply);
      };
  act::modify_actor<Commander>(rnp::commander_id(), fn5);

  msg::army::add_setup(UnitTypes::Terran_Vulture, 2);
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

  msg::army::add_setup(UnitTypes::Terran_Science_Vessel, 1);
  msg::army::add_setup(UnitTypes::Terran_Wraith, 5);
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

  msg::army::add_setup(UnitTypes::Terran_Battlecruiser, 2);
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
