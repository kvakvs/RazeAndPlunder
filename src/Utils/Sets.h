#pragma once

#include <BWAPI.h>
#include <BWAPI/SetContainer.h>
#include "../MainAgents/BaseAgent.h"

using namespace BWAPI;

/** Author: Johan Hagelback (johan.hagelback@gmail.com)
 */

class BaseLocationItem {
public:
  explicit BaseLocationItem(TilePosition pos) {
    baseLocation = pos;
    frameVisited = Broodwar->getFrameCount();
  };

  TilePosition baseLocation;
  int frameVisited;
};

class BaseLocationSet : public SetContainer<BaseLocationItem*, std::hash<void*>> {
public:

};

class Agentset : public SetContainer<BaseAgent*, std::hash<void*>> {
public:

};
