#include "Actor.h"
#include "Scheduler.h"
#include "Algorithm.h"

namespace act {

std::string ActorId::string() const {
  char out[64];
  auto result = sprintf_s(out, sizeof(out), "<%d.%d>", flavour_, id_);
  return std::string(out, result);
}

Message::~Message() {
}

Actor::~Actor() {
  for (auto& morigin: ac_monitors_) {

  }
}

void Actor::ac_monitor(const ActorId& id) {
  auto dst = sched().find_actor(id);
  if (dst) {
    ac_monitors_.insert(id);
    dst->ac_monitors_.insert(self());
  }
}

void msg::Monitor::send(const ActorId& receiver, const ActorId& t) {
  send_message<Monitor>(receiver, t);
}

} // ns act
