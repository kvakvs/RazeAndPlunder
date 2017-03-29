#include "LurkerAgent.h"
#include "../../Pathfinding/NavigationAgent.h"
#include "../../MainAgents/TargetingAgent.h"

using namespace BWAPI;

bool LurkerAgent::use_abilities() {
  //Check if enemy units are visible
  bool enemyVisible = false;
  for (auto& u : Broodwar->enemy()->getUnits()) {
    if (u->exists()) {
      if (unit_->getDistance(u) <= unit_->getType().sightRange() && not u->getType().isFlyer()) {
        enemyVisible = true;
        break;
      }
    }
  }

  if (enemyVisible && not unit_->isBurrowed()) {
    if (unit_->burrow()) {
      return true;
    }
  }
  if (not enemyVisible && unit_->isBurrowed()) {
    if (unit_->unburrow()) {
      return true;
    }
  }

  return false;
}
