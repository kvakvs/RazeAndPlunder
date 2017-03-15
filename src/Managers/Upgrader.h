#pragma once

#include "MainAgents/BaseAgent.h"

/** Upgrader handles upgrades and techs. Add the requested upgrade and/or tech
 * to the upgrader and the bot executes it when enough resources are available.
 *
 * The Upgrader is implemented as a singleton class. Each class that needs to
 * access Upgrader can request an instance, and all classes shares the same
 * Upgrader instance.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class Upgrader {
  bool debug_;
  std::vector<BWAPI::UpgradeType> upgrades_;
  std::vector<BWAPI::TechType> techs_;

private:
  bool canUpgrade(BWAPI::UpgradeType type, BWAPI::Unit unit);
  bool canResearch(BWAPI::TechType type, BWAPI::Unit unit) const;
  std::string format(std::string str) const;

public:
  Upgrader();
  ~Upgrader();

  // Checks if there is an upgrade the specified agent need to upgrade/research. 
  bool checkUpgrade(BaseAgent* agent);

  // Switch on/off debug info printing to screen. 
  void toggleDebug();

  // Prints debug info to screen. 
  void printInfo() const;

  // Adds an upgrade to the upgrade queue. 
  void addUpgrade(BWAPI::UpgradeType type);

  // Adds a tech to the tech queue. 
  void addTech(BWAPI::TechType tech);
};
