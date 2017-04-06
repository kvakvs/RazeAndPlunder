#include "AgentManager.h"
#include "MainAgents/AgentFactory.h"
#include "BuildingPlacer.h"
#include "Commander/Commander.h"
#include "Constructor.h"
#include "ResourceManager.h"
#include "MainAgents/WorkerAgent.h"
#include "Utils/Profiler.h"
#include "Glob.h"
#include "RnpConst.h"

#include <BWAPI.h>

using namespace BWAPI;

AgentManager::AgentManager() {
  last_call_frame_ = Broodwar->getFrameCount();
}

AgentManager::~AgentManager() {
}

const BaseAgent* AgentManager::get_agent(int unit_id) const {
  // Construct an actor ID from its unit ID and use it as a key
  return act::whereis<BaseAgent>(rnp::unit_actor_key(unit_id));
}

const BaseAgent* AgentManager::get_closest_base(const TilePosition& pos) const {
  const BaseAgent* agent = nullptr;
  float best_dist = LIKE_VERY_FAR;

  // TODO: this can maybe be optimized? cache depots?
  act::for_each_actor<BaseAgent>(
    [&pos,&agent,&best_dist](const BaseAgent* a) {
      if (a->unit_type().isResourceDepot()) {
        float dist = rnp::distance(pos, a->get_unit()->getTilePosition());
        if (dist < best_dist) {
          best_dist = dist;
          agent = a;
        }
      }
    }
  );
  return agent;
}

const BaseAgent* AgentManager::get_closest_agent(TilePosition pos, UnitType type) const {
  const BaseAgent* agent = nullptr;
  float best_dist = LIKE_VERY_FAR;

  // TODO: optimize? sensitivity radius and use grid?
  act::for_each_actor<BaseAgent>(
    [&pos,&type,&best_dist,&agent](const BaseAgent* a) {
      if (a->is_of_type(type)) {
        float dist = rnp::distance(pos, a->get_unit()->getTilePosition());
        if (dist < best_dist) {
          best_dist = dist;
          agent = a;
        }
      }
    });
  return agent;
}

void AgentManager::add_agent(Unit unit) const {
  //Special case: Dont add some Zerg things as agents.
  if ( rnp::same_type(unit, UnitTypes::Zerg_Larva)
    || rnp::same_type(unit, UnitTypes::Zerg_Egg)
    || rnp::same_type(unit, UnitTypes::Zerg_Cocoon)
    || rnp::same_type(unit, UnitTypes::Zerg_Lurker_Egg)) {
    return;
  }

  bool found = false;
  act::for_each_actor<BaseAgent>(
    [unit,&found](const BaseAgent* a) {
      if (a->matches(unit)) {
        found = true;
        return;
      }
    });

  if (not found) {
    auto new_agent_id = AgentFactory::get_instance()->create_agent(unit);
    auto new_agent = act::whereis<BaseAgent>(new_agent_id);
//    std::cout << "agentmgr::add_agent: created agent for " 
//      << unit->getType() << std::endl;

    if (new_agent->is_building()) {
      rnp::building_placer()->add_constructed_building(unit);
      UnitType ut = unit->getType();
      Constructor::modify([=](Constructor* c) {
        c->unlock(ut);
        rnp::resources()->unlock_resources(ut);
      });
    }
    else {
      msg::commander::unit_created(new_agent_id);
    }
  }
}

void AgentManager::on_unit_destroyed(Unit dead) {
  auto unit_id = dead->getID();
  auto actor_id = rnp::unit_actor_key(unit_id);
  auto a = act::whereis<BaseAgent>(actor_id);

  if (a) {
    if (a->is_building()) {
      rnp::building_placer()->on_building_destroyed(dead);
    }

    act::send_message<msg::unit::Destroyed>(actor_id);
    msg::commander::unit_destroyed(actor_id);

    //Special case: If a bunker is destroyed, we need to remove
    //the bunker squad.
    if (dead->getType().getID() == UnitTypes::Terran_Bunker.getID()) {
      auto& squad_id = a->get_squad_id();
      msg::commander::remove_squad(squad_id);
    }

    act::signal(actor_id, act::Signal::Kill);
    //dead_agents_[unit_id] = std::move(iter->second);
    //agents_.erase(iter);
  }
}

void AgentManager::on_drone_morphed(Unit unit) {
//  act::for_each_alive(
//    [this,unit](const BaseAgent* a) {
//      if (a->matches(unit)) {
//        agents_.erase(unit->getID());
//        add_agent(unit);
//        return;
//      }
//    });
  //No match found. Add it anyway.
  if (unit->exists()) {
    add_agent(unit);
  }
}

//void AgentManager::cleanup() {
//  for_each_alive(
//    [this](BaseAgent* a) {
//      if (not a->is_alive()) {
//        // If dead, move to the dead
//        auto unit_id = a->get_unit_id();
//        auto iter = agents_.find(unit_id);
//        if (iter != agents_.end()) {
//          dead_agents_[unit_id] = std::move(iter->second);
//          agents_.erase(iter);
//        }
//      }
//    });
//}

