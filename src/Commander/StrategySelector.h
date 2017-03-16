#pragma once

#include "Commander/Commander.h"

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

  int getTotal() {
    if (total_ == 0) return 1; //To avoid division by zero.
    return total_;
  }

  bool matches() const;
};

struct Strategy {
  BWAPI::Race race_;
  std::string strat_id_;

  Strategy(BWAPI::Race mRace, std::string mId) {
    race_ = mRace;
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
  std::vector<Strategy> strategies_;
  std::vector<StrategyStats> stats_;

  std::string current_strategy_id_;
  bool active_;

private:
  std::string getFilename();
  std::string getWriteFilename();
  void addEntry(std::string line);
  int toInt(std::string& str);

  void selectStrategy();

public:
  StrategySelector();
  ~StrategySelector();

  // Returns the selected strategy for this game. 
  Commander::Ptr getStrategy();

  // Loads the stats file. 
  void loadStats();

  // Prints debug info to the screen. 
  void printInfo();

  // Adds the result after a game is finished. 
  void addResult(int win);

  // Saves the stats file. 
  void saveStats();

  // Enable strategy updates. 
  void enable();

  // Disable strategy updates. 
  void disable();
};
