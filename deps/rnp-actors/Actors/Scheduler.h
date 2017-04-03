#pragma once

#include "ActorId.h"
#include <map>
#include <memory>
#include <functional>

namespace act {

class Actor;

// Stores a registry of everything
// Root object for a swarm of actors, performs scheduling too
class Scheduler {
  // TODO: optimize, split this into multiple maps per actor flavour
  using ActorPtr = std::unique_ptr<Actor>;
  using ActorMap = std::map<ActorId, ActorPtr>;
  ActorMap actors_; // running
  ActorMap actors_suspended_; // sleeping
#error TODO: wakeup timer or wakeup message?

  static std::unique_ptr<Scheduler> singleton_;
  uint32_t next_actor_id_ = 0;
  
  // global time for everything, is externally controlled and synced with
  // broodwar current frame
  size_t now_; 

public:
  Scheduler(): actors_(), now_(0) {
  }

  size_t now() const {
    return now_;
  }

  void hard_sync_now(size_t new_now);

  void wakeup(const ActorId& id) {
    // TODO: wake up when run queues are added
  }

  uint32_t next_id() { 
    return next_actor_id_++;
  }

  void add(const ActorId& id, ActorPtr&& movable_ptr);

  const auto& all_actors() const {
    return actors_;
  }

  static Scheduler& instance() {
    if (!singleton_) {
      singleton_ = std::make_unique<Scheduler>();
    }
    return *singleton_;
  }

  // Gives CPU time to the scheduler
  void tick();

  Actor* find_actor(const ActorId& id) {
    auto itr = actors_.find(id);
    if (itr == actors_.end()) {
      return nullptr;
    }
    return itr->second.get();
  }

  // Removes actor from the run queue
  void suspend(const ActorId& id, size_t tick_count) {
    suspend(id);
#error TODO: Setup wake up timer or something
  }

  void suspend(const ActorId& id);
  void schedule(const ActorId& id);
};

inline Scheduler& sched() {
  return Scheduler::instance();
}

} // ns act
