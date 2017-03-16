#pragma once

#include <BWAPI.h>
#include <BWAPI/SetContainer.h>
#include "MainAgents/BaseAgent.h"

/** Author: Johan Hagelback (johan.hagelback@gmail.com)
 */

class BaseLocationItem {
public:
  explicit BaseLocationItem(BWAPI::TilePosition pos) : baseLocation(pos) {
    frameVisited = BWAPI::Broodwar->getFrameCount();
  };

  BWAPI::TilePosition baseLocation;
  int frameVisited = 0;
};

class BaseLocationSet : public BWAPI::SetContainer<BaseLocationItem*, std::hash<void*>> {
public:

};

class Agentset : public BWAPI::SetContainer<BaseAgent*, std::hash<void*>> {
public:

};
