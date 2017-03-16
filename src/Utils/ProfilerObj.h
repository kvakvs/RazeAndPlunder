#pragma once

#include "MainAgents/BaseAgent.h"
#include <memory>

/** Helper class for Profiler. This class represents a profiling of one specific
 * codeblock. Profiling can be done on any number of codeblocks.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class ProfilerObj {
public:
  using Ptr = std::unique_ptr<ProfilerObj>;

private:
  double pc_freq_ = 0.0;
  __int64 counter_start_ = 0;
  __int64 counter_end_ = 0;

  std::string id_;
  double max_time_ = 0.0;
  double total_ = 0.0;
  int start_calls_ = 0;
  int end_calls_ = 0;
  int last_show_frame_ = 0;

  int timeouts_short_ = 0;
  int timeouts_medium_ = 0;
  int timeouts_long_ = 0;

public:
  ProfilerObj(const std::string& mId);
  ~ProfilerObj();

  // Checks if this object matches the specified id string. 
  bool matches(const std::string& mId);

  // Starts measuring a codeblock. 
  void start();

  // Stops measuring a codeblock. 
  void end();

  // Print data to the ingame chat window. 
  void show();

  // Returns the html string for this profiling object. 
  std::string getDumpStr();
};
