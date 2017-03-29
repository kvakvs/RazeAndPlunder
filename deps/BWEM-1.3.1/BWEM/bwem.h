//////////////////////////////////////////////////////////////////////////
//
// This file is part of the BWEM Library.
// BWEM is free software, licensed under the MIT/X11 License. 
// A copy of the license is provided with the library in the LICENSE file.
// Copyright (c) 2015, 2016, Igor Dimitrijevic
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "BWEM/map.h"
#include "BWEM/tiles.h"
#include "BWEM/area.h"
#include "BWEM/cp.h"
#include "BWEM/base.h"
#include "BWEM/neutral.h"
#include "BWEM/gridMap.h"
#include "BWEM/examples.h"
#include "BWEM/mapPrinter.h"
#include "BWEM/mapDrawer.h"
#include "BWEM/utils.h"
#include "BWEM/bwapiExt.h"
#include "BWEM/defs.h"

/*


To see examples of how to use the BWEM library, have a look at the functions declared in examples.h.
All the examples are runable, just follow the steps in the "Getting started" page of the html documentation.

To get into the documentation of the API, just read the comments in the following files (preferably in this order):
	tiles.h
	map.h
	area.h
	cp.h
	base.h
	neutral.h


Many of the algorithms used in the analysis are parametrised and thus can be easily modified:

- To change the way the Areas or created (make them smaller or greater), look at the "Condition for the neighboring areas to merge"
  in MapImpl::ComputeTempAreas.
  Also look at the constant area_min_miniTiles defined in defs.h.

- To change the threshold between Seas and Lakes, look at the constants defined in defs.h.

- To change the shape of the ChokePoints, just modify the helper function chooseNeighboringArea.

- To change the way the Bases are located, look at the constants defined in defs.h.

- To change the way the StartingLocations are assigned to Bases, look at MapImpl::FindBasesForStartingLocations
  and the constant max_tiles_between_StartingLocation_and_its_AssignedBase defined in defs.h.


If you are interested in some or all the processes of the analysis, you sould start at MapImpl::Initialize(),
in which sub-processes are called in sequentially steps.


*/


namespace BWEM {


} // namespace BWEM
