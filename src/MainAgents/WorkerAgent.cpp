#include "WorkerAgent.h"
#include "../Managers/AgentManager.h"
#include "Pathfinding/NavigationAgent.h"
#include "../Managers/BuildingPlacer.h"
#include "../Managers/Constructor.h"
#include "Commander/Commander.h"
#include "../Managers/ResourceManager.h"
#include "Glob.h"
#include "RnpUtil.h"

using namespace BWAPI;

WorkerAgent::WorkerAgent(Unit mUnit) {
  unit_ = mUnit;
  type_ = unit_->getType();
  unit_id_ = unit_->getID();
  setState(GATHER_MINERALS);
  startBuildFrame = 0;
  startSpot = TilePosition(-1, -1);
  agent_type_ = "WorkerAgent";

  toBuild = UnitTypes::None;
}

void WorkerAgent::destroyed() {
  if (currentState == MOVE_TO_SPOT || currentState == CONSTRUCT || currentState == FIND_BUILDSPOT) {
    if (not Constructor::isZerg()) {
      rnp::constructor()->handleWorkerDestroyed(toBuild, unit_id_);
      rnp::building_placer()->clear_temp(toBuild, buildSpot);
      setState(GATHER_MINERALS);
    }
  }
}

void WorkerAgent::printInfo() {
  int e = Broodwar->getFrameCount() - info_update_frame_;
  if (e >= info_update_time_ || (sx_ == 0 && sy_ == 0)) {
    info_update_frame_ = Broodwar->getFrameCount();
    sx_ = unit_->getPosition().x;
    sy_ = unit_->getPosition().y;
  }

  Broodwar->drawBoxMap(sx_ - 2, sy_, sx_ + 152, sy_ + 90, Colors::Black, true);
  Broodwar->drawTextMap(sx_ + 4, sy_, "\x03%s", unit_->getType().getName().c_str());
  Broodwar->drawLineMap(sx_, sy_ + 14, sx_ + 150, sy_ + 14, Colors::Blue);

  Broodwar->drawTextMap(sx_ + 2, sy_ + 15, "Id: \x11%d", unit_id_);
  Broodwar->drawTextMap(sx_ + 2, sy_ + 30, "Position: \x11(%d,%d)", unit_->getTilePosition().x, unit_->getTilePosition().y);
  Broodwar->drawTextMap(sx_ + 2, sy_ + 45, "Goal: \x11(%d,%d)", goal_.x, goal_.y);
  if (squad_id_ == -1) Broodwar->drawTextMap(sx_ + 2, sy_ + 60, "Squad: \x15None");
  else Broodwar->drawTextMap(sx_ + 2, sy_ + 60, "Squad: \x11%d", squad_id_);
  Broodwar->drawTextMap(sx_ + 2, sy_ + 75, "State: \x11%s", getStateAsText().c_str());

  Broodwar->drawLineMap(sx_, sy_ + 89, sx_ + 150, sy_ + 89, Colors::Blue);
}

void WorkerAgent::debug_showGoal() {
  if (not isAlive()) return;
  if (not unit_->isCompleted()) return;

  if (currentState == GATHER_MINERALS || currentState == GATHER_GAS) {
    Unit target = unit_->getTarget();
    if (target != nullptr) {
      Position a = unit_->getPosition();
      Position b = target->getPosition();
      Broodwar->drawLineMap(a.x, a.y, b.x, b.y, Colors::Teal);
    }
  }

  if (currentState == MOVE_TO_SPOT || currentState == CONSTRUCT) {
    if (buildSpot.x > 0) {
      int w = toBuild.tileWidth() * 32;
      int h = toBuild.tileHeight() * 32;

      Position a = unit_->getPosition();
      Position b = Position(buildSpot.x * 32 + w / 2, buildSpot.y * 32 + h / 2);
      Broodwar->drawLineMap(a.x, a.y, b.x, b.y, Colors::Teal);

      Broodwar->drawBoxMap(buildSpot.x * 32, buildSpot.y * 32, buildSpot.x * 32 + w, buildSpot.y * 32 + h, Colors::Blue, false);
    }
  }

  if (unit_->isRepairing()) {
    Unit targ = unit_->getOrderTarget();
    if (targ != nullptr) {
      Position a = unit_->getPosition();
      Position b = targ->getPosition();
      Broodwar->drawLineMap(a.x, a.y, b.x, b.y, Colors::Green);

      Broodwar->drawTextMap(unit_->getPosition().x, unit_->getPosition().y, "Repairing %s", targ->getType().getName().c_str());
    }
  }

  if (unit_->isConstructing()) {
    Unit targ = unit_->getOrderTarget();
    if (targ != nullptr) {
      Position a = unit_->getPosition();
      Position b = targ->getPosition();
      Broodwar->drawLineMap(a.x, a.y, b.x, b.y, Colors::Green);

      Broodwar->drawTextMap(unit_->getPosition().x, unit_->getPosition().y, "Constructing %s", targ->getType().getName().c_str());
    }
  }
}

