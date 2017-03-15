#pragma once

#include "Commander/Commander.h"
#include <BWAPI.h>


/** This class saves results (winner, building score, unit score, kill score) from
 * played games. The results are stored to a semicolon-separated csv file in the bwapi-data/AI/
 * folder. Note that fog-of-war must be disabled to see opponent score.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class Statistics {
  std::string getFilename();
  bool active;

public:
  Statistics();
  ~Statistics();

  // Saves result from a game to file. 
  void saveResult(int win);

  // Enable statistics. 
  void enable();

  // Disable statistics. 
  void disable();
};
