#include "PathAB.h"
#include "RnpUtil.h"

using namespace BWAPI;

namespace rnp {

PathAB::Iterator::Iterator(PathAB& p): path_(p) {
  // We plan paths via middle and last point for each segment, so 2x points
  // also first point is pushed right away
  steps_.reserve(p.cp_path_.size() * 2 + 1);
  steps_.push_back(p.begin_);
}

TilePosition PathAB::Iterator::next() {
  // Have enough premade steps, just take another one
  if (index_ < steps_.size()) {
    return current_ = steps_[index_++];
  }

  if (make_more_steps()) {
    RNP_ASSERT(index_ < steps_.size());
    return current_ = steps_[index_++];
  }

  return current_ = path_.dest_;
}

bool PathAB::Iterator::make_more_steps() {
  if (cp_index_ >= path_.cp_path_.size()) {
    return false;
  }
  // Begin either from first position of PathAB, or from the
  // previous chokepoint
  TilePosition seg_begin;
  if (cp_index_ == 0) {
    seg_begin = path_.begin_;
  }
  else {
    seg_begin = TilePosition(path_.cp_path_[cp_index_ - 1]->Center());
  }
      
  auto seg_end_cp = path_.cp_path_[cp_index_++];
  auto seg_end = TilePosition(seg_end_cp->Center());

  make_path_segment(seg_begin, seg_end);
  return true;
}

void PathAB::Iterator::make_path_segment(const TilePosition& a, 
                                         const TilePosition& b) {
  // Naive implementation, place middle and end points
  steps_.push_back(middle_point(a, b));
  steps_.push_back(b);
}

PathAB::PathAB(const TilePosition& a, const TilePosition& b)
: begin_(a), dest_(b) {
  auto& bwem = BWEM::Map::Instance();

  auto a1 = Position(rnp::clamp_to_map(bwem, a));
  auto b1 = Position(rnp::clamp_to_map(bwem, b));
//  auto a1 = Position(a);
//  auto b1 = Position(b);

  cp_path_ = bwem.GetPath(a1, b1, &length_px_);
  begin_area_ = bwem.GetNearestArea(WalkPosition(a1));
  end_area_ = bwem.GetNearestArea(WalkPosition(b1));
}

bool PathAB::is_good() const {
  if (not begin_area_ || not end_area_) {
    return false;
  }
  // Path exists if there are any chokepoints found, or if areas for begin
  // and end match
  return not cp_path_.empty() || begin_area_ == end_area_;
}

int PathAB::count_steps() const {
  return cp_path_.size() + (begin_area_ == end_area_ ? 1 : 2);
}

} // ns rnp