bool WorkerAgent::checkRepair() {
  if (unit_->getType().getID() != UnitTypes::Terran_SCV.getID()) return false;
  if (unit_->isRepairing()) return true;

  //Find closest unit that needs repairing
  BaseAgent* toRepair = nullptr;
  double bestDist = 10000;
  auto& agents = rnp::agent_manager()->getAgents();
  for (auto& a : agents) {
    if (a->isAlive() && a->isDamaged() && a->getUnitType().isMechanical() && a->getUnitID() != unit_id_) {
      double cDist = a->getUnit()->getPosition().getDistance(unit_->getPosition());
      if (cDist < bestDist) {
        bestDist = cDist;
        toRepair = a;
      }
    }
  }

  //Repair it
  if (toRepair != nullptr) {
    unit_->repair(toRepair->getUnit());
    return true;
  }

  return false;
}

void WorkerAgent::computeSquadWorkerActions() {
  //Repairing
  if (checkRepair()) return;

  //No repairing. Gather minerals
  auto sq = rnp::commander()->getSquad(squad_id_);
  if (sq) {
    //If squad is not ative, let the worker gather
    //minerals while not doing any repairs
    if (not sq->isActive()) {
      if (unit_->isIdle()) {
        Unit mineral = rnp::building_placer()->find_closest_mineral(unit_->getTilePosition());
        if (mineral != nullptr) {
          unit_->rightClick(mineral);
          return;
        }
      }
    }
    else {
      rnp::navigation()->computeMove(this, goal_);
      return;
    }
  }
}

bool WorkerAgent::isFreeWorker() {
  if (currentState != GATHER_MINERALS) return false;
  if (toBuild.getID() != UnitTypes::None.getID()) return false;
  if (unit_->isConstructing()) return false;
  Unit b = unit_->getTarget();
  if (b != nullptr) if (b->isBeingConstructed()) return false;
  if (unit_->isRepairing()) return false;
  if (squad_id_ != -1) return false;

  return true;
}


void WorkerAgent::computeActions() {
  //To prevent order spamming
  last_order_frame_ = Broodwar->getFrameCount();

  if (squad_id_ != -1) {
    computeSquadWorkerActions();
    return;
  }
  //Check if workers are too far away from a base when attacking
  if (currentState == ATTACKING) {
    if (unit_->getTarget() != nullptr) {
      BaseAgent* base = rnp::agent_manager()->getClosestBase(unit_->getTilePosition());
      if (base != nullptr) {
        double dist = base->getUnit()->getTilePosition().getDistance(unit_->getTilePosition());
        if (dist > 25) {
          //Stop attacking. Return home
          unit_->stop();
          unit_->rightClick(base->getUnit());
          setState(GATHER_MINERALS);
          return;
        }
      }
    }
    else {
      //No target, return to gather minerals
      setState(GATHER_MINERALS);
      return;
    }
  }

  if (currentState == GATHER_GAS) {
    if (unit_->isIdle()) {
      //Not gathering gas. Reset.
      setState(GATHER_MINERALS);
    }
  }

  if (currentState == REPAIRING) {
    Unit target = unit_->getTarget();
    bool cont = true;
    if (target == nullptr) cont = false;
    if (target != nullptr && target->getHitPoints() >= target->getInitialHitPoints()) cont = false;

    if (not cont) {
      reset();
    }
    return;
  }

  if (currentState == GATHER_MINERALS) {
    if (unit_->isIdle()) {
      Unit mineral = rnp::building_placer()->find_closest_mineral(unit_->getTilePosition());
      if (mineral != nullptr) {
        unit_->rightClick(mineral);
      }
    }
  }

  if (currentState == FIND_BUILDSPOT) {
    if (not rnp::is_valid_position(buildSpot)) {
      buildSpot = rnp::building_placer()->find_build_spot(toBuild);
    }
    if (buildSpot.x >= 0) {
      setState(MOVE_TO_SPOT);
      startBuildFrame = Broodwar->getFrameCount();
      if (toBuild.isResourceDepot()) {
        rnp::commander()->update_squad_goals();
      }
    }
  }

  if (currentState == MOVE_TO_SPOT) {
    if (not buildSpotExplored()) {
      Position toMove = Position(buildSpot.x * 32 + 16, buildSpot.y * 32 + 16);
      if (toBuild.isRefinery()) toMove = Position(buildSpot);
      unit_->rightClick(toMove);
    }

    if (buildSpotExplored() && not unit_->isConstructing()) {
      bool ok = unit_->build(toBuild, buildSpot);
      if (not ok) {
        rnp::building_placer()->mark_position_blocked(buildSpot);
        rnp::building_placer()->clear_temp(toBuild, buildSpot);
        //Cant build at selected spot, get a new one.
        setState(FIND_BUILDSPOT);
      }
    }

    if (unit_->isConstructing()) {
      setState(CONSTRUCT);
      startSpot = TilePosition(-1, -1);
    }
  }

  if (currentState == CONSTRUCT) {
    if (isBuilt()) {
      //Build finished.
      BaseAgent* agent = rnp::agent_manager()->getClosestBase(unit_->getTilePosition());
      if (agent != nullptr) {
        unit_->rightClick(agent->getUnit()->getPosition());
      }
      setState(GATHER_MINERALS);
    }
  }
}

