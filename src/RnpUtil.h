#pragma once
#include "BWEM/bwem.h"
#include <assert.h>

#define RNP_ASSERT(X) assert(X)

namespace rnp {

  BWAPI::TilePosition get_center(const BWEM::Area& a);

  inline BWAPI::TilePosition get_center(const BWEM::Area* a) {
    return get_center(*a);
  }

  const BWEM::ChokePoint* 
    get_nearest_chokepoint(const BWEM::Map& bwem,
                           const BWAPI::TilePosition& tpos);

  bool is_inside(const BWEM::Area& area, const BWAPI::Position& pos);

  // TODO: this
  inline int choke_width(const BWEM::ChokePoint *) { return 4 * 32; }

  // With calculated chokepoint path takes distance from start to the 
  // first node of the path, then chokepoint path length, and then adds
  // distance to the end position
  int tile_distance(const BWEM::CPPath& p,
    const BWAPI::TilePosition& start, const BWAPI::TilePosition& end);

  template <class Pos>
  bool is_valid_position(const Pos& p) {
    return p.x != -1 || p.y != -1;
  }

} // ns rnp
