#pragma once

#include "BWAPI/Position.h"
#include "BWEM/gridMap.h"

namespace rnp {

// Calculated chokepoint path from point A to point B
// With iterator the path is subdivided into more detailed steps and they
// can be taken one by one from the iterator
class PathAB {
  // Chokepoint path is very low resolution path of intermediate chokepoints to pass
  BWEM::CPPath cp_path_;

  int length_px_ = 0;

  const BWEM::Area* begin_area_ = nullptr;
  const BWEM::Area* end_area_ = nullptr;

  BWAPI::TilePosition begin_;
  BWAPI::TilePosition dest_;

public:
  using Ptr = std::unique_ptr < PathAB >;

  // Iterator slowly populates steps_ with detailed move coords as it advances
  class Iterator {
  public:
    using Ptr = std::unique_ptr < Iterator > ;

    PathAB& path_;
    std::vector<BWAPI::TilePosition> steps_;
    // last returned position, for future needs
    BWAPI::TilePosition current_;

    // Position in subdivided step-by-step path. 
    size_t index_ = 0;

    // Index in cp_path_ of the parent. If we need more nodes to iterate through,
    // we then take another next node from the parent cp_path_, subdivide it into 
    // nice pieces and follow it.
    size_t cp_index_ = 0;

    explicit Iterator(PathAB& p);

    // Step forward in the steps_ array, populate more steps as needed
    BWAPI::TilePosition next();

    // True if last path item was given out and there is no more
    bool is_finished() const {
      return cp_index_ >= path_.cp_path_.size() 
        && index_ >= steps_.size(); 
    }

  private:
    bool make_more_steps();

    void make_path_segment(const BWAPI::TilePosition& a, const BWAPI::TilePosition& b);
  };

public:
  PathAB(const BWAPI::TilePosition& a, const BWAPI::TilePosition& b);
  PathAB(const PathAB& other) = delete;
  PathAB& operator = (const PathAB& other) const = delete;

  bool is_good() const;

  int length() const { return length_px_; }

  // Returns cppath length + begin + end (if not same area)
  int count_steps() const;

  Iterator::Ptr begin() { return std::make_unique<Iterator>(*this); }
};

} // ns rnp
