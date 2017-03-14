#include "BWEMUtil.h"

namespace rnp {

  BWAPI::TilePosition get_center(const BWEM::Area& a) {
    return BWAPI::TilePosition(a.TopLeft() + a.BoundingBoxSize() * 0.5f);
  }

  BWEM::ChokePoint* get_nearest_chokepoint(const BWEM::Map& bwem, const BWAPI::TilePosition& tpos) {

  }

  BaseVec get_bases(const BWEM::Map& bwem) {
    BaseVec result(16);
    for (auto& a : bwem.Areas()) {
      for (auto& b : a.Bases()) {
        result.push_back(&b);
      }
    }
    return result;
  }

} // ns rnp
