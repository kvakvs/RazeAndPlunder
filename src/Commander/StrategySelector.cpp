#include "StrategySelector.h"
#include "Protoss/ProtossMain.h"
#include "Terran/TerranMain.h"
#include "Zerg/LurkerRush.h"
#include "Zerg/ZergMain.h"
#include <fstream>

using namespace BWAPI;

StrategySelector* StrategySelector::instance = nullptr;

bool StrategyStats::matches() {
  std::string mMapHash = Broodwar->mapHash();
  std::string mOwnRace = Broodwar->self()->getRace().getName();
  if (mMapHash == mapHash && mOwnRace == ownRace) {
    Race oRace = Broodwar->enemy()->getRace();
    if (oRace.getID() != Races::Unknown.getID()) {
      //Opponent race is known. Match race as well.
      if (oRace.getName() == opponentRace) {
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
  active = true;

  strategies.push_back(Strategy(Races::Protoss, ProtossMain::getStrategyId()));
  strategies.push_back(Strategy(Races::Terran, TerranMain::getStrategyId()));
  strategies.push_back(Strategy(Races::Zerg, LurkerRush::getStrategyId()));
  strategies.push_back(Strategy(Races::Zerg, ZergMain::getStrategyId()));

  loadStats();
}

StrategySelector* StrategySelector::getInstance() {
  if (instance == nullptr) {
    instance = new StrategySelector();
  }
  return instance;
}

StrategySelector::~StrategySelector() {
  instance = nullptr;
}

void StrategySelector::enable() {
  active = true;
}

void StrategySelector::disable() {
  active = false;
}

void StrategySelector::selectStrategy() {
  int totWon = 0;
  int totPlay = 0;
  for (int i = 0; i < (int)stats.size(); i++) {
    std::string mOwnRace = Broodwar->self()->getRace().getName();

    if (stats.at(i).matches()) {
      totWon += stats.at(i).won;
      totPlay += stats.at(i).total;
    }
  }
  if (totPlay == 0) totPlay = 1; //To avoid division by zero

  //Random probability select one strategy
  bool found = false;
  int i = 0;
  while (!found) {
    i = rand() % (int)stats.size();

    //Entry matches
    if (stats.at(i).matches()) {
      //Calculate probability for this entry.
      int chance = stats.at(i).won * 100 / stats.at(i).getTotal();
      chance = chance * totWon / totPlay;

      //Have 75% chance to try a strategy that
      //hasn't been tested much yet.
      if (stats.at(i).total <= 2) chance = 75;

      //Set a max/min so all strategies have a chance
      //to be played.
      if (chance < 15) chance = 15;
      if (chance > 85) chance = 85;

      //Make the roll!
      int roll = rand() % 100;
      if (roll <= chance) {
        currentStrategyId = stats.at(i).strategyId;
        Broodwar << "Strategy selected: " << currentStrategyId << " (Roll: " << roll << " Prob: " << chance << ")" << std::endl;
        found = true;
        return;
      }
    }
  }
}

Commander::Ptr StrategySelector::getStrategy() {
  int tot = 0;
  for (int i = 0; i < (int)stats.size(); i++) {
    if (stats.at(i).matches()) tot++;
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
    if (self_race == Races::Terran.getID()) currentStrategyId = "TerranMain";
    else if (self_race == Races::Protoss.getID()) currentStrategyId = "ProtossMain";
    else if (self_race == Races::Zerg.getID()) currentStrategyId = "LurkerRush";
  }
  Broodwar << "Strategy: " << currentStrategyId << std::endl;

  //Get Commander for strategy
  if (currentStrategyId == "ProtossMain") return std::make_shared<ProtossMain>();
  if (currentStrategyId == "TerranMain") return std::make_shared<TerranMain>();
  if (currentStrategyId == "LurkerRush") return std::make_shared<LurkerRush>();
  if (currentStrategyId == "ZergMain") return std::make_shared<ZergMain>();

  bwem_assert(false);
  return nullptr;
}

void StrategySelector::printInfo() {
  Broodwar->drawTextScreen(180, 5, "\x0F%s", currentStrategyId.c_str());
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
    while (!inFile.eof()) {
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
  s.strategyId = t;
  line = line.substr(i + 1, line.length());

  i = line.find(";");
  t = line.substr(0, i);
  s.ownRace = t;
  line = line.substr(i + 1, line.length());

  i = line.find(";");
  t = line.substr(0, i);
  s.opponentRace = t;
  line = line.substr(i + 1, line.length());

  i = line.find(";");
  t = line.substr(0, i);
  s.won = toInt(t);
  line = line.substr(i + 1, line.length());

  i = line.find(";");
  t = line.substr(0, i);
  s.lost = toInt(t);
  line = line.substr(i + 1, line.length());

  i = line.find(";");
  t = line.substr(0, i);
  s.draw = toInt(t);
  line = line.substr(i + 1, line.length());

  i = line.find(";");
  t = line.substr(0, i);
  s.total = toInt(t);
  line = line.substr(i + 1, line.length());

  i = line.find(";");
  t = line.substr(0, i);
  s.mapName = t;
  line = line.substr(i + 1, line.length());

  i = line.find(";");
  t = line.substr(0, i);
  s.mapHash = t;
  line = line.substr(i + 1, line.length());

  stats.push_back(s);
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
  if (not active) return;

  std::string opponentRace = Broodwar->enemy()->getRace().getName();
  std::string mapHash = Broodwar->mapHash();

  //Check if we have the entry already
  for (int i = 0; i < (int)stats.size(); i++) {
    if (mapHash == stats.at(i).mapHash && opponentRace == stats.at(i).opponentRace && currentStrategyId == stats.at(i).strategyId) {
      stats.at(i).total++;
      if (win == 0) stats.at(i).lost++;
      if (win == 1) stats.at(i).won++;
      if (win == 2) stats.at(i).draw++;
      return;
    }
  }

  StrategyStats s = StrategyStats();
  s.total++;
  if (win == 0) s.lost++;
  if (win == 1) s.won++;
  if (win == 2) s.draw++;
  s.strategyId = currentStrategyId;
  s.mapHash = mapHash;
  s.mapName = Broodwar->mapFileName();
  s.ownRace = Broodwar->self()->getRace().getName();
  s.opponentRace = opponentRace;
  stats.push_back(s);
}

void StrategySelector::saveStats() {
  if (not active) return;

  //Fill entries in stats file for combinations that have
  //not yet been played.
  std::string mapHash = Broodwar->mapHash();
  std::string opponentRace = Broodwar->enemy()->getRace().getName();
  std::string ownRace = Broodwar->self()->getRace().getName();

  for (int i = 0; i < (int)strategies.size(); i++) {
    bool found = false;
    for (int s = 0; s < (int)stats.size(); s++) {
      if (strategies.at(i).strategyId == stats.at(s).strategyId && mapHash == stats.at(s).mapHash && opponentRace == stats.at(s).opponentRace) {
        //Matches
        found = true;
        break;
      }
    }

    if (not found) {
      //Only fill in the strategies for
      //the same race
      if (ownRace == strategies.at(i).race.getName()) {
        //Add entry
        StrategyStats s = StrategyStats();
        s.mapHash = mapHash;
        s.mapName = Broodwar->mapFileName();
        s.opponentRace = opponentRace;
        s.ownRace = strategies.at(i).race.getName();
        s.strategyId = strategies.at(i).strategyId;

        stats.push_back(s);
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
    for (int i = 0; i < (int)stats.size(); i++) {
      std::stringstream s2;
      s2 << stats.at(i).strategyId;
      s2 << ";";
      s2 << stats.at(i).ownRace;
      s2 << ";";
      s2 << stats.at(i).opponentRace;
      s2 << ";";
      s2 << stats.at(i).won;
      s2 << ";";
      s2 << stats.at(i).lost;
      s2 << ";";
      s2 << stats.at(i).draw;
      s2 << ";";
      s2 << stats.at(i).total;
      s2 << ";";
      s2 << stats.at(i).mapName;
      s2 << ";";
      s2 << stats.at(i).mapHash;
      s2 << ";\n";

      outFile << s2.str();
    }
    outFile.close();
  }
}
