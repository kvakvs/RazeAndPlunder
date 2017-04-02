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
    fsm_set_state(SquadState::ASSIST);
    set_goal(m_goal);
  }
}

void RushSquad::tick_inactive() {
  fill_with_free_workers();

  if (is_full()) {
    active_ = true;
    return;
  }

  auto def_spot = rnp::commander()->find_chokepoint();
  if (rnp::is_valid_position(def_spot)) {
    goal_ = def_spot;
  }
}

void RushSquad::tick_active() {
  if (active_priority_ != priority_) {
    priority_ = active_priority_;
  }

  auto target_id = find_worker_target();
  act::for_each_in<BaseAgent>(
    members_,
    [target_id](const BaseAgent* a) {
    msg::unit::attack(a->self(), target_id);
  });

  // auto start_loc = Broodwar->self()->getStartLocation();
  TilePosition e_pos = rnp::exploration()->get_random_spotted_building();
  if (rnp::is_valid_position(e_pos)) {
    goal_ = e_pos;
    set_member_goals(goal_);
  }
}

act::ActorId RushSquad::find_worker_target() const {
  try {
    act::ActorId::Set worker_ids;

    auto is_worker_predicate = [](const act::ActorId& id) -> bool {
          auto ba = act::whereis<BaseAgent>(id);
          return ba ? ba->get_unit()->getType().isWorker() : false;
        };

    act::for_each_in<BaseAgent>(
      members_,
      [&worker_ids,is_worker_predicate](const BaseAgent* a) {
        constexpr int MAX_RANGE = 12 * TILEPOSITION_SCALE;
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

bool RushSquad::has_goal() const {
  return rnp::is_valid_position(goal_);
}
