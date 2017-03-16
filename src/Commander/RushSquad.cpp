#include "RushSquad.h"
#include "../Managers/AgentManager.h"
#include "../Managers/ExplorationManager.h"
#include "Commander/Commander.h"
#include "Glob.h"

using namespace BWAPI;

RushSquad::RushSquad(int mId, std::string mName, int mPriority)
: Squad(mId, SquadType::RUSH, mName, mPriority) {
  goal_ = Broodwar->self()->getStartLocation();
  current_state_ = State::NOT_SET;
}

bool RushSquad::isActive() {
  return active_;
}

void RushSquad::defend(TilePosition mGoal) {
  if (not active_) {
    setGoal(mGoal);
  }
}

void RushSquad::attack(TilePosition mGoal) {

}

void RushSquad::assist(TilePosition mGoal) {
  if (not isUnderAttack()) {
    current_state_ = State::ASSIST;
    setGoal(mGoal);
  }
}

void RushSquad::computeActions() {
  if (not active_) {
    //Check if we need workers in the squad
    for (int i = 0; i < (int)setup_.size(); i++) {
      if (setup_.at(i).current < setup_.at(i).no && setup_.at(i).type.isWorker()) {
        int no = setup_.at(i).no - setup_.at(i).current;
        for (int j = 0; j < no; j++) {
          BaseAgent* w = rnp::agent_manager()->findClosestFreeWorker(Broodwar->self()->getStartLocation());
          if (w != nullptr) addMember(w);
        }
      }
    }

    if (isFull()) {
      active_ = true;
    }

    TilePosition defSpot = rnp::commander()->find_chokepoint();
    if (rnp::is_valid_position(defSpot)) {
      goal_ = defSpot;
    }
    return;
  }

  //First, remove dead agents
  removeDestroyed();

  if (active_) {
    if (active_priority_ != priority_) {
      priority_ = active_priority_;
    }

    Unit target = findWorkerTarget();
    if (target != nullptr) {
      for (auto& a : agents_) {
        if (a->isAlive()) {
          a->getUnit()->attack(target);
        }
      }
    }

    TilePosition ePos = rnp::exploration()->get_closest_spotted_building(Broodwar->self()->getStartLocation());
    if (rnp::is_valid_position(ePos)) {
      goal_ = ePos;
      setMemberGoals(goal_);
    }
  }
}

Unit RushSquad::findWorkerTarget() {
  try {
    double maxRange = 12 * 32;

    for (auto& a : agents_) {
      for (auto& u : Broodwar->enemy()->getUnits()) {
        if (u->exists()) {
          if (u->getType().isWorker()) {
            double dist = a->getUnit()->getDistance(u);
            if (dist <= maxRange) {
              return u;
            }
          }
        }
      }
    }
  }
  catch (std::exception) {

  }

  return nullptr;
}

void RushSquad::clearGoal() {
  goal_ = TilePosition(-1, -1);
}

TilePosition RushSquad::getGoal() {
  return goal_;
}

bool RushSquad::hasGoal() {
  if (goal_.x < 0 || goal_.y < 0) {
    return false;
  }
  return true;
}
