#pragma once

#include "../MainAgents/BaseAgent.h"
//#include <windows.h>

/** Helper class for Profiler. This class represents a profiling of one specific
 * codeblock. Profiling can be done on any number of codeblocks.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class ProfilerObj {

private:
  double PCFreq;
  __int64 CounterStart;
  __int64 CounterEnd;

  std::string id;
  double maxTime;
  double total;
  int startCalls;
  int endCalls;
  int lastShowFrame;

  int timeouts_short;
  int timeouts_medium;
  int timeouts_long;

public:
  // Constructor 
  ProfilerObj(std::string mId);

  // Destructor 
  ~ProfilerObj();

  // Checks if this object matches the specified id string. 
  bool matches(std::string mId);

  // Starts measuring a codeblock. 
  void start();

  // Stops measuring a codeblock. 
  void end();

  // Print data to the ingame chat window. 
  void show();

  // Returns the html string for this profiling object. 
  std::string getDumpStr();
};

