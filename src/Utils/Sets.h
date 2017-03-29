#pragma once

#include <BWAPI.h>
#include <BWAPI/SetContainer.h>
#include "MainAgents/BaseAgent.h"
#include <memory>

/** Author: Johan Hagelback (johan.hagelback@gmail.com)
 */

class BaseLocation {
public:
  using Ptr = std::unique_ptr< BaseLocation >;
  struct Hasher {
    size_t operator () (const Ptr &rgn) const {
      return std::hash<int>()(rgn->base_location_.x)
        ^ std::hash<int>()(rgn->base_location_.y);
    }
  };
  using Set = std::unordered_set < Ptr >;

  explicit BaseLocation(BWAPI::TilePosition pos) : base_location_(pos) {
    frame_visited_ = BWAPI::Broodwar->getFrameCount();
  };

  BWAPI::TilePosition base_location_;
  int frame_visited_ = 0;
};
