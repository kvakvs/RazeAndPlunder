#include "StrategySelector.h"
#include "Protoss/ProtossMain.h"
#include "Terran/TerranMain.h"
#include "Zerg/LurkerRush.h"
#include "Zerg/ZergMain.h"
#include <fstream>

using namespace BWAPI;

bool StrategyStats::matches() const {
  std::string mMapHash = Broodwar->mapHash();
  std::string mOwnRace = Broodwar->self()->getRace().getName();
  if (mMapHash == map_hash_ && mOwnRace == my_race_) {
    Race oRace = Broodwar->enemy()->getRace();
    if (oRace.getID() != Races::Unknown.getID()) {
      //Opponent race is known. Match race as well.
      if (oRace.getName() == opponent_race_) {
        return true;
      }
      else {
        return false;
      }
    }
    return true;
  }
  return false;
}

StrategySelector::StrategySelector() {
  active_ = true;

  strategies_.push_back(Strategy(Races::Protoss, ProtossMain::getStrategyId()));
  strategies_.push_back(Strategy(Races::Terran, TerranMain::getStrategyId()));
  strategies_.push_back(Strategy(Races::Zerg, LurkerRush::getStrategyId()));
  strategies_.push_back(Strategy(Races::Zerg, ZergMain::getStrategyId()));

  loadStats();
}

StrategySelector::~StrategySelector() {
}

void StrategySelector::enable() {
  active_ = true;
}

void StrategySelector::disable() {
  active_ = false;
}

void StrategySelector::selectStrategy() {
  int totWon = 0;
  int totPlay = 0;
  for (int i = 0; i < (int)stats_.size(); i++) {
    std::string mOwnRace = Broodwar->self()->getRace().getName();

    if (stats_.at(i).matches()) {
      totWon += stats_.at(i).won_;
      totPlay += stats_.at(i).total_;
    }
  }
  if (totPlay == 0) totPlay = 1; //To avoid division by zero

  //Random probability select one strategy
  bool found = false;
  int i = 0;
  while (not found) {
    i = rand() % (int)stats_.size();

    //Entry matches
    if (stats_.at(i).matches()) {
      //Calculate probability for this entry.
      int chance = stats_.at(i).won_ * 100 / stats_.at(i).getTotal();
      chance = chance * totWon / totPlay;

      //Have 75% chance to try a strategy that
      //hasn't been tested much yet.
      if (stats_.at(i).total_ <= 2) chance = 75;

      //Set a max/min so all strategies have a chance
      //to be played.
      if (chance < 15) chance = 15;
      if (chance > 85) chance = 85;

      //Make the roll!
      int roll = rand() % 100;
      if (roll <= chance) {
        current_strategy_id_ = stats_.at(i).strat_id_;
        Broodwar << "Strategy selected: " << current_strategy_id_ << " (Roll: " << roll << " Prob: " << chance << ")" << std::endl;
        found = true;
        return;
      }
    }
  }
}

Commander::Ptr StrategySelector::getStrategy() {
  int tot = 0;
  for (int i = 0; i < (int)stats_.size(); i++) {
    if (stats_.at(i).matches()) tot++;
  }

  if (tot > 0) {
    //Select a strategy among the tested
    //ones.
    selectStrategy();
  }
  else {
    //No strategy has been tested for this combo.
    //Return one of the available strategies.
    auto self_race = Broodwar->self()->getRace().getID();
    if (self_race == Races::Terran.getID()) current_strategy_id_ = "TerranMain";
    else if (self_race == Races::Protoss.getID()) current_strategy_id_ = "ProtossMain";
    else if (self_race == Races::Zerg.getID()) current_strategy_id_ = "LurkerRush";
  }
  Broodwar << "Strategy: " << current_strategy_id_ << std::endl;

  //Get Commander for strategy
  if (current_strategy_id_ == "ProtossMain") return std::make_shared<ProtossMain>();
  if (current_strategy_id_ == "TerranMain") return std::make_shared<TerranMain>();
  if (current_strategy_id_ == "LurkerRush") return std::make_shared<LurkerRush>();
  if (current_strategy_id_ == "ZergMain") return std::make_shared<ZergMain>();

  bwem_assert(false);
  return nullptr;
}

void StrategySelector::printInfo() {
  Broodwar->drawTextScreen(180, 5, "\x0F%s", current_strategy_id_.c_str());
}

void StrategySelector::loadStats() {
  std::string filename = getFilename();

  std::ifstream inFile;
  inFile.open(filename.c_str());
  if (not inFile) {
    //No file found.
    return;
  }
  else {
    std::string line;
    char buffer[256];
    while (not inFile.eof()) {
      inFile.getline(buffer, 256);
      if (buffer[0] != ';') {
        std::stringstream ss;
        ss << buffer;
        line = ss.str();
        addEntry(line);
      }
    }
    inFile.close();
  }
}

