#pragma once

#include "Commander/Commander.h"

//
// Implements Opprimobot's default terran one-vs-all strategy
//

enum class TerranStrategyState {
  NoStage,    // does nothing, switches to stage1
  Stage1,     // first manual build orders
  Stage2,     // Get some defense teams
  Stage3,
  Stage4,
  Stage5,
  Stage6,
  Stage7,
  Stage8,
  EndGame
};

/**  This is the Commander class for a defensive Marine/Siege Tank/Goliath
 * based strategy.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class TerranMain : public Commander
                 , public rnp::FiniteStateMachine<TerranStrategyState>
{
private:
  using FsmBaseClass = rnp::FiniteStateMachine<TerranStrategyState>;

  act::ActorId main_sq_;
  act::ActorId secondary_sq_;
  act::ActorId backup1_sq_;
  act::ActorId backup2_sq_;
  act::ActorId rush_sq_;
  act::ActorId scout2_sq_;

public:
  TerranMain();
  ~TerranMain();

  // Called each update to issue orders.
  void tick() override;

  // Returns the unique id for this strategy. 
  static std::string get_strategy_id() {
    return "TerranMain";
  }

  void fsm_on_transition(TerranStrategyState old_st, 
                         TerranStrategyState new_st) override;
};
