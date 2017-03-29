#pragma once 

#include <cstdint>
#include <memory>
#include <list>
#include <iso646.h>
#include <unordered_set>
#include <cassert>
#include "Scheduler.h"
#include "ActorId.h"

namespace act {

enum class Signal {
  Kill, PeriodicThink
};

//------------------------------------------------------------
// Base class for a message
//------------------------------------------------------------
class Message {
public:
  using Ptr = std::unique_ptr<Message>;

  // If set, will be allowed to perform some operations on a mutable 
  // pointer to actor
  virtual bool apply_code(Actor* actor) { return false;  }

  virtual ~Message();
};

template <class T>
class ActiveMessage: public Message {
public:
  explicit ActiveMessage(const std::function<void(T*)>& apply_fun)
    : apply_fun_(apply_fun) {
  }

  // If this is set, it will be called
  std::function<void(T*)> apply_fun_;

  // If set, will be allowed to perform some operations on a mutable 
  // pointer to actor
  bool apply_code(Actor* actor) override {
    if (static_cast<bool>(apply_fun_)) {
      apply_fun_(static_cast<T*>(actor));
      return true;
    }
    return false;
  }
};

namespace msg {

class Monitor : public Message {
public:
  ActorId target_;

  Monitor(const ActorId& id) : target_(id) {}

  static void send(const ActorId& receiver, const ActorId& t);
};

} // ns msg

//------------------------------------------------------------
// Base actor, superclass for everything-actor
// Registers itself in "Process registry", reacts to messages, can send messages
// Receives CPU time via the method call
//------------------------------------------------------------
class Actor {
  ActorId ac_id_;
  // TODO: Message matching one by one and waking up by scheduler
  std::list<Message::Ptr> ac_msgs_;
  ActorId::Set ac_monitors_;

  class Signals {
  public:
    bool    sig_kill : 1; // having this true will remove the actor
    bool    sig_periodic_think : 1; // time to wake up and do on_frame logic
    Signals(): sig_kill(false), sig_periodic_think(false) {
    }
  };
  Signals ac_sig_;

public:
  using Ptr = std::unique_ptr<Actor>;

  Actor(): ac_id_(), ac_msgs_(), ac_monitors_(), ac_sig_() {}
  virtual ~Actor();

  const ActorId& self() const {
    assert(ac_id_.is_valid());
    return ac_id_;
  }
  void set_ac_id(const ActorId& id) { ac_id_ = id; }

  // Return integer flavour of this actor, used for incoming message filtering
  virtual uint32_t ac_flavour() const = 0;

  virtual void handle_message(Message *m) = 0;

  // Internal tick for actors, calls virtual tick
  void ac_tick() {
    ac_handle_mailbox();
    tick();
  }

  virtual void tick() = 0;

  void ac_accept_message(Message::Ptr&& m) {
    ac_msgs_.push_back(std::move(m));
  }

  void ac_monitor(const ActorId& id);

  void ac_signal(Signal sig) {
    switch (sig) {
    case Signal::Kill: 
      ac_sig_.sig_kill = true;
      break;
    case Signal::PeriodicThink:
      ac_sig_.sig_periodic_think = true;
      break;
    default:
      assert(not "signal supported");
    }
  }

protected:
  // Called by handle_message only, if the message is not picked up
  void unhandled_message(Message *m) {
    assert(not "handled message");
  }

private:
  void ac_handle_mailbox() {
    while (not ac_msgs_.empty()) {
      Message::Ptr& ptr = ac_msgs_.front();
      // Attempt to let active message run its code. If the message was a
      // data only kind, then we call handle_message instead
      if (not ptr->apply_code(this)) {
        handle_message(ptr.get());
      }
      ac_msgs_.pop_front();
    }
  }
};

void signal(const ActorId& id, Signal sig);

} // ns act
