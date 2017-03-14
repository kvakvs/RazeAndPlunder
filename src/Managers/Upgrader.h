#pragma once

#include "../MainAgents/BaseAgent.h"

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

private:
  static Upgrader* instance;

  Upgrader();

  bool debug;

  std::vector<BWAPI::UpgradeType> upgrades;
  std::vector<BWAPI::TechType> techs;

  bool canUpgrade(BWAPI::UpgradeType type, BWAPI::Unit unit);
  bool canResearch(BWAPI::TechType type, BWAPI::Unit unit);
  std::string format(std::string str);

public:
  // Destructor. 
  ~Upgrader();

  // Returns the instance to the Upgrader that is currently used. 
  static Upgrader* getInstance();

  // Checks if there is an upgrade the specified agent need to upgrade/research. 
  bool checkUpgrade(BaseAgent* agent);

  // Switch on/off debug info printing to screen. 
  void toggleDebug();

  // Prints debug info to screen. 
  void printInfo();

  // Adds an upgrade to the upgrade queue. 
  void addUpgrade(BWAPI::UpgradeType type);

  // Adds a tech to the tech queue. 
  void addTech(BWAPI::TechType tech);
};
