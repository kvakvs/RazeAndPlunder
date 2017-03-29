#include "StructureAgent.h"
#include "Managers/AgentManager.h"
#include "Managers/Constructor.h"
#include "Managers/Upgrader.h"
#include "Commander/Commander.h"
#include "MainAgents/WorkerAgent.h"
#include "Managers/ResourceManager.h"
#include <iso646.h>
#include "Glob.h"

using namespace BWAPI;

StructureAgent::StructureAgent() {

}

StructureAgent::~StructureAgent() {

}

StructureAgent::StructureAgent(Unit mUnit) {
  unit_ = mUnit;
  type_ = unit_->getType();
  unit_id_ = unit_->getID();
  agent_type_ = "StructureAgent";
}

void StructureAgent::debug_show_goal() const {
  if (not is_alive()) return;

  auto unit_type = unit_->getType();

  //Draw "is working" box
  int total = 0;
  int done = 0;
  std::string txt;
  Color cColor = Colors::Blue;
  int bar_h = 18;

  if (unit_->isBeingConstructed()) {
    total = unit_type.buildTime();
    done = total - unit_->getRemainingBuildTime();
    txt = "";
    bar_h = 8;
  }
  if (not unit_->isBeingConstructed() && unit_type.isResourceContainer()) {
    total = unit_->getInitialResources();
    done = unit_->getResources();
    txt = "";
    cColor = Colors::Orange;
    bar_h = 8;
  }
  if (unit_->isResearching()) {
    total = unit_->getTech().researchTime();
    done = total - unit_->getRemainingResearchTime();
    txt = unit_->getTech().getName();
  }
  if (unit_->isUpgrading()) {
    total = unit_->getUpgrade().upgradeTime();
    done = total - unit_->getRemainingUpgradeTime();
    txt = unit_->getUpgrade().getName();
  }
  if (unit_->isTraining()) {
    auto t_queue = unit_->getTrainingQueue();
    if (not  t_queue.empty()) {
      auto& t = *(t_queue.begin());
      total = t.buildTime();
      txt = t.getName();
      done = total - unit_->getRemainingTrainTime();
    }
  }

  if (total > 0) {
    int w = unit_type.tileWidth() * 32;
    int h = unit_type.tileHeight() * 32;

    //Start 
    Position s(unit_->getPosition().x - w / 2, unit_->getPosition().y - 30);
    //End
    Position e(s.x + w, s.y + bar_h);
    //Progress
    int prg = (int)((double)done / (double)total * w);
    Position p(s.x + prg, s.y + bar_h);

    Broodwar->drawBoxMap(s.x, s.y, e.x, e.y, cColor, false);
    Broodwar->drawBoxMap(s.x, s.y, p.x, p.y, cColor, true);

    Broodwar->drawTextMap(s.x + 5, s.y + 2, txt.c_str());
  }

  if (not  unit_->isBeingConstructed() && unit_type.getID() == UnitTypes::Terran_Bunker.getID()) {
    int w = unit_type.tileWidth() * 32;
    int h = unit_type.tileHeight() * 32;

    auto unit_pos = unit_->getPosition();
    Broodwar->drawTextMap(unit_pos.x - w / 2, unit_pos.y - 10, unit_type.getName().c_str());

    //Draw Loaded box
    Position a(unit_pos.x - w / 2, unit_pos.y - h / 2);
    Position b(a.x + 94, a.y + 6);
    Broodwar->drawBoxMap(a.x, a.y, b.x, b.y, Colors::Green, false);

    auto loaded_units = unit_->getLoadedUnits();
    if (not  loaded_units.empty()) {
      Position a1(unit_pos.x - w / 2, unit_pos.y - h / 2);
      Position b1(a1.x + loaded_units.size() * 24, a1.y + 6);

      Broodwar->drawBoxMap(a1.x, a1.y, b1.x, b1.y, Colors::Green, true);
    }
  }
}

void StructureAgent::tick() {
  if (is_alive()) {
    if (not unit_->isIdle()) return;

    if (rnp::upgrader()->check_upgrade(this)) return;

    if (Constructor::is_terran()) {
      //Check addons here
      if (is_of_type(UnitTypes::Terran_Science_Facility)) {
        if (unit_->getAddon() == nullptr) {
          unit_->buildAddon(UnitTypes::Terran_Physics_Lab);
        }
      }
      if (is_of_type(UnitTypes::Terran_Starport)) {
        if (unit_->getAddon() == nullptr) {
          unit_->buildAddon(UnitTypes::Terran_Control_Tower);
        }
      }
      if (is_of_type(UnitTypes::Terran_Factory)) {
        if (unit_->getAddon() == nullptr) {
          unit_->buildAddon(UnitTypes::Terran_Machine_Shop);
        }
      }
    }

    if (not unit_->isBeingConstructed() 
        && unit_->isIdle()
        && get_unit()->getType().canProduce()) {
      //Iterate through all unit types
      for (auto& u : UnitTypes::allUnitTypes()) {
        //Check if we can (and need) to build the unit
        if (can_build_unit(u)) {
          //Build it!
          unit_->train(u);
        }
      }
    }

    //Check for Spire upgrade
    if (is_of_type(UnitTypes::Zerg_Spire)) {
      if (Broodwar->canMake(UnitTypes::Zerg_Greater_Spire, unit_) && rnp::agent_manager()->get_finished_units_count(UnitTypes::Zerg_Hive) > 0) {
        if (rnp::resources()->has_resources(UnitTypes::Zerg_Greater_Spire)) {
          rnp::resources()->lock_resources(UnitTypes::Zerg_Greater_Spire);
          unit_->morph(UnitTypes::Zerg_Greater_Spire);
          return;
        }
      }
    }

    //Check for Creep Colony upgrade
    if (is_of_type(UnitTypes::Zerg_Creep_Colony)) {
      if (rnp::resources()->has_resources(UnitTypes::Zerg_Sunken_Colony)) {
        rnp::resources()->lock_resources(UnitTypes::Zerg_Sunken_Colony);
        unit_->morph(UnitTypes::Zerg_Sunken_Colony);
        return;
      }
    }
  }
}

