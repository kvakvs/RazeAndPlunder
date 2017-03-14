#pragma once

#include "Commander.h"

struct StrategyStats {
  std::string mapHash;
  std::string mapName;
  std::string strategyId;
  std::string ownRace;
  std::string opponentRace;
  int won;
  int lost;
  int draw;
  int total;

  StrategyStats() {
    won = 0;
    lost = 0;
    draw = 0;
    total = 0;
  }

  int getTotal() {
    if (total == 0) return 1; //To avoid division by zero.
    return total;
  }

  bool matches();
};

struct Strategy {
  BWAPI::Race race;
  std::string strategyId;

  Strategy(BWAPI::Race mRace, std::string mId) {
    race = mRace;
    strategyId = mId;
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
  std::vector<Strategy> strategies;
  std::vector<StrategyStats> stats;

  static StrategySelector* instance;
  StrategySelector();

  std::string currentStrategyId;

  std::string getFilename();
  std::string getWriteFilename();
  void addEntry(std::string line);
  int toInt(std::string& str);

  void selectStrategy();

  bool active;

public:
  // Returns the instance of the class. 
  static StrategySelector* getInstance();

  // Destructor 
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
