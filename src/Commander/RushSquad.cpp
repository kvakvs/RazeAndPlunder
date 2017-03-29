#include "RushSquad.h"
#include "Managers/AgentManager.h"
#include "Managers/ExplorationManager.h"
#include "Influencemap/MapManager.h"
#include "Commander/Commander.h"
#include "Glob.h"

using namespace BWAPI;

RushSquad::RushSquad(std::string m_name, int m_priority)
: Squad(SquadType::RUSH, m_name, m_priority) {
  goal_ = Broodwar->self()->getStartLocation();
  current_state_ = State::NOT_SET;
}

bool RushSquad::is_active() const {
  return active_;
}

void RushSquad::defend(TilePosition m_goal) {
  if (not active_) {
    set_goal(m_goal);
  }
}

void RushSquad::attack(TilePosition mGoal) {

}

void RushSquad::assist(TilePosition m_goal) {
  if (not is_under_attack()) {
    current_state_ = State::ASSIST;
    set_goal(m_goal);
  }
}

void RushSquad::prepare_rush_squad() {
  //Check if we need workers in the squad
  auto start_loc = Broodwar->self()->getStartLocation();
  for (size_t i = 0; i < setup_.size(); i++) {
    if (setup_[i].current_count_ < setup_[i].count_ 
      && setup_[i].type_.isWorker()) {
      int todo_count = setup_[i].count_ - setup_[i].current_count_;
      for (int j = 0; j < todo_count; j++) {
        auto w = rnp::agent_manager()->find_closest_free_worker(start_loc);
        if (w) {
          add_member(w->self());
        }
      } // for to do
    } // if worker and want more
  } // for setup

  if (is_full()) {
    active_ = true;
  }

  auto def_spot = rnp::commander()->find_chokepoint();
  if (rnp::is_valid_position(def_spot)) {
    goal_ = def_spot;
  }
}

void RushSquad::tick_active_rush_squad() {
  if (active_priority_ != priority_) {
    priority_ = active_priority_;
  }

  auto target_id = find_worker_target();
  act::for_each_in<BaseAgent>(
    members_,
    [target_id](const BaseAgent* a) {
      msg::unit::attack(a->self(), target_id);
    });

  auto start_loc = Broodwar->self()->getStartLocation();
  TilePosition e_pos = rnp::exploration()->get_random_spotted_building();
  if (rnp::is_valid_position(e_pos)) {
    goal_ = e_pos;
    set_member_goals(goal_);
  }
}

void RushSquad::tick() {
  if (not active_) {
    prepare_rush_squad();
    return;
  }
  if (active_) {
    tick_active_rush_squad();
  }
  Squad::tick();
}

act::ActorId RushSquad::find_worker_target() const {
  try {
    act::ActorId::Set worker_ids;

    // TODO: Maybe let actor cache its type? Then need to track morph events or something
    auto is_worker_predicate = [](const act::ActorId& id) -> bool {
          auto ba = act::whereis<BaseAgent>(id);
          return ba ? ba->get_unit()->getType().isWorker() : false;
        };

    act::for_each_in<BaseAgent>(
      members_,
      [&worker_ids,is_worker_predicate](const BaseAgent* a) {
        constexpr int MAX_RANGE = 12 * 32;
        auto a_pos = a->get_unit()->getTilePosition();
        auto in_r = rnp::actors_in_range(a_pos, MAX_RANGE);
        std::copy_if(in_r.begin(), in_r.end(),
                     std::insert_iterator<act::ActorId::Set>(worker_ids, worker_ids.begin()),
                     is_worker_predicate);
      });
    // Here in worker_ids we have everything in range which also are workers
    return worker_ids.empty() ? act::ActorId() : *(worker_ids.begin());
  }
  catch (std::exception) {
  }

  return act::ActorId();
}

void RushSquad::clear_goal() {
  goal_ = rnp::make_bad_position();
}

bool RushSquad::has_goal() const {
  return rnp::is_valid_position(goal_);
}
