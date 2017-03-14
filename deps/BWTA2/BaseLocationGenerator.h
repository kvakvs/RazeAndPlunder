#pragma once

#include <BWTA.h>


namespace BWTA {
  void detectBaseLocations(std::set<BaseLocation*>& baseLocations);
  void attachResourcePointersToBaseLocations(std::set<BaseLocation*>& baseLocations);
  void calculateBaseLocationProperties();
}
