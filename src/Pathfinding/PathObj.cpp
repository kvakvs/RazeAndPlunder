#if 1
#include "PathObj.h"
#include "BWEM/gridMap.h"

using namespace BWAPI;

PathObj::PathObj(TilePosition cStart, TilePosition cEnd) {
  start_ = cStart;
  end_ = cEnd;
  finished_ = false;
}

PathObj::~PathObj() {

}

bool PathObj::matches(TilePosition cStart, TilePosition cEnd) {
  //Check if end almost matches
  double dist = cEnd.getDistance(end_);
  if (dist > 3) return false;

  //Check if start almost matches.
  dist = cStart.getDistance(start_);
  if (dist > 5) return false;
  return true;
}

bool PathObj::is_finished() {
  return finished_;
}

void PathObj::calculate_path() {
  auto& bwem = BWEM::Map::Instance();
  auto pstart = BWAPI::Position(start_);
  auto pend = BWAPI::Position(end_);
  // TODO: Assert/check if bwem.GetArea(a) and (b) are not NULL (see bwem.GetPath comment)

  path_ = bwem.GetPath(pstart, pend);
  finished_ = true;
}

const BWEM::CPPath& PathObj::get_path() {
  return path_;
}
#endif //0