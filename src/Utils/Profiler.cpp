#include "Profiler.h"
#include <fstream>
#include <iso646.h>

Profiler::Profiler() {
}

Profiler::~Profiler() {
}

ProfilerObj* Profiler::get_profiler_obj(const std::string& mId) {
  auto iter = profile_objects_.find(mId);
  if (iter == profile_objects_.end()) {
    return nullptr;
  }
  return iter->second.get();
}


void Profiler::start(const std::string& mId) {
  if (not active_) return;

  ProfilerObj* cObj = get_profiler_obj(mId);
  if (cObj != nullptr) {
    cObj->start();
  }
  else {
    auto newObj = std::make_unique<ProfilerObj>(mId);
    newObj->start();
    profile_objects_[mId] = std::move(newObj);
  }
}

void Profiler::end(const std::string& mId) {
  if (not active_) return;

  ProfilerObj* cObj = get_profiler_obj(mId);
  if (cObj != nullptr) cObj->end();
}

void Profiler::dump_to_file() {
  if (not active_) return;

  std::ofstream ofile;
  ofile.open("bwapi-data\\AI\\Profiling_OpprimoBot.html");

  ofile << "<html><head>\n";
  ofile << "<style type='text/css'>\n";
  ofile << "body{ background-color: #999999;}\n";
  ofile << "table { background-color: white; border: 1px solid; width: 800;}\n";
  ofile << "tr {border: 1px solid;}\n";
  ofile << "td.h {background-color: #ccccff; text-align: center;}\n";
  ofile << "td {background-color: white; text-align: center;}\n";
  ofile << "</style>\n";
  ofile << "<title>OpprimoBot Profiler</title></head><body>\n";
  ofile << "<table>";

  ofile << "<tr><td class='h'>";
  ofile << "Id";
  ofile << "</td><td class='h'>";
  ofile << "AvgCallTime";
  ofile << "</td><td class='h'>";
  ofile << "TotalTime";
  ofile << "</td><td class='h'>";
  ofile << "Calls";
  ofile << "</td><td class='h'>";
  ofile << "MaxTime";
  ofile << "</td><td class='h'>";
  ofile << "TO_10sec";
  ofile << "</td><td class='h'>";
  ofile << "TO_1sec";
  ofile << "</td><td class='h'>";
  ofile << "TO_55ms";
  ofile << "</td>";
  ofile << "</tr>\n";

  for (auto& o_pair : profile_objects_) {
    ofile << o_pair.second->getDumpStr().c_str();
  }

  ofile << "</table>";
  ofile << "</body></html>";

  ofile.close();
}
