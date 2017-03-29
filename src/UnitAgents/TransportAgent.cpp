#include "TransportAgent.h"
#include "Pathfinding/NavigationAgent.h"
#include "Commander/Commander.h"
#include "Commander/Squad.h"
#include "Glob.h"
#include "RnpUtil.h"

using namespace BWAPI;

TransportAgent::TransportAgent(Unit mUnit) {
  unit_ = mUnit;
  type_ = unit_->getType();
  max_load_ = type_.spaceProvided();
  current_load_ = 0;
  unit_id_ = unit_->getID();
  agent_type_ = "TransportAgent";

  goal_ = rnp::make_bad_position();
}

int TransportAgent::get_current_load() {
  auto sq = rnp::commander()->get_squad(squad_id_);
  if (sq) {
    int load = 0;

    act::for_each_in<BaseAgent>(
      sq->get_members(),
      [this,&load](const BaseAgent* a) {
        auto aunit = a->get_unit();
        if (a->is_alive()
          && aunit->isLoaded()
          && aunit->getTransport()->getID() == unit_->getID()) 
        {
          load += a->unit_type().spaceRequired();
        }
      });

    current_load_ = load;
  }

  return current_load_;
}

bool TransportAgent::is_valid_load_unit(const BaseAgent* a) const {
  if (a->unit_type().isFlyer()) return false;
  if (a->get_unit()->isLoaded()) return false;
  if (a->get_unit()->isBeingConstructed()) return false;
  if (a->get_unit_id() == unit_->getID()) return false;
  return true;
}

const BaseAgent* TransportAgent::find_unit_to_load(int spaceLimit) {
  const BaseAgent* agent = nullptr;
  float best_dist = 1e+12f;

  auto sq = rnp::commander()->get_squad(squad_id_);
  if (sq) {
    act::for_each_in<BaseAgent>(
      sq->get_members(),
      [this,&agent,&best_dist](const BaseAgent* a) {
        auto a_pos = a->get_unit()->getPosition();
        float c_dist = rnp::distance(a_pos, unit_->getPosition());
        if (c_dist < best_dist) {
          best_dist = c_dist;
          agent = a;
        }
      });
  } // if sq

  return agent;
}

void TransportAgent::tick() {
  if (unit_->isBeingConstructed()) return;

  int current_load = get_current_load();
  int enemy_count = any_enemy_units_visible();

  if (enemy_count == 0) {
    if (current_load < max_load_) {
      auto to_load = find_unit_to_load(max_load_ - current_load);
      if (to_load != nullptr) {
        unit_->load(to_load->get_unit());
        return;
      }
    }
  }
  else {
    if (current_load > 0) {
      TilePosition t(unit_->getTilePosition());
      unit_->unloadAll();
      return;
    }
  }

  rnp::navigation()->compute_move(this, goal_);
}
