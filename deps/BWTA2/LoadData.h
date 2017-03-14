#pragma once

#include "RegionImpl.h"

namespace BWTA {
  void loadMapFromBWAPI();
  void loadMap();
  void load_data(std::string filename);
  void save_data(std::string filename);
}
