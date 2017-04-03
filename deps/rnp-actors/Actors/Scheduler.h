#pragma once

#include "ActorId.h"
#include <map>
#include <memory>
#include <functional>
#include <set>

namespace act {

class Actor;

using TimeType = size_t;

// Stores a registry of everything
// Root object for a swarm of actors, performs scheduling too
class Scheduler {
  // TODO: optimize, split this into multiple maps per actor flavour
  using ActorPtr = std::unique_ptr<Actor>;
  using ActorMap = std::map<ActorId, ActorPtr>;
  using ActorList = std::list<Actor*>;
  ActorMap actors_; // running
  ActorMap actors_suspended_; // sleeping

  static std::unique_ptr<Scheduler> singleton_;
  uint32_t next_actor_id_ = 0;
  
  // global time for everything, is externally controlled and synced with
  // broodwar current frame
  TimeType now_; 

public:
  Scheduler(): actors_(), now_(0) {
  }

  TimeType now() const {
    return now_;
  }

  void hard_sync_now(TimeType new_now);

  void wakeup(const ActorId& id) {
    // TODO: wake up when run queues are added
  }

  uint32_t next_id() { 
    return next_actor_id_++;
  }

  void add(const ActorId& id, ActorPtr&& movable_ptr);

  ActorList all_actors() const;

  static Scheduler& instance() {
    if (!singleton_) {
      singleton_ = std::make_unique<Scheduler>();
    }
    return *singleton_;
  }

  // Scans suspended actors and checks if the time has come for them to wake up
  // TODO: Optimize maybe? or something
  void check_wakeups();

  // Gives CPU time to the scheduler
  void tick();

  Actor* find_actor(const ActorId& id);

  // Removes actor from the run queue
  void suspend(const ActorId& id, TimeType tick_count);
  void suspend(const ActorId& id) {
    suspend(id, 0);
  }
  void schedule(const ActorId& id);
};

inline Scheduler& sched() {
  return Scheduler::instance();
}

inline void suspend(const ActorId& id, TimeType wakeup_time) {
  sched().suspend(id, wakeup_time);
}

inline void suspend(const ActorId& id) {
  sched().suspend(id, 0);
}

} // ns act