void StrategySelector::addEntry(std::string line) {
  if (line == "") return;

  StrategyStats s = StrategyStats();
  int i;
  std::string t;

  i = line.find(";");
  t = line.substr(0, i);
  s.strat_id_ = t;
  line = line.substr(i + 1, line.length());

  i = line.find(";");
  t = line.substr(0, i);
  s.my_race_ = t;
  line = line.substr(i + 1, line.length());

  i = line.find(";");
  t = line.substr(0, i);
  s.opponent_race_ = t;
  line = line.substr(i + 1, line.length());

  i = line.find(";");
  t = line.substr(0, i);
  s.won_ = toInt(t);
  line = line.substr(i + 1, line.length());

  i = line.find(";");
  t = line.substr(0, i);
  s.lost_ = toInt(t);
  line = line.substr(i + 1, line.length());

  i = line.find(";");
  t = line.substr(0, i);
  s.draw_ = toInt(t);
  line = line.substr(i + 1, line.length());

  i = line.find(";");
  t = line.substr(0, i);
  s.total_ = toInt(t);
  line = line.substr(i + 1, line.length());

  i = line.find(";");
  t = line.substr(0, i);
  s.map_name_ = t;
  line = line.substr(i + 1, line.length());

  i = line.find(";");
  t = line.substr(0, i);
  s.map_hash_ = t;
  line = line.substr(i + 1, line.length());

  stats_.push_back(s);
}

int StrategySelector::toInt(std::string& str) {
  std::stringstream ss(str);
  int n;
  ss >> n;
  return n;
}

std::string StrategySelector::getFilename() {
  std::stringstream ss;
  ss << "bwapi-data\\AI\\";
  //ss << "bwapi-data\\read\\"; //Tournament persistent storage version
  ss << "Strategies_OpprimoBot.csv";

  return ss.str();
}

std::string StrategySelector::getWriteFilename() {
  std::stringstream ss;
  ss << "bwapi-data\\AI\\";
  //ss << "bwapi-data\\write\\"; //Tournament persistent storage version
  ss << "Strategies_OpprimoBot.csv";

  return ss.str();
}

void StrategySelector::addResult(int win) {
  if (not active_) return;

  std::string opponentRace = Broodwar->enemy()->getRace().getName();
  std::string mapHash = Broodwar->mapHash();

  //Check if we have the entry already
  for (int i = 0; i < (int)stats_.size(); i++) {
    if (mapHash == stats_.at(i).map_hash_ && opponentRace == stats_.at(i).opponent_race_ && current_strategy_id_ == stats_.at(i).strat_id_) {
      stats_.at(i).total_++;
      if (win == 0) stats_.at(i).lost_++;
      if (win == 1) stats_.at(i).won_++;
      if (win == 2) stats_.at(i).draw_++;
      return;
    }
  }

  StrategyStats s = StrategyStats();
  s.total_++;
  if (win == 0) s.lost_++;
  if (win == 1) s.won_++;
  if (win == 2) s.draw_++;
  s.strat_id_ = current_strategy_id_;
  s.map_hash_ = mapHash;
  s.map_name_ = Broodwar->mapFileName();
  s.my_race_ = Broodwar->self()->getRace().getName();
  s.opponent_race_ = opponentRace;
  stats_.push_back(s);
}

void StrategySelector::saveStats() {
  if (not active_) return;

  //Fill entries in stats file for combinations that have
  //not yet been played.
  std::string mapHash = Broodwar->mapHash();
  std::string opponentRace = Broodwar->enemy()->getRace().getName();
  std::string ownRace = Broodwar->self()->getRace().getName();

  for (int i = 0; i < (int)strategies_.size(); i++) {
    bool found = false;
    for (int s = 0; s < (int)stats_.size(); s++) {
      if (strategies_.at(i).strat_id_ == stats_.at(s).strat_id_ && mapHash == stats_.at(s).map_hash_ && opponentRace == stats_.at(s).opponent_race_) {
        //Matches
        found = true;
        break;
      }
    }

    if (not found) {
      //Only fill in the strategies for
      //the same race
      if (ownRace == strategies_.at(i).race_.getName()) {
        //Add entry
        StrategyStats s = StrategyStats();
        s.map_hash_ = mapHash;
        s.map_name_ = Broodwar->mapFileName();
        s.opponent_race_ = opponentRace;
        s.my_race_ = strategies_.at(i).race_.getName();
        s.strat_id_ = strategies_.at(i).strat_id_;

        stats_.push_back(s);
      }
    }
  }

  //Save the file
  std::string filename = getWriteFilename();

  std::ofstream outFile;
  outFile.open(filename.c_str());
  if (not outFile) {
    Broodwar << "Error writing to stats file!" << std::endl;
  }
  else {
    for (int i = 0; i < (int)stats_.size(); i++) {
      std::stringstream s2;
      s2 << stats_.at(i).strat_id_;
      s2 << ";";
      s2 << stats_.at(i).my_race_;
      s2 << ";";
      s2 << stats_.at(i).opponent_race_;
      s2 << ";";
      s2 << stats_.at(i).won_;
      s2 << ";";
      s2 << stats_.at(i).lost_;
      s2 << ";";
      s2 << stats_.at(i).draw_;
      s2 << ";";
      s2 << stats_.at(i).total_;
      s2 << ";";
      s2 << stats_.at(i).map_name_;
      s2 << ";";
      s2 << stats_.at(i).map_hash_;
      s2 << ";\n";

      outFile << s2.str();
    }
    outFile.close();
  }
}
