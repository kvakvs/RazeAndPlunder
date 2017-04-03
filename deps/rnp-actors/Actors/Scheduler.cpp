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

void Scheduler::tick() {
  // TODO: implement run queues, and wake up for periodic processing signal
  for (auto& a : actors_) {
    a.second->ac_tick();
  }
}

void Scheduler::suspend(const ActorId& id) {
  auto run_i = actors_.find(id);
  if (run_i == actors_.end()) {
    // if its not running, then i'm sure it is already suspended!
    assert(actors_.find(id) != actors_.end());
    return;
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
  assert(new_now > now_);
  now_ = new_now;
}

void Scheduler::add(const ActorId& id, Scheduler::ActorPtr&& movable_ptr) {
  //assert(find_actor(id) == nullptr);
  movable_ptr->set_ac_id(id);
  actors_[id] = std::move(movable_ptr);
}

} // act
