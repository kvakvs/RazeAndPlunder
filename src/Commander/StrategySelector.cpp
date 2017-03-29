#include "StrategySelector.h"
#include "Protoss/ProtossMain.h"
#include "Terran/TerranMain.h"
#include "Zerg/LurkerRush.h"
#include "Zerg/ZergMain.h"
#include <fstream>
#include "Glob.h"

using namespace BWAPI;

bool StrategyStats::matches() const {
  std::string map_hash = Broodwar->mapHash();
  std::string own_race = Broodwar->self()->getRace().getName();

  if (map_hash == map_hash_ && own_race == my_race_) {
    Race oRace = Broodwar->enemy()->getRace();
  
    if (oRace.getID() != Races::Unknown.getID()) {
      
      //Opponent race is known. Match race as well.
      if (oRace.getName() == opponent_race_) {
        return true;
      }
       return false;
    }
    return true;
  }
  return false;
}

StrategySelector::StrategySelector()
    : strategies_(), stats_(), current_strategy_id_()
{
  active_ = true;

//  strategies_.push_back(Strategy(Races::Protoss, ProtossMain::get_strategy_id()));
  strategies_.push_back(Strategy(Races::Terran, TerranMain::get_strategy_id()));
//  strategies_.push_back(Strategy(Races::Zerg, LurkerRush::get_strategy_id()));
//  strategies_.push_back(Strategy(Races::Zerg, ZergMain::get_strategy_id()));

  load_stats();
}

StrategySelector::~StrategySelector() {
}

void StrategySelector::enable() {
  active_ = true;
}

void StrategySelector::disable() {
  active_ = false;
}

void StrategySelector::select_strategy() {
  int tot_won = 0;
  int tot_play = 0;
  for (size_t i = 0; i < stats_.size(); i++) {
    //auto& mOwnRace = Broodwar->self()->getRace().getName();

    if (stats_[i].matches()) {
      tot_won += stats_[i].won_;
      tot_play += stats_[i].total_;
    }
  }
  if (tot_play == 0) tot_play = 1; //To avoid division by zero

  //Random probability select one strategy
  while (true) {
    auto i = rand() % (int)stats_.size();

    //Entry matches
    if (stats_[i].matches()) {
      //Calculate probability for this entry.
      int chance = stats_[i].won_ * 100 / stats_[i].getTotal();
      chance = chance * tot_won / tot_play;

      //Have 75% chance to try a strategy that
      //hasn't been tested much yet.
      if (stats_[i].total_ <= 2) chance = 75;

      //Set a max/min so all strategies have a chance
      //to be played.
      if (chance < 15) chance = 15;
      if (chance > 85) chance = 85;

      //Make the roll!
      int roll = rand() % 100;
      if (roll <= chance) {
        current_strategy_id_ = stats_[i].strat_id_;
        Broodwar << "Strategy selected: " << current_strategy_id_ 
          << " (Roll: " << roll << " Prob: " << chance << ")" << std::endl;
        rnp::log()->info("Strategy selected: {0} (Roll: {1}, prob: {2})",
                         current_strategy_id_, roll, chance);
        return;
      }
    }
  }
}

const Commander* StrategySelector::get_strategy() {
  int tot = 0;
  for (int i = 0; i < (int)stats_.size(); i++) {
    if (stats_[i].matches()) tot++;
  }

  if (tot > 0) {
    //Select a strategy among the tested
    //ones.
    select_strategy();
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
  rnp::log()->info("Strategy: {}", current_strategy_id_);

  //Get Commander for strategy
//  if (current_strategy_id_ == "ProtossMain") return std::make_shared<ProtossMain>();
//  if (current_strategy_id_ == "TerranMain") return std::make_shared<TerranMain>();
//  if (current_strategy_id_ == "LurkerRush") return std::make_shared<LurkerRush>();
//  if (current_strategy_id_ == "ZergMain") return std::make_shared<ZergMain>();
  return act::spawn_get_ptr<TerranMain>(ActorFlavour::Singleton);

  bwem_assert(false);
  return nullptr;
}

void StrategySelector::print_info() const {
  Broodwar->drawTextScreen(180, 5, "\x0F%s", current_strategy_id_.c_str());
}

void StrategySelector::load_stats() {
  std::string filename = get_filename();

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
        add_entry(line);
      }
    }
    inFile.close();
  }
}

