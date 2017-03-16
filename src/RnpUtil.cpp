#include "RnpUtil.h"

using namespace BWAPI;

namespace rnp {

  TilePosition get_center(const BWEM::Area& a) {
    return TilePosition(a.TopLeft() + a.BoundingBoxSize() * 0.5f);
  }

  const BWEM::ChokePoint* get_nearest_chokepoint(const BWEM::Map& bwem, const TilePosition& tpos) {
    WalkPosition tpos1(tpos);
    auto area = bwem.GetNearestArea(tpos1);
    
    const BWEM::ChokePoint* result = nullptr;
    int result_dist = 1 << 30;

    for (auto cp : area->ChokePoints()) {
      auto cp_dist = cp->Center().getApproxDistance(tpos1);
      if (cp_dist < result_dist) {
        result_dist = cp_dist;
        result = cp;
      }
    }
    return result;
  }

  bool is_inside(const BWEM::Area& area, const BWAPI::Position& pos) {
    auto tl = area.TopLeft();
    auto br = area.BottomRight();
    // TODO: Make a proper area check, not a box check
    return 
      pos.x >= tl.x && pos.x <= br.x &&
      pos.y >= tl.y && pos.y <= br.y;
  }

  int tile_distance(const BWEM::CPPath& p, 
    const TilePosition& start, const TilePosition& end) {
    if (p.empty()) {
      return 0;
    }

    auto pfirst = TilePosition(p.front()->Center());
    auto plast = TilePosition(p.back()->Center());
    int distance = pfirst.getApproxDistance(start) + plast.getApproxDistance(end);

    // for points 2 to last, sum up distances between each and previous point
    auto pprev = pfirst;
    for (auto pi = p.begin() + 1; pi != p.end(); ++pi) {
      auto pcurr = TilePosition((*pi)->Center());
      distance += pprev.getApproxDistance(pcurr);
      pprev = pcurr;
    }

    return distance;
  }

} // ns rnp
