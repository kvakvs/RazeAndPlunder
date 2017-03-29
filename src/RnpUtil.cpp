#include "RnpUtil.h"

using namespace BWAPI;

namespace rnp {

TilePosition get_center(const BWEM::Area& a) {
  auto a_bbox = a.BoundingBoxSize();
  return a.TopLeft() + TilePosition(a_bbox.x / 2, a_bbox.y / 2);
}

const BWEM::ChokePoint* get_nearest_chokepoint(const BWEM::Map& bwem, const TilePosition& tpos) {
  WalkPosition tpos1(tpos);
  auto area = bwem.GetNearestArea(tpos1);

  const BWEM::ChokePoint* result = nullptr;
  float result_dist = 1.0e+12f;

  for (auto cp : area->ChokePoints()) {
    auto cp_dist = rnp::distance(cp->Center(), tpos1);
    if (cp_dist < result_dist) {
      result_dist = cp_dist;
      result = cp;
    }
  }
  return result;
}

bool is_inside(const BWEM::Area& area, const BWAPI::Position& pos) {
  auto& bwem = BWEM::Map::Instance();
  return bwem.GetNearestArea(TilePosition(pos))->Id() == area.Id();
}

bool is_my_unit(BWAPI::Unit unit) {
  return unit->getPlayer()->getID() == Broodwar->self()->getID();
}

void DelayCounter::start(int frames) {
  trigger_after_frame_ = BWAPI::Broodwar->getFrameCount() + frames;
}

bool DelayCounter::is_ready() {
  auto now = BWAPI::Broodwar->getFrameCount();
  if (now >= trigger_after_frame_) {
    trigger_after_frame_ = -1;
    return true;
  }
  return false;
}

} // ns rnp