void StrategySelector::add_entry(const std::string& line0) {
  if (line0.empty()) return;
  
  auto line(line0);

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
  s.won_ = to_int(t);
  line = line.substr(i + 1, line.length());

  i = line.find(";");
  t = line.substr(0, i);
  s.lost_ = to_int(t);
  line = line.substr(i + 1, line.length());

  i = line.find(";");
  t = line.substr(0, i);
  s.draw_ = to_int(t);
  line = line.substr(i + 1, line.length());

  i = line.find(";");
  t = line.substr(0, i);
  s.total_ = to_int(t);
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

int StrategySelector::to_int(std::string& str) {
  std::stringstream ss(str);
  int n;
  ss >> n;
  return n;
}

std::string StrategySelector::get_filename() {
  std::stringstream ss;
  ss << "bwapi-data\\AI\\";
  //ss << "bwapi-data\\read\\"; //Tournament persistent storage version
  ss << "Strategies_OpprimoBot.csv";

  return ss.str();
}

std::string StrategySelector::get_write_filename() {
  std::stringstream ss;
  ss << "bwapi-data\\AI\\";
  //ss << "bwapi-data\\write\\"; //Tournament persistent storage version
  ss << "Strategies_OpprimoBot.csv";

  return ss.str();
}

void StrategySelector::add_result(rnp::MatchResult win) {
  if (not active_) return;

  std::string opponentRace = Broodwar->enemy()->getRace().getName();
  std::string mapHash = Broodwar->mapHash();

  //Check if we have the entry already
  for (size_t i = 0; i < stats_.size(); i++) {
    if (mapHash == stats_[i].map_hash_ 
      && opponentRace == stats_[i].opponent_race_ 
      && current_strategy_id_ == stats_[i].strat_id_) 
    {
      stats_[i].total_++;
      switch (win) {
      case rnp::MatchResult::Loss: stats_[i].lost_++; break;
      case rnp::MatchResult::Win: stats_[i].won_++; break;
      case rnp::MatchResult::Draw: stats_[i].draw_++; break;
      }
      return;
    }
  }

  StrategyStats s = StrategyStats();
  s.total_++;
  switch (win) {
  case rnp::MatchResult::Loss: s.lost_++; break;
  case rnp::MatchResult::Win: s.won_++; break;
  case rnp::MatchResult::Draw: s.draw_++; break;
  }
  s.strat_id_ = current_strategy_id_;
  s.map_hash_ = mapHash;
  s.map_name_ = Broodwar->mapFileName();
  s.my_race_ = Broodwar->self()->getRace().getName();
  s.opponent_race_ = opponentRace;
  stats_.push_back(s);
}

void StrategySelector::save_stats() {
  if (not active_) return;

  //Fill entries in stats file for combinations that have
  //not yet been played.
  std::string mapHash = Broodwar->mapHash();
  std::string opponentRace = Broodwar->enemy()->getRace().getName();
  std::string ownRace = Broodwar->self()->getRace().getName();

  for (int i = 0; i < (int)strategies_.size(); i++) {
    bool found = false;
    for (int s = 0; s < (int)stats_.size(); s++) {
      if (strategies_[i].strat_id_ == stats_[s].strat_id_ && mapHash == stats_[s].map_hash_ && opponentRace == stats_[s].opponent_race_) {
        //Matches
        found = true;
        break;
      }
    }

    if (not found) {
      //Only fill in the strategies for
      //the same race
      if (ownRace == strategies_[i].race_.getName()) {
        //Add entry
        StrategyStats s = StrategyStats();
        s.map_hash_ = mapHash;
        s.map_name_ = Broodwar->mapFileName();
        s.opponent_race_ = opponentRace;
        s.my_race_ = strategies_[i].race_.getName();
        s.strat_id_ = strategies_[i].strat_id_;

        stats_.push_back(s);
      }
    }
  }

  //Save the file
  std::string filename = get_write_filename();

  std::ofstream outFile;
  outFile.open(filename.c_str());
  if (not outFile) {
    Broodwar << "Error writing to stats file!" << std::endl;
  }
  else {
    for (int i = 0; i < (int)stats_.size(); i++) {
      std::stringstream s2;
      s2 << stats_[i].strat_id_;
      s2 << ";";
      s2 << stats_[i].my_race_;
      s2 << ";";
      s2 << stats_[i].opponent_race_;
      s2 << ";";
      s2 << stats_[i].won_;
      s2 << ";";
      s2 << stats_[i].lost_;
      s2 << ";";
      s2 << stats_[i].draw_;
      s2 << ";";
      s2 << stats_[i].total_;
      s2 << ";";
      s2 << stats_[i].map_name_;
      s2 << ";";
      s2 << stats_[i].map_hash_;
      s2 << ";\n";

      outFile << s2.str();
    }
    outFile.close();
  }
}