bool WorkerAgent::isBuilt() {
  if (unit_->isConstructing()) return false;

  Unit b = unit_->getTarget();
  if (b != nullptr) if (b->isBeingConstructed()) return false;

  return true;
}

bool WorkerAgent::buildSpotExplored() {
  int sightDist = 64;
  if (toBuild.isRefinery()) {
    sightDist = 160; //5 tiles
  }

  double dist = unit_->getPosition().getDistance(Position(buildSpot));
  if (dist > sightDist) {
    return false;
  }
  return true;
}

int WorkerAgent::getState() {
  return currentState;
}

void WorkerAgent::setState(int state) {
  currentState = state;

  if (state == GATHER_MINERALS) {
    startSpot = TilePosition(-1, -1);
    buildSpot = TilePosition(-1, -1);
    toBuild = UnitTypes::None;
  }
}

bool WorkerAgent::canBuild(UnitType type) {
  if (unit_->isIdle()) {
    return true;
  }
  if (unit_->isGatheringMinerals()) {
    return true;
  }
  return false;
}

bool WorkerAgent::assignToBuild(UnitType type) {
  toBuild = type;
  buildSpot = rnp::building_placer()->find_build_spot(toBuild);
  if (buildSpot.x >= 0) {
    rnp::resources()->lockResources(toBuild);
    rnp::building_placer()->fill_temp(toBuild, buildSpot);
    setState(FIND_BUILDSPOT);
    return true;
  }
  else {
    startSpot = TilePosition(-1, -1);
    return false;
  }
}

void WorkerAgent::reset() {
  if (currentState == MOVE_TO_SPOT) {
    //The buildSpot is probably not reachable. Block it.	
    rnp::building_placer()->mark_position_blocked(buildSpot);
    rnp::building_placer()->clear_temp(toBuild, buildSpot);
  }

  if (unit_->isConstructing()) {
    unit_->cancelConstruction();
    rnp::building_placer()->clear_temp(toBuild, buildSpot);
  }

  if (unit_->isRepairing()) {
    unit_->cancelConstruction();
  }

  setState(GATHER_MINERALS);
  unit_->stop();
  BaseAgent* base = rnp::agent_manager()->getClosestBase(unit_->getTilePosition());
  if (base != nullptr) {
    unit_->rightClick(base->getUnit()->getPosition());
  }
}

bool WorkerAgent::isConstructing(UnitType type) {
  if (currentState == FIND_BUILDSPOT || currentState == MOVE_TO_SPOT || currentState == CONSTRUCT) {
    if (toBuild.getID() == type.getID()) {
      return true;
    }
  }
  return false;
}

// Returns the state of the agent as text. Good for printouts. 
std::string WorkerAgent::getStateAsText() {
  std::string strReturn = "";
  switch (currentState) {
  case GATHER_MINERALS:
    strReturn = "GATHER_MINERALS";
    break;
  case GATHER_GAS:
    strReturn = "GATHER_GAS";
    break;
  case FIND_BUILDSPOT:
    strReturn = "FIND_BUILDSPOT";
    break;
  case MOVE_TO_SPOT:
    strReturn = "MOVE_TO_SPOT";
    break;
  case CONSTRUCT:
    strReturn = "CONSTRUCT";
    break;
  case REPAIRING:
    strReturn = "REPAIRING";
    break;
  };
  return strReturn;
}

bool WorkerAgent::assignToFinishBuild(Unit building) {
  if (isFreeWorker()) {
    setState(REPAIRING);
    unit_->rightClick(building);
    return true;
  }
  return false;
}

bool WorkerAgent::assignToRepair(Unit building) {
  if (isFreeWorker()) {
    setState(REPAIRING);
    bool ok = unit_->repair(building);
    unit_->rightClick(building);
    return true;
  }
  return false;
}
