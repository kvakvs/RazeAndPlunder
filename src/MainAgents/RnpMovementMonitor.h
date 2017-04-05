#pragma once

#include <BWAPI.h>
#include <Actors/Actor.h>
#include "RnpConst.h"

class BaseAgent;

namespace rnp {

// Collects the metric (distance) of how close did the Agent get to its 'goal' 
// and when was the last improvement to this metric. If the last improvement
// happened too long ago, it means we are stuck and should cancel the movement
// and do something better instead
class MovementProgressMonitor {
  int last_improvement_frame_;
  float best_distance_ = LIKE_VERY_FAR;
public:
  MovementProgressMonitor();

  void reset();

  // Check with the agent_->unit location if the distance to it improved
  void update(const BaseAgent* ba);

  int get_frames_since_last_improvement() const {
    return BWAPI::Broodwar->getFrameCount() - last_improvement_frame_;
  }

  float best_dist() const { return best_distance_; }

  bool is_possibly_stuck(size_t frames, float dist) const {
    size_t last_i = get_frames_since_last_improvement();
    // True if improvement in distance was too long ago
    // AND distance is still too far (longer than 2.0 tiles)
    return last_i > frames && best_distance_ > dist;
  }
};

} // ns rnp
