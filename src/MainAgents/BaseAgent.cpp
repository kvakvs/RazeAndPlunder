#include "BaseAgent.h"
#include "../Managers/Constructor.h"
//#include "../Managers/AgentManager.h"
//#include "../Managers/ResourceManager.h"
#include "../Managers/ExplorationManager.h"
#include "Glob.h"

using namespace BWAPI;

BaseAgent::BaseAgent()
    : unit_(), trail_(), goal_(-1, -1)
    , type_(UnitTypes::Unknown)
    , alive_(true)
    , squad_id_(-1)
    , info_update_time_(20)
    , agent_type_("What?")
{
}

BaseAgent::BaseAgent(Unit mUnit)
    : trail_(), unit_(mUnit)
    , type_(mUnit->getType())
    , alive_(true)
    , unit_id_(unit_->getID())
    , squad_id_(-1)
    , goal_(-1, -1)
    , agent_type_("BaseAgent")
{
}

BaseAgent::~BaseAgent() {

}

int BaseAgent::getLastOrderFrame() {
  return last_order_frame_;
}

std::string BaseAgent::getTypeName() {
  return agent_type_;
}

void BaseAgent::printInfo() {

}

int BaseAgent::getUnitID() {
  return unit_id_;
}

std::string BaseAgent::format(std::string str) {
  std::string res = str;

  int i = str.find("_");
  if (i >= 0) {
    res = str.substr(i + 1, str.length());
  }

  if (res == "Siege Tank Tank Mode") res = "Siege Tank";
  if (res == "Siege Tank Siege Mode") res = "Siege Tank (sieged)";

  return res;
}

UnitType BaseAgent::getUnitType() {
  return type_;
}

Unit BaseAgent::getUnit() {
  return unit_;
}

bool BaseAgent::matches(Unit mUnit) {
  if (mUnit->getID() == unit_id_) {
    return true;
  }
  return false;
}

bool BaseAgent::isOfType(UnitType type) {
  if (unit_->getType().getID() == type.getID()) {
    return true;
  }
  return false;
}

bool BaseAgent::isOfType(UnitType mType, UnitType toCheckType) {
  if (mType.getID() == toCheckType.getID()) {
    return true;
  }
  return false;
}

bool BaseAgent::isBuilding() {
  if (unit_->getType().isBuilding()) {
    return true;
  }
  return false;
}

bool BaseAgent::isWorker() {
  if (unit_->getType().isWorker()) {
    return true;
  }
  return false;
}

bool BaseAgent::isUnit() {
  if (unit_->getType().isBuilding() || unit_->getType().isWorker() || unit_->getType().isAddon()) {
    return false;
  }
  return true;
}

bool BaseAgent::isUnderAttack() {
  if (unit_ == nullptr) return false;
  if (not unit_->exists()) return false;

  if (unit_->isAttacking()) return true;
  if (unit_->isStartingAttack()) return true;

  double r = unit_->getType().seekRange();
  if (unit_->getType().sightRange() > r) {
    r = unit_->getType().sightRange();
  }

  for (auto& u : Broodwar->enemy()->getUnits()) {
    double dist = unit_->getPosition().getDistance(u->getPosition());
    if (dist <= r) {
      return true;
    }
  }

  return false;
}

void BaseAgent::destroyed() {
  alive_ = false;
}

bool BaseAgent::isAlive() {
  if (not unit_->exists()) {
    return false;
  }
  return alive_;
}

bool BaseAgent::isDamaged() {
  if (unit_->isBeingConstructed()) return false;
  if (unit_->getRemainingBuildTime() > 0) return false;

  if (unit_->getHitPoints() < unit_->getType().maxHitPoints()) {
    return true;
  }
  return false;
}

bool BaseAgent::isDetectorWithinRange(TilePosition pos, int range) {
  for (auto& u : Broodwar->enemy()->getUnits()) {
    if (u->getType().isDetector()) {
      double dist = u->getDistance(Position(pos));
      if (dist <= range) {
        return true;
      }
    }
  }
  return false;
}

void BaseAgent::setSquadID(int id) {
  squad_id_ = id;
}

int BaseAgent::getSquadID() {
  return squad_id_;
}

bool BaseAgent::enemyUnitsVisible() {
  double r = unit_->getType().sightRange();

  for (auto& u : Broodwar->enemy()->getUnits()) {
    double dist = unit_->getPosition().getDistance(u->getPosition());
    if (dist <= r) {
      return true;
    }
  }
  return false;
}

void BaseAgent::setGoal(TilePosition goal) {
  if (unit_->getType().isFlyer() || unit_->getType().isFlyingBuilding()) {
    //Flyers, can always move to goals.
    this->goal_ = goal;
  }
  else {
    //Ground units, check if we can reach goal.
    if (rnp::exploration()->can_reach(this, goal)) {
      this->goal_ = goal;
    }
  }
}

void BaseAgent::clearGoal() {
  goal_ = TilePosition(-1, -1);
}

TilePosition BaseAgent::getGoal() {
  return goal_;
}

void BaseAgent::addTrailPosition(WalkPosition wt) {
  //Check if position already is in trail
  if (trail_.size() > 0) {
    WalkPosition lwt = trail_.at(trail_.size() - 1);
    if (lwt.x == wt.x && lwt.y == wt.y) return;
  }

  trail_.push_back(wt);
  if (trail_.size() > 20) {
    trail_.erase(trail_.begin());
  }
}

std::vector<WalkPosition> BaseAgent::getTrail() {
  return trail_;
}
