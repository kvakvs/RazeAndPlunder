#include "ExplorationSquad.h"
#include "UnitAgents/UnitAgent.h"
#include "Managers/AgentManager.h"
#include "Managers/ExplorationManager.h"
#include <iso646.h>
#include "Glob.h"

using namespace BWAPI;

ExplorationSquad::ExplorationSquad(int mId, std::string mName, int mPriority)
: Squad(mId, SquadType::EXPLORER, mName, mPriority) {
  goal_ = Broodwar->self()->getStartLocation();
  current_state_ = State::NOT_SET;
}

bool ExplorationSquad::isActive() {
  return active_;
}

void ExplorationSquad::defend(TilePosition mGoal) {

}

void ExplorationSquad::attack(TilePosition mGoal) {

}

void ExplorationSquad::assist(TilePosition mGoal) {

}

void ExplorationSquad::computeActions() {
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
  }

  //First, remove dead agents
  removeDestroyed();

  if (agents_.size() > 0 && !active_) {
    //Activate as soon as a unit has been built.
    active_ = true;
  }

  //All units dead, go back to inactive
  if ((int)agents_.size() == 0) {
    active_ = false;
    return;
  }

  if (active_) {
    if (active_priority_ != priority_) {
      priority_ = active_priority_;
    }

    TilePosition nGoal = rnp::exploration()->getNextToExplore(this);
    if (nGoal.x >= 0) {
      this->goal_ = nGoal;
      setMemberGoals(goal_);
    }
  }
}

void ExplorationSquad::clearGoal() {

}

TilePosition ExplorationSquad::getGoal() {
  return goal_;
}

bool ExplorationSquad::hasGoal() {
  if (goal_.x < 0 || goal_.y < 0) {
    return false;
  }
  return true;
}