void AgentManager::tick() {
//  rnp::profiler()->start("OnFrame_AgentManager");
//
//  //Start time
//  LARGE_INTEGER li;
//  QueryPerformanceFrequency(&li);
//  auto pc_freq = double(li.QuadPart) / 1000.0;
//  QueryPerformanceCounter(&li);
//  auto counter_start = li.QuadPart;
//  auto now = Broodwar->getFrameCount();
//
//  act::for_each_actor<BaseAgent>(
//    [this,now,counter_start,pc_freq](const BaseAgent* a) {
//      const auto FRAME_LIMIT = 83.0;
//      if (now - a->get_last_order_frame() > 30) {
//        //a->on_frame();
//        // Thinking is done with signal, to avoid growing message queue by
//        // sending repeated think commands, we set a flag to true instead
//        //act::signal(a->get_ac_id(), act::Signal::PeriodicThink);
//        const_cast<BaseAgent*>(a)->ac_tick();
//      }
//
////      LARGE_INTEGER le;
////      QueryPerformanceCounter(&le);
////      auto elapsed = (le.QuadPart - counter_start) / pc_freq;
////      if (elapsed >= FRAME_LIMIT) {
////        std::cout << BOT_PREFIX_DEBUG "Frame limit exceeded " << FRAME_LIMIT << " ms\n";
////        return; // note this return must break on_frame, not the inner lambda!
////      }
//    });
//  rnp::profiler()->end("OnFrame_AgentManager");
}

size_t AgentManager::get_workers_count() const {
  size_t w_cnt = 0;
  act::for_each_actor<BaseAgent>(
    [&w_cnt](const BaseAgent* a) {
      if (a->is_worker()) {
        w_cnt++;
      }
    });
  return w_cnt;
}

size_t AgentManager::get_mining_workers_count() const {
  size_t cnt = 0;
  act::for_each_actor<BaseAgent>(
    [&cnt](const BaseAgent* a) {
      if (a->is_worker()) {
        auto w = static_cast<const WorkerAgent*>(a);
        if (w->fsm_state() == WorkerState::GATHER_MINERALS) {
          cnt++;
        }
      }
    });
  return cnt;
}

const BaseAgent*
AgentManager::find_closest_free_worker(const TilePosition& pos) const {
  const BaseAgent* base_agent = nullptr;
  float best_dist = LIKE_VERY_FAR;

  act::for_each_actor<BaseAgent>(
    [&best_dist,&base_agent,&pos](const BaseAgent* a) {
      if (a->is_available_worker()) {
        float c_dist = rnp::distance(pos, a->get_unit()->getTilePosition());
        if (c_dist < best_dist) {
          best_dist = c_dist;
          base_agent = a;
        }
      }
    });
  return base_agent;
}

bool AgentManager::is_any_agent_repairing_this_agent(
  const BaseAgent* repaired_agent) const 
{
  auto result = false;

  act::for_each_actor<BaseAgent>(
    [&result,repaired_agent](const BaseAgent* a) {
      if (a->is_worker()) {
        auto unit = a->get_unit();
        if (unit->getTarget() != nullptr 
          && unit->getTarget()->getID() == repaired_agent->get_unit_id()) {
          //Already have an assigned builder
          result = true;
          return;
        }
      }
    });
  return result;
}

size_t AgentManager::get_in_production_count(UnitType type) const {
  size_t cnt = 0;
  act::for_each_actor<BaseAgent>(
    [&type,&cnt](const BaseAgent* a) {
      if (a->is_of_type(type) && a->get_unit()->isBeingConstructed()) {
        cnt++;
      }
    });
  return cnt;
}

bool AgentManager::have_a_completed_building(UnitType type) const {
  auto result = false;

  act::for_each_actor<BaseAgent>(
    [&result,&type](const BaseAgent* a) {
      if (a->is_of_type(type)) {
        if (not a->get_unit()->isBeingConstructed()) {
          result = true;
          return;
        }
      }
    });
  return result;
}

size_t AgentManager::get_units_of_type_count(UnitType type) const {
  size_t cnt = 0;
  act::for_each_actor<BaseAgent>(
    [&cnt,&type](const BaseAgent* a) {
      if (a->is_alive()) {
        if (a->is_of_type(type)) {
          cnt++;
        }
      }
    });
  return cnt;
}

size_t AgentManager::get_finished_units_count(UnitType type) const {
  size_t cnt = 0;
  act::for_each_actor<BaseAgent>(
    [&cnt,&type](const BaseAgent* a) {
      if (a->is_alive()) {
        if (a->is_of_type(type) && not a->get_unit()->isBeingConstructed()) {
          cnt++;
        }
      }
    });
  return cnt;
}

size_t AgentManager::get_bases_count() const {
  size_t cnt = 0;
  act::for_each_actor<BaseAgent>(
    [&cnt](const BaseAgent* a) {
      if (a->is_alive()) {
        if (a->unit_type().isResourceDepot() && not a->get_unit()->isBeingConstructed()) {
          cnt++;
        }
      }
    });
  return cnt;
}

bool AgentManager::is_worker_targeting_unit(Unit target) const {
  auto result = false;
  act::for_each_actor<BaseAgent>(
    [&result,target](const BaseAgent* a) {
      if (a->is_worker()) {
        //auto unit = a->get_unit();
        auto unitTarget = a->get_unit()->getTarget();
        if (unitTarget != nullptr && unitTarget->getID() == target->getID()) {
          result = true;
          return;
        }
      }
    });
  return result;
}

void AgentManager::handle_message(act::Message* incoming) {
  unhandled_message(incoming);
}
