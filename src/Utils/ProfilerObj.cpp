#include "ProfilerObj.h"
#include <windows.h>
#include <profileapi.h>

using namespace BWAPI;

ProfilerObj::ProfilerObj(const std::string& mId): id_(mId) {
}

ProfilerObj::~ProfilerObj() {
}

bool ProfilerObj::matches(const std::string& mId) {
  return mId == id_;
}

void ProfilerObj::start() {
  LARGE_INTEGER li;
  QueryPerformanceFrequency(&li);

  pc_freq_ = double(li.QuadPart) / 1000.0;

  QueryPerformanceCounter(&li);
  counter_start_ = li.QuadPart;

  start_calls_++;
}

void ProfilerObj::end() {
  LARGE_INTEGER li;
  QueryPerformanceCounter(&li);
  double elapsed = (li.QuadPart - counter_start_) / pc_freq_;

  total_ += elapsed;

  end_calls_++;

  if (elapsed >= 10000.0) timeouts_long_++;
  if (elapsed >= 1000.0) timeouts_medium_++;
  if (elapsed >= 85.0) timeouts_short_++;
  if (elapsed > max_time_) max_time_ = elapsed;
}

void ProfilerObj::show() {
  if (Broodwar->getFrameCount() - last_show_frame_ < 400) return;

  last_show_frame_ = Broodwar->getFrameCount();

  double avg = (double)total_ / (double)end_calls_;

  Broodwar << id_ << ": AvgFrame: " << (int)avg << " MaxFrame: " << max_time_
           << " TO_10k: " << timeouts_long_ << " TO_1k: " << timeouts_medium_
           << " TO_85ms: " << timeouts_short_ << std::endl;
  if (timeouts_long_ >= 1 || timeouts_medium_ >= 10 || timeouts_short_ >= 320) {
    Broodwar << id_ << ": Timeout fail!!!" << std::endl;
  }
  if (start_calls_ != end_calls_) {
    Broodwar << id_ << ": Warning! Start- and endcalls mismatch " << start_calls_ << "/" << end_calls_ << std::endl;
  }
}

std::string ProfilerObj::getDumpStr() {
  double avg = total_ / (double)end_calls_;

  std::stringstream ss;

  ss << "<tr><td>";
  ss << id_;
  ss << "</td><td>";
  ss << avg;
  ss << "</td><td>";
  ss << total_;
  ss << "</td><td>";
  if (start_calls_ == end_calls_) {
    ss << end_calls_;
  }
  else {
    ss << "Calls missmatch (";
    ss << start_calls_;
    ss << "/";
    ss << end_calls_;
    ss << ")";
  }
  ss << "</td><td>";
  ss << max_time_;
  ss << "</td><td>";
  ss << timeouts_long_;
  ss << "</td><td>";
  ss << timeouts_medium_;
  ss << "</td><td>";
  ss << timeouts_short_;
  ss << "</td>";
  ss << "</tr>\n";

  return ss.str();
}
