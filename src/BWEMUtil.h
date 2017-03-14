#pragma once
#include "bwem.h"

namespace rnp {

  BWAPI::TilePosition get_center(const BWEM::Area& a);
  inline BWAPI::TilePosition get_center(const BWEM::Area* a) { return get_center(*a); }

  BWEM::ChokePoint* get_nearest_chokepoint(const BWEM::Map& bwem, const BWAPI::TilePosition& tpos);

  using BaseVec = std::vector < const BWEM::Base* > ;
  BaseVec get_bases(const BWEM::Map& bwem);
  
} // ns rnp
