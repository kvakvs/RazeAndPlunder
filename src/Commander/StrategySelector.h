#pragma once

#include "Commander/Commander.h"

namespace rnp {
  enum class MatchResult;
}

struct StrategyStats {
  std::string map_hash_;
  std::string map_name_;
  std::string strat_id_;
  std::string my_race_;
  std::string opponent_race_;
  int won_ = 0;
  int lost_ = 0;
  int draw_ = 0;
  int total_ = 0;

  StrategyStats(): map_hash_(), map_name_(), strat_id_(), my_race_()
      , opponent_race_()
  {
  }

  int get_total() const {
    if (total_ == 0) return 1; //To avoid division by zero.
    return total_;
  }

  bool matches() const;
};

struct Strategy {
  BWAPI::Race race_ = BWAPI::Races::Unknown;
  std::string strat_id_;

  Strategy(BWAPI::Race race, std::string mId): strat_id_() {
    race_ = race;
    strat_id_ = mId;
  }
};

/** When a game is started a strategy is selected depending on the map and, if known,
 * the opponent race. After each game the result is stored to a statistics file
 * (bwapi-data/AI/Strategies_OpprimoBot.csv). Strategies that previously have been
 * successful have a higher probability of being selected, but all strategies have
 * at least 15% chance of being used. 
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class StrategySelector {
private:
  std::vector<Strategy> strategies_;
  std::vector<StrategyStats> stats_;

  std::string current_strategy_id_;
  bool active_ = false;

private:
  static std::string get_filename();

  static std::string get_write_filename();

  void add_entry(const std::string& line);

  static int to_int(std::string& str);

  void select_strategy();

public:
  StrategySelector();
  ~StrategySelector();

  // Returns the selected strategy for this game. 
  const CommanderStrategy* spawn_strategy_actor();

  // Loads the stats file. 
  void load_stats();

  // Prints debug info to the screen. 
  void print_info() const;

  // Adds the result after a game is finished. 
  void add_result(rnp::MatchResult win);

  // Saves the stats file. 
  void save_stats();

  // Enable strategy updates. 
  void enable();

  // Disable strategy updates. 
  void disable();
};
