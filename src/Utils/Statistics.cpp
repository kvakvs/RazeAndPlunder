#include "Statistics.h"
#include <fstream>
#include "Glob.h"
#include <iso646.h>

using namespace BWAPI;

Statistics::Statistics() {
  active = true;
}

Statistics::~Statistics() {
}

void Statistics::enable() {
  active = true;
}

void Statistics::disable() {
  active = false;
}

std::string Statistics::getFilename() {
  std::stringstream ss;
  ss << "bwapi-data\\AI\\";
  //ss << "bwapi-data\\write\\"; //Tournament persistent storage version
  ss << "Statistics_OpprimoBot.csv";

  return ss.str();
}

void Statistics::save_result(rnp::MatchResult win) {
  if (not active) return;

  std::stringstream ss;
  ss << Broodwar->self()->getRace().getName();
  ss << ";";
  ss << Broodwar->enemy()->getRace().getName();
  ss << ";";
  ss << Broodwar->mapFileName();
  ss << ";";
  switch (win) {
  case rnp::MatchResult::Win: ss << "Won"; break;
  case rnp::MatchResult::Loss: ss << "Lost"; break;
  case rnp::MatchResult::Draw: ss << "Draw"; break;
  }
  ss << ";";
  ss << Broodwar->self()->getUnitScore();
  ss << ";";
  ss << Broodwar->self()->getBuildingScore();
  ss << ";";
  ss << Broodwar->self()->getKillScore();
  ss << ";";
  ss << Broodwar->enemy()->getUnitScore();
  ss << ";";
  ss << Broodwar->enemy()->getBuildingScore();
  ss << ";";
  ss << Broodwar->enemy()->getKillScore();
  ss << "\n";

  //Save the file
  std::string filename = getFilename();

  std::ofstream outFile;
  outFile.open(filename.c_str(), std::ios::out | std::ios::app);
  if (not outFile) {
    Broodwar << "Error writing to stats file!" << std::endl;
  }
  else {
    outFile << ss.str();
    outFile.close();
  }
}
