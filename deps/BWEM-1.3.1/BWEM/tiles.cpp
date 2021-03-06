//////////////////////////////////////////////////////////////////////////
//
// This file is part of the BWEM Library.
// BWEM is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, 2016, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////


#include "BWEM/tiles.h"
#include "BWEM/neutral.h"
#include "BWEM/bwapiExt.h"
#include "winutils.h"


using namespace BWAPI;
using namespace BWAPI::UnitTypes::Enum;

namespace { auto& bw = Broodwar; }

using namespace std;


namespace BWEM {

  using namespace detail;
  using namespace BWAPI_ext;

  namespace { enum {sizeof_Tile = sizeof(Tile), sizeof_MiniTile = sizeof(MiniTile)}; }

  //////////////////////////////////////////////////////////////////////////////////////////////
  //                                                                                          //
  //                                  class MiniTile
  //                                                                                          //
  //////////////////////////////////////////////////////////////////////////////////////////////

  const Area::id MiniTile::blockingCP = std::numeric_limits<Area::id>::min();


  //////////////////////////////////////////////////////////////////////////////////////////////
  //                                                                                          //
  //                                  class Tile
  //                                                                                          //
  //////////////////////////////////////////////////////////////////////////////////////////////


  int Tile::StackedNeutrals() const {
    int stackSize = 0;
    for (Neutral* pStacked = GetNeutral(); pStacked; pStacked = pStacked->NextStacked())
      ++stackSize;

    return stackSize;
  };


} // namespace BWEM
