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
class TerranMain : public CommanderStrategy
                 , public rnp::FiniteStateMachine<TerranStrategyState>
{
public:
  size_t workers_per_refinery() const override;
  size_t adjust_workers_count(size_t workers_now) const override;

private:
  using FsmBaseClass = rnp::FiniteStateMachine<TerranStrategyState>;

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
private:
  void on_enter_stage1();
  void on_enter_stage3();
  void on_enter_stage4();
  void on_enter_stage5();
  void on_enter_stage6();
  void on_enter_stage7();
  void on_enter_stage8();
  void on_enter_endgame();
  void on_enter_stage2() const;
};
