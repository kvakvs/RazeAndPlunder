#include "PathObj.h"
#include "BWEM/gridMap.h"

using namespace BWAPI;

PathObj::PathObj(TilePosition cStart, TilePosition cEnd) {
  start = cStart;
  end = cEnd;
  finished = false;
}

PathObj::~PathObj() {

}

bool PathObj::matches(TilePosition cStart, TilePosition cEnd) {
  //Check if end almost matches
  double dist = cEnd.getDistance(end);
  if (dist > 3) return false;

  //Check if start almost matches.
  dist = cStart.getDistance(start);
  if (dist > 5) return false;
  return true;
}

bool PathObj::isFinished() {
  return finished;
}

void PathObj::calculatePath() {
  auto& bwem = BWEM::Map::Instance();
  auto pstart = BWAPI::Position(start);
  auto pend = BWAPI::Position(end);
  // TODO: Assert/check if bwem.GetArea(a) and (b) are not NULL (see bwem.GetPath comment)

  path = bwem.GetPath(pstart, pend);
  finished = true;
}

const BWEM::CPPath& PathObj::getPath() {
  return path;
}
