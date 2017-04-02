#pragma once

namespace rnp {

template <typename StateType>
class FiniteStateMachine {
private:
  StateType fsm_state_;
public:
  FiniteStateMachine(StateType start_st): fsm_state_(start_st) {
  }
  virtual ~FiniteStateMachine() {}

  void fsm_set_state(StateType st) {
    auto old_st = fsm_state_;
    fsm_state_ = st;
    fsm_on_transition(old_st, st);
  }
  
  StateType fsm_state() const { return fsm_state_; }

  virtual void fsm_on_transition(StateType old_st, StateType new_st) = 0;
};

} // ns rnp
