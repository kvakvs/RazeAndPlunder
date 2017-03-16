#include "TransportAgent.h"
#include "Pathfinding/NavigationAgent.h"
#include "Commander/Commander.h"
#include "Commander/Squad.h"
#include "Glob.h"

using namespace BWAPI;

TransportAgent::TransportAgent(Unit mUnit) {
  unit_ = mUnit;
  type_ = unit_->getType();
  maxLoad = type_.spaceProvided();
  currentLoad = 0;
  unit_id_ = unit_->getID();
  agent_type_ = "TransportAgent";

  goal_ = TilePosition(-1, -1);
}

int TransportAgent::getCurrentLoad() {
  auto sq = rnp::commander()->getSquad(squad_id_);
  if (sq) {
    int load = 0;
    Agentset agents = sq->getMembers();
    for (auto& a : agents) {
      if (a->isAlive()) {
        if (a->getUnit()->isLoaded()) {
          if (a->getUnit()->getTransport()->getID() == unit_->getID()) {
            load += a->getUnitType().spaceRequired();
          }
        }
      }
    }
    currentLoad = load;
  }

  return currentLoad;
}

bool TransportAgent::isValidLoadUnit(BaseAgent* a) {
  if (a->getUnitType().isFlyer()) return false;
  if (a->getUnit()->isLoaded()) return false;
  if (a->getUnit()->isBeingConstructed()) return false;
  if (a->getUnitID() == unit_->getID()) return false;
  return true;
}

BaseAgent* TransportAgent::findUnitToLoad(int spaceLimit) {
  BaseAgent* agent = nullptr;
  double bestDist = 100000;

  auto sq = rnp::commander()->getSquad(squad_id_);
  if (sq) {
    Agentset agents = sq->getMembers();
    for (auto& a : agents) {
      if (isValidLoadUnit(a)) {
        double cDist = unit_->getPosition().getDistance(a->getUnit()->getPosition());
        if (cDist < bestDist) {
          bestDist = cDist;
          agent = a;
        }
      }
    }
  }

  return agent;
}

void TransportAgent::computeActions() {
  if (unit_->isBeingConstructed()) return;

  int currentLoad = getCurrentLoad();
  int eCnt = enemyUnitsVisible();

  if (eCnt == 0) {
    if (currentLoad < maxLoad) {
      BaseAgent* toLoad = findUnitToLoad(maxLoad - currentLoad);
      if (toLoad != nullptr) {
        unit_->load(toLoad->getUnit());
        return;
      }
    }
  }
  else {
    if (currentLoad > 0) {
      TilePosition t = unit_->getTilePosition();
      unit_->unloadAll();
      return;
    }
  }

  rnp::navigation()->computeMove(this, goal_);
}
