#include "ExplorationSquad.h"
#include "UnitAgents/UnitAgent.h"
#include "Managers/AgentManager.h"
#include "Managers/ExplorationManager.h"
#include <iso646.h>
#include "Glob.h"

using namespace BWAPI;

ExplorationSquad::ExplorationSquad(std::string mName, int mPriority)
: Squad(SquadType::EXPLORER, mName, mPriority), delay_respawn_()
{
  goal_ = Broodwar->self()->getStartLocation();
  current_state_ = State::NOT_SET;
  delay_respawn_.start(0);
}

void ExplorationSquad::try_fill_the_squad() {
  auto start_loc = Broodwar->self()->getStartLocation();
  auto agent_manager = rnp::agent_manager();

  //Check if we need workers in the squad
  for (auto& setup : setup_) {
    if (setup.current_count_ < setup.wanted_count_ 
      && setup.type_.isWorker()) {
      int todo_count = setup.wanted_count_ - setup.current_count_;
      for (int j = 0; j < todo_count; j++) {
        auto w = agent_manager->find_closest_free_worker(start_loc);
        if (w) {
          add_member(w->self());
        }
      }
    }
  }
}

void ExplorationSquad::tick_active_explo_squad() {
  if (active_priority_ != priority_) {
    priority_ = active_priority_;
  }

  if (m_next_explore_.value_up_to_date(rnp::seconds(10))) {
    // do nothing if the value is too fresh
    return;
  }

  auto n_goal = rnp::exploration()->get_next_to_explore(this);
  if (rnp::is_valid_position(n_goal)) {
    this->goal_ = n_goal;
    set_member_goals(goal_);
  }

  m_next_explore_.update(goal_);
}

void ExplorationSquad::tick() {
  if (not active_) { // && delay_respawn_.is_ready()) {
    try_fill_the_squad();
    active_ |= is_full();
  }

  if (not members_.empty() && not active_) {
    //Activate as soon as 1 unit has been built
    active_ = true;
  }

  //All units dead, go back to inactive
  if (members_.empty() && active_) {
    active_ = false;
    //delay_respawn_.start(rnp::seconds(20)); // no instant borrow worker, wait
    return;
  }

  if (active_) {
    tick_active_explo_squad();
  }

  Squad::tick();
}

void ExplorationSquad::clear_goal() {

}

bool ExplorationSquad::has_goal() const {
  return rnp::is_valid_position(goal_);
}
