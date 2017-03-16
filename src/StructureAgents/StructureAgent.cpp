#include "StructureAgent.h"
#include "../Managers/AgentManager.h"
#include "../Managers/Constructor.h"
#include "../Managers/Upgrader.h"
#include "Commander/Commander.h"
#include "MainAgents/WorkerAgent.h"
#include "../Managers/ResourceManager.h"
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

void StructureAgent::debug_showGoal() {
  if (not isAlive()) return;

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
      Position a(unit_pos.x - w / 2, unit_pos.y - h / 2);
      Position b(a.x + loaded_units.size() * 24, a.y + 6);

      Broodwar->drawBoxMap(a.x, a.y, b.x, b.y, Colors::Green, true);
    }
  }
}

void StructureAgent::computeActions() {
  if (isAlive()) {
    if (not unit_->isIdle()) return;

    if (rnp::upgrader()->checkUpgrade(this)) return;

    if (Constructor::isTerran()) {
      //Check addons here
      if (isOfType(UnitTypes::Terran_Science_Facility)) {
        if (unit_->getAddon() == nullptr) {
          unit_->buildAddon(UnitTypes::Terran_Physics_Lab);
        }
      }
      if (isOfType(UnitTypes::Terran_Starport)) {
        if (unit_->getAddon() == nullptr) {
          unit_->buildAddon(UnitTypes::Terran_Control_Tower);
        }
      }
      if (isOfType(UnitTypes::Terran_Factory)) {
        if (unit_->getAddon() == nullptr) {
          unit_->buildAddon(UnitTypes::Terran_Machine_Shop);
        }
      }
    }

    if (not unit_->isBeingConstructed() && unit_->isIdle() && getUnit()->getType().canProduce()) {
      //Iterate through all unit types
      for (auto& u : UnitTypes::allUnitTypes()) {
        //Check if we can (and need) to build the unit
        if (canBuildUnit(u)) {
          //Build it!
          unit_->train(u);
        }
      }
    }

    //Check for Spire upgrade
    if (isOfType(UnitTypes::Zerg_Spire)) {
      if (Broodwar->canMake(UnitTypes::Zerg_Greater_Spire, unit_) && rnp::agent_manager()->countNoFinishedUnits(UnitTypes::Zerg_Hive) > 0) {
        if (rnp::resources()->hasResources(UnitTypes::Zerg_Greater_Spire)) {
          rnp::resources()->lockResources(UnitTypes::Zerg_Greater_Spire);
          unit_->morph(UnitTypes::Zerg_Greater_Spire);
          return;
        }
      }
    }

    //Check for Creep Colony upgrade
    if (isOfType(UnitTypes::Zerg_Creep_Colony)) {
      if (rnp::resources()->hasResources(UnitTypes::Zerg_Sunken_Colony)) {
        rnp::resources()->lockResources(UnitTypes::Zerg_Sunken_Colony);
        unit_->morph(UnitTypes::Zerg_Sunken_Colony);
        return;
      }
    }
  }
}

bool StructureAgent::canBuild(UnitType type) {
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
  if (not rnp::resources()->hasResources(type)) {
    return false;
  }

  //All clear. Build the unit.
  return true;
}

bool StructureAgent::canBuildUnit(UnitType type) {
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
  if (not rnp::resources()->hasResources(type)) {
    return false;
  }

  //All clear. Build the unit.
  return true;
}

void StructureAgent::printInfo() {
  int sx = unit_->getPosition().x;
  int sy = unit_->getPosition().y;

  int h = sy + 46;
  if (squad_id_ != -1) h += 15;
  if (isOfType(UnitTypes::Terran_Bunker)) h += 60;

  Broodwar->drawBoxMap(sx - 2, sy, sx + 102, h, Colors::Black, true);
  Broodwar->drawTextMap(sx, sy, "\x03%s", format(unit_->getType().getName()).c_str());
  Broodwar->drawLineMap(sx, sy + 14, sx + 100, sy + 14, Colors::Purple);

  Broodwar->drawTextMap(sx + 2, sy + 15, "Id: \x11%d", unit_id_);
  Broodwar->drawTextMap(sx + 2, sy + 30, "Position: \x11(%d,%d)", unit_->getTilePosition().x, unit_->getTilePosition().y);
  if (squad_id_ != -1) Broodwar->drawTextMap(sx + 2, sy + 45, "Squad: \x11%d", squad_id_);

  Broodwar->drawLineMap(sx, h - 61, sx + 100, h - 61, Colors::Purple);

  if (isOfType(UnitTypes::Terran_Bunker)) {
    auto sq = rnp::commander()->getSquad(squad_id_);
    if (sq) {
      int no = 0;
      Agentset agents = sq->getMembers();
      for (auto& a : agents) {
        Broodwar->drawTextMap(sx + 2, h - 60 + 15 * no, "%s \x11(%d)", format(a->getUnitType().getName()).c_str(), a->getUnitID());
        no++;
      }
    }
    Broodwar->drawLineMap(sx, h - 1, sx + 100, h - 1, Colors::Purple);
  }
}

void StructureAgent::sendWorkers() {
  //We have constructed a new base. Make some workers move here.
  int noWorkers = rnp::agent_manager()->getNoWorkers();
  int toSend = noWorkers / rnp::agent_manager()->countNoBases();
  int hasSent = 0;

  auto& agents = rnp::agent_manager()->getAgents();
  for (auto& a : agents) {
    if (a->isAlive() && a->isFreeWorker()) {
      Unit worker = a->getUnit();
      WorkerAgent* wa = (WorkerAgent*)a;
      worker->rightClick(unit_->getPosition());
      hasSent++;
    }

    if (hasSent >= toSend) {
      return;
    }
  }
}

bool StructureAgent::canMorphInto(UnitType type) {
  //1. Check canBuild
  if (not Broodwar->canMake(type, unit_)) {
    return false;
  }

  //Zerg morph units sometimes bugs in canMake, so we need to do an extra check
  if (type.getID() == UnitTypes::Zerg_Defiler_Mound.getID() && rnp::agent_manager()->countNoFinishedUnits(UnitTypes::Zerg_Greater_Spire) == 0) {
    return false;
  }

  //2. Check if we have enough resources
  if (not rnp::resources()->hasResources(type)) {
    return false;
  }

  //3. Check if unit is already morphing
  if (unit_->isMorphing()) {
    return false;
  }

  //All clear. Build the unit.
  return true;
}

bool StructureAgent::canEvolveUnit(UnitType type) {
  //1. Check if we need the unit in a squad
  if (not rnp::commander()->is_unit_needed(type)) {
    return false;
  }

  //2. Check canBuild
  if (not Broodwar->canMake(type, unit_)) {
    return false;
  }

  //3. Check if we have enough resources
  if (not rnp::resources()->hasResources(type)) {
    return false;
  }

  //All clear. Build the unit.
  return true;
}
