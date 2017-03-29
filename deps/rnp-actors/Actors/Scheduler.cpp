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

void Scheduler::add(const ActorId& id, Scheduler::ActorPtr&& movable_ptr) {
  //assert(find_actor(id) == nullptr);
  movable_ptr->set_ac_id(id);
  actors_[id] = std::move(movable_ptr);
}

} // act
