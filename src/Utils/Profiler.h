#pragma once

#include <BWAPI.h>
#include <memory>

#include "ProfilerObj.h"

/** This class is used to measure the time (in milliseconds) it takes to execute a codeblock.
 * It also counts timeouts according to the rules from the AIIDE 2011 bot competition. If one of
 * the following three conditions are true, a bot is disqualified due to timeout:
 * - 1 frame over 1 minute execution time.
 * - 10 frames over 1 second execution time.
 * - 320 frames over 55 ms execution time.
 * 
 * The profiler can measure multiple parallell codeblocks. It uses an identifier string to differ
 * between different blocks.
 *
 * After a game is ended all profiling data is stored in a html file in the bwapi-data/AI folder.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class Profiler {
private:
  std::map<std::string, ProfilerObj::Ptr> profile_objects_;
  bool active_ = true;

  ProfilerObj* getObj(const std::string& mId);

public:
  Profiler();
  ~Profiler();

  // Starts measuring. Put at beginning of a codeblock.// Make sure the startiId is the same as the end id. 
  void start(const std::string& mId);

  // Stops measuring. Put at the end of a codeblock.
  // Make sure the startiId is the same as the end id. 
  void end(const std::string& mId);

  // Stores all profiling data to file. 
  void dumpToFile();

  // Enable profiling. 
  void enable() {
    active_ = true;
  }

  // Disable profiling. 
  void disable() {
    active_ = false;
  }
};
