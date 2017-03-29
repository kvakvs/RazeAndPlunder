#pragma once

namespace act {

template <class Cls>
void for_each_actor(std::function<void(const Cls*)> fun) {
  for (const auto& actor : sched().all_actors()) {
    auto cls = dynamic_cast<const Cls*>(actor.second.get());
    if (cls) {
      fun(cls);
    }
  }
}

enum class ForEach { Break, Continue };
enum class ForEachResult { Interrupted, Finished };

// Same as for_each_alive, but the handler must return true to continue or
// false to end the loop immediately
template <class Cls> ForEachResult
interruptible_for_each_actor(std::function<ForEach(const Cls*)> fun) {
  for (const auto& actor : sched().all_actors()) {
    auto cls = dynamic_cast<const Cls*>(actor.second.get());
    if (cls) {
      if (fun(cls) == ForEach::Break) {
        return ForEachResult::Interrupted;
      }
    }
  }
  return ForEachResult::Finished;
}

// Iterate set of actor ids, resolve each actor and call a handler
template <class Cls>
void for_each_in(const ActorId::Set& ids, std::function<void(const Cls*)> fun) {
  for (const auto& id : ids) {
    auto actor0 = sched().find_actor(id);
    auto actor = dynamic_cast<const Cls*>(actor0);
    if (actor) {
      fun(actor);
    }
  }
}

// Same as for_each_in, but the handler must return true to continue or false to
// end the loop immediately
template <class Cls> ForEachResult
interruptible_for_each_in(const ActorId::Set& ids,
                          std::function<ForEach(const Cls*)> fun) {
  for (const auto& id : ids) {
    auto actor0 = sched().find_actor(id);
    auto actor = dynamic_cast<const Cls*>(actor0);
    if (actor) {
      if (fun(actor) == ForEach::Break) {
        return ForEachResult::Interrupted;
      };
    }
  }
  return ForEachResult::Finished;
}

// Looks up the actor by 'dst' id, constructs a message object from args and
// places the message object into actor's message queue.
template <class T, typename... Args>
void send_message(const ActorId& dst, Args&& ...args) {
  auto actor = sched().find_actor(dst);
  if (actor) {
    auto m = std::make_unique<T>(std::forward<Args>(args)...);
    assert(actor->ac_flavour() == dst.flavour());
    actor->ac_accept_message(std::move(m));
  }
}

template <class T, typename... Args>
void send_message(const Actor* dst, Args&& ...args) {
  send_message(dst->self(), std::forward<Args>(args)...);
}

// Sends a templated message to the actor, where the message contains 
// a function to execute on a mutable actor pointer converted to T*
// IMPORTANT: Don't let lambdas capture references, use [=] instead
template <class T>
void modify_actor(const ActorId& id, std::function<void(T*)> func) {
  send_message<ActiveMessage<T>>(id, func);
}

//template <class T>
//void operator << (const ActorId& dst, std::unique_ptr<T>&& msg) {
//  auto actor = sched().find_actor(dst);
//  if (actor) {
//    assert(actor->ac_flavour() == dst.flavour());
//    actor->ac_accept_message(std::move(msg));
//  }
//}

//template <class T>
//void operator << (const Actor* dst, std::unique_ptr<T>&& msg) {
//  dst->self() << std::move(msg);
//}

// Find an actor of given type, check class with dynamic cast
// TODO: Optimize the storage for actors, group by type
template <class T>
const T* whereis(const ActorId& id) {
  if (not id.is_valid()) { return nullptr; }
  return dynamic_cast<T*>(sched().find_actor(id));
}

// Creates unique ptr of given object with ctor args, and places it into the
// actor dictionary
template <class ActorClass, class Flavour, typename... Args>
ActorId spawn(Flavour flavour, Args&& ...args) {
  auto& scheduler = sched();
  auto new_actor = std::make_unique<ActorClass>(std::forward<Args>(args)...);
  auto new_id = ActorId(flavour, scheduler.next_id());
  scheduler.add(new_id, std::move(new_actor));
  return new_id;
}

// Creates unique ptr of given object with ctor args, and places it into the
// actor dictionary. Returns const pointer to the object
template <class ActorClass, class Flavour, typename... Args>
const ActorClass* spawn_get_ptr(Flavour flavour, Args&& ...args) {
  auto& scheduler = sched();
  auto new_actor = std::make_unique<ActorClass>(std::forward<Args>(args)...);
  const ActorClass* new_actor_p = new_actor.get();
  auto new_id = ActorId(flavour, scheduler.next_id());
  scheduler.add(new_id, std::move(new_actor));
  return new_actor_p;
}

// Same as spawn with flavour, but now also has constructor which is allowed to
// modify newborn actor before it is added to the world
template <class ActorClass, class Flavour, typename... Args>
ActorId spawn(Flavour flavour, std::function<void(ActorClass*)> ctor, Args&& ...args) {
  auto& scheduler = sched();
  auto new_actor = std::make_unique<ActorClass>(std::forward<Args>(args)...);

  ctor(new_actor.get());

  auto new_id = ActorId(flavour, scheduler.next_id());
  scheduler.add(new_id, std::move(new_actor));
  return new_id;
}

template <class ActorClass>
void spawn(const ActorId& id, std::unique_ptr<ActorClass> && ptr) {
  auto& scheduler = sched();
  scheduler.add(id, std::move(ptr));
}

} // ns act
