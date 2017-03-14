#include "BWEMUtil.h"

namespace rnp {

  BWAPI::TilePosition get_center(const BWEM::Area& a) {
    return BWAPI::TilePosition(a.TopLeft() + a.BoundingBoxSize() * 0.5f);
  }

  const BWEM::ChokePoint* get_nearest_chokepoint(const BWEM::Map& bwem, const BWAPI::TilePosition& tpos) {
    BWAPI::WalkPosition tpos1(tpos);
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

//  BaseVec get_bases(const BWEM::Map& bwem) {
//    BaseVec result(16);
//    for (auto& a : bwem.Areas()) {
//      for (auto& b : a.Bases()) {
//        result.push_back(&b);
//      }
//    }
//    return result;
//  }

  bool is_inside(const BWEM::Area& area, const BWAPI::Position& pos) {
    auto tl = area.TopLeft();
    auto br = area.BottomRight();
    // TODO: Make a proper area check, not a box check
    return 
      pos.x >= tl.x && pos.x <= br.x &&
      pos.y >= tl.y && pos.y <= br.y;
  }

} // ns rnp
