#include "Scheduler.h"
#include "Actor.h"

namespace act {

std::unique_ptr<Scheduler> Scheduler::singleton_;

void signal(const ActorId& id, Signal sig) {
  auto& scheduler = sched();
  auto actor = scheduler.find_actor(id);
  if (actor) {
    actor->ac_signal(sig);
    scheduler.wakeup(id);
  }
  else {
    assert(not "exists");
  }
}

void Scheduler::check_wakeups() {
  for (auto iter = actors_suspended_.begin(), i_end = actors_suspended_.end();
       iter != i_end; /* dum dum */ ) 
  {
    if (iter->second->ac_is_scheduled_wakeup(now_)) {
      actors_[iter->first] = std::move(iter->second);
      iter = actors_suspended_.erase(iter);
    }
    else {
      ++iter;
    }
  }
}

void Scheduler::tick() {
  check_wakeups();

  ActorList to_run;
  constexpr size_t MAX_ITER_PER_TICK = 1;

  for (size_t i = 0; i < MAX_ITER_PER_TICK; ++i) {
    to_run.clear();

    // Collect all runnable actor pointers
    for (auto& a : actors_) {
      // There is a guarantee that no actor ever is destroyed while here
      // When they are going to die, they set exit bit in their flags
      to_run.push_back(a.second.get());
    }

    // Run the logic for all runnable actors
    for (auto& actor : to_run) {
      actor->ac_tick();
    }
  }
}

Actor* Scheduler::find_actor(const ActorId& id) {
  auto itr = actors_.find(id);
  if (itr == actors_.end()) {
    itr = actors_suspended_.find(id);
    if (itr == actors_suspended_.end()) {
      return nullptr;
    }
  }
  return itr->second.get();
}

void Scheduler::suspend(const ActorId& id, TimeType wakeup_t) {
  auto run_i = actors_.find(id);
  if (run_i == actors_.end()) {
    // if its not running, then i'm sure it is already suspended!
    assert(actors_suspended_.find(id) != actors_suspended_.end());
    return;
  }
  if (wakeup_t) {
    run_i->second->ac_wakeup_at(wakeup_t);
  }
  actors_suspended_[id] = std::move(run_i->second);
  actors_.erase(run_i);
}

void Scheduler::schedule(const ActorId& id) {
  auto suspended_i = actors_suspended_.find(id);
  if (suspended_i == actors_suspended_.end()) {
    // if its not suspended, then i'm sure it is already running!
    assert(actors_.find(id) != actors_.end());
    return;
  }
  actors_[id] = std::move(suspended_i->second);
  actors_suspended_.erase(suspended_i);
}

void Scheduler::hard_sync_now(size_t new_now) {
  assert(new_now > now_ || (not now_));
  now_ = new_now;
}

void Scheduler::add(const ActorId& id, Scheduler::ActorPtr&& movable_ptr) {
  //assert(find_actor(id) == nullptr);
  movable_ptr->set_ac_id(id);
  actors_[id] = std::move(movable_ptr);
}

Scheduler::ActorList Scheduler::all_actors() const {
  ActorList result;
  for (auto& a : actors_) {
    result.push_back(a.second.get());
  }
  for (auto& a : actors_suspended_) {
    result.push_back(a.second.get());
  }
  return result;
}

} // act
