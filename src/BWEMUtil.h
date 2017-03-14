#pragma once
#include "bwem.h"

namespace rnp {

  BWAPI::TilePosition get_center(const BWEM::Area& a);
  inline BWAPI::TilePosition get_center(const BWEM::Area* a) { return get_center(*a); }

  const BWEM::ChokePoint* 
    get_nearest_chokepoint(const BWEM::Map& bwem, const BWAPI::TilePosition& tpos);

//  using BaseVec = std::vector < const BWEM::Base* > ;
//  BaseVec get_bases(const BWEM::Map& bwem);

  bool is_inside(const BWEM::Area& area, const BWAPI::Position& pos);

  // TODO: this
  inline int choke_width(const BWEM::ChokePoint *) { return 4 * 32; }
  
} // ns rnp
