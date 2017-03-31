#include "MainAgents/RnpMovementMonitor.h"
#include "MainAgents/BaseAgent.h"
#include "RnpUtil.h"

using namespace BWAPI;

namespace rnp {

MovementProgressMonitor::MovementProgressMonitor()
  : last_improvement_frame_(Broodwar->getFrameCount()) {
}

void MovementProgressMonitor::reset() {
  last_improvement_frame_ = Broodwar->getFrameCount();
  best_distance_ = LIKE_VERY_FAR;
}

void MovementProgressMonitor::update(const BaseAgent* agent) {
  auto u = agent->get_unit();
  if (not u || not u->exists()) {
    return;
  }

  TilePosition pos = u->getTilePosition();
  auto& goal = agent->get_goal();
  auto dist = rnp::distance(goal, pos);
  if (dist < best_distance_) {
    best_distance_ = dist;
    last_improvement_frame_ = Broodwar->getFrameCount();
  }
}

} // ns rnp