bool StructureAgent::can_build(UnitType type) const {
  //1. Check if this unit can construct the unit
  std::pair<UnitType, int> builder = type.whatBuilds();
  if (builder.first.getID() != unit_->getType().getID()) {
    return false;
  }

  //2. Check canBuild
  if (not Broodwar->canMake(type, unit_)) {
    return false;
  }

  //3. Check if we have enough resources
  if (not rnp::resources()->has_resources(type)) {
    return false;
  }

  //All clear. Build the unit.
  return true;
}

bool StructureAgent::can_build_unit(UnitType type) const {
  //1. Check if race matches
  if (type.getRace().getID() != Broodwar->self()->getRace().getID()) {
    return false;
  }

  //2. Check if this unit can construct the unit
  std::pair<UnitType, int> builder = type.whatBuilds();
  if (builder.first.getID() != unit_->getType().getID()) {
    return false;
  }

  //3. Check if we need the unit in a squad
  if (not rnp::commander()->is_unit_needed(type)) {
    return false;
  }

  //4. Check canBuild
  if (not Broodwar->canMake(type, unit_)) {
    return false;
  }

  //5. Check if we have enough resources
  if (not rnp::resources()->has_resources(type)) {
    return false;
  }

  //All clear. Build the unit.
  return true;
}

void StructureAgent::debug_print_info() const {
  int sx = unit_->getPosition().x;
  int sy = unit_->getPosition().y;

  int h = sy + 46;
  if (squad_id_.is_valid()) h += 15;
  if (is_of_type(UnitTypes::Terran_Bunker)) h += 60;

  Broodwar->drawBoxMap(sx - 2, sy, sx + 102, h, Colors::Black, true);
  Broodwar->drawTextMap(sx, sy, "\x03%s", format(unit_->getType().getName()).c_str());
  Broodwar->drawLineMap(sx, sy + 14, sx + 100, sy + 14, Colors::Purple);

  Broodwar->drawTextMap(sx + 2, sy + 15, "Id: \x11%d", unit_id_);
  Broodwar->drawTextMap(sx + 2, sy + 30, "Position: \x11(%d,%d)",
                        unit_->getTilePosition().x, unit_->getTilePosition().y);
  if (squad_id_.is_valid()) {
    Broodwar->drawTextMap(sx + 2, sy + 45, "Squad: \x11%d", squad_id_);
  }

  Broodwar->drawLineMap(sx, h - 61, sx + 100, h - 61, Colors::Purple);

  if (is_of_type(UnitTypes::Terran_Bunker)) {
    auto sq = rnp::commander()->get_squad(squad_id_);
    if (sq) {
      int no = 0;
      act::for_each_in<BaseAgent>(
        sq->get_members(),
        [sx,h,&no](const BaseAgent* a) {
        Broodwar->drawTextMap(sx + 2, h - 60 + 15 * no, "%s \x11(%d)",
                              format(a->unit_type().getName()).c_str(), 
                              a->get_unit_id());
        no++;
      });
    }
    Broodwar->drawLineMap(sx, h - 1, sx + 100, h - 1, Colors::Purple);
  }
}

void StructureAgent::send_workers() {
  //We have constructed a new base. Make some workers move here.
  int no_workers = rnp::agent_manager()->get_workers_count();
  int to_send = no_workers / rnp::agent_manager()->get_bases_count();
  int has_sent = 0;

  act::for_each_actor<BaseAgent>(
    [this,&has_sent,&to_send](const BaseAgent* a) {
      if (a->is_available_worker()) {
        Unit worker = a->get_unit();
        //auto wa = (WorkerAgent*)a.get();
        worker->rightClick(unit_->getPosition());
        has_sent++;
      }

      if (has_sent >= to_send) {
        return;
      }
    });
}

bool StructureAgent::can_morph_into(UnitType type) const {
  //1. Check canBuild
  if (not Broodwar->canMake(type, unit_)) {
    return false;
  }

  //Zerg morph units sometimes bugs in canMake, so we need to do an extra check
  if (type.getID() == UnitTypes::Zerg_Defiler_Mound.getID() && rnp::agent_manager()->get_finished_units_count(UnitTypes::Zerg_Greater_Spire) == 0) {
    return false;
  }

  //2. Check if we have enough resources
  if (not rnp::resources()->has_resources(type)) {
    return false;
  }

  //3. Check if unit is already morphing
  if (unit_->isMorphing()) {
    return false;
  }

  //All clear. Build the unit.
  return true;
}

bool StructureAgent::can_evolve_unit(UnitType type) const {
  //1. Check if we need the unit in a squad
  if (not rnp::commander()->is_unit_needed(type)) {
    return false;
  }

  //2. Check canBuild
  if (not Broodwar->canMake(type, unit_)) {
    return false;
  }

  //3. Check if we have enough resources
  if (not rnp::resources()->has_resources(type)) {
    return false;
  }

  //All clear. Build the unit.
  return true;
}
