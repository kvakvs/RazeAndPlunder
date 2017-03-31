#pragma once

#include "BWEM/bwem.h"
#include "BWAPI.h"
#include <functional>
#include "Actors/Actor.h"
#include "RnpConst.h"
#include "Actors/Algorithm.h"

#if RNP_DEBUG
#include <assert.h>
#define RNP_ASSERT(X) assert(X)
#else
  #define RNP_ASSERT(X) 
#endif // debug

namespace std {
template <>
struct hash<BWAPI::TilePosition> {
  size_t operator ()(const BWAPI::TilePosition& rgn) const {
    return std::hash<int>()(rgn.x) ^ std::hash<int>()(rgn.y);
  }
};
}

namespace rnp {

//
// Coordinates and distances stuff
//

BWAPI::TilePosition get_center(const BWEM::Area& a);

inline BWAPI::TilePosition get_center(const BWEM::Area* a) {
  return get_center(*a);
}

const BWEM::ChokePoint*
get_nearest_chokepoint(const BWEM::Map& bwem,
                       const BWAPI::TilePosition& tpos);

bool is_inside(const BWEM::Area& area, const BWAPI::Position& pos);

// TODO: this
int choke_width(const BWEM::ChokePoint* cp);

// With calculated chokepoint path takes distance from start to the 
// first node of the path, then chokepoint path length, and then adds
// distance to the end position
//  int tile_distance(const BWEM::CPPath& p,
//    const BWAPI::TilePosition& start, const BWAPI::TilePosition& end);

//
// Position stuff
//

template <class POS>
POS middle_point(const POS& a, const POS& b) {
  return POS((a.x + b.x) / 2, (a.y + b.y) / 2);
}

// A convenience constructor for a -1;-1 map position used in errors and stuff
inline BWAPI::TilePosition make_bad_position() {
  return BWAPI::TilePosition{-1, -1};
}

template <class POS>
bool is_valid_position(const POS& p) {
  // Tile 1000 (pixel 32000) is used to denote special error types for positions
  // like unknown. When x = 32000, y defines the special value type
  return p.x != -1 && p.x < 1000
      && p.y != -1;
}

template <class POS>
bool is_unknown_position(const POS& p) {
  return p.x == BWAPI::Positions::Unknown.x && p.y == BWAPI::Positions::Unknown.y;
}

template <class POS>
bool is_valid_map_position(const POS& p0) {
  BWAPI::TilePosition p(p0);
  return p.x >= 0 && p.x < BWAPI::Broodwar->mapWidth()
      && p.y >= 0 && p.x < BWAPI::Broodwar->mapHeight();
}

inline BWAPI::TilePosition clamp_to_map(const BWEM::Map& map, const BWAPI::TilePosition& p) {
  auto map_size = map.Size();
  int x = std::min(std::max(p.x, 0), map_size.x - 1);
  int y = std::min(std::max(p.y, 0), map_size.y - 1);

  return BWAPI::TilePosition(x, y);
}

// 1% precision approx distance based on 
// https://en.wikibooks.org/wiki/Algorithms/Distance_approximations
// A more rough integer ~8% precision is found in BWAPI (getApproxDistance)
template <class POS>
float distance(const POS& a, const POS& b) {
  decltype(POS::x) dx = std::abs(a.x - b.x);
  decltype(POS::x) dy = std::abs(a.y - b.y);
  if (dy < dx) {
    std::swap(dy, dx);
  }
  return 0.41f * float(dx) + 0.941246f * float(dy);
}

//
// Unit stuff
//

bool is_my_unit(BWAPI::Unit unit);

inline bool same_type(BWAPI::UnitType unit_type, BWAPI::UnitType isa) {
  return unit_type.getID() == isa.getID();
}

inline bool same_type(BWAPI::Unit unit, BWAPI::UnitType isa) {
  return unit->getType().getID() == isa.getID();
}

template <typename FUN>
void for_each_base(FUN fun) {
  auto& bwem = BWEM::Map::Instance();
  for (auto& area : bwem.Areas()) {
    for (auto& base : area.Bases()) {
      fun(base);
    }
  }
}


template <class ActorClass>
act::ActorId spawn_unit(BWAPI::Unit unit) {
  act::ActorId actor_id(ActorFlavour::Unit, unit->getID());
  act::spawn_with_id<ActorClass>(actor_id, unit);
  return actor_id;
}

//
// Other helper stuff
//

// For frame counters
inline int seconds(int sec) {
  return sec * 24;
}

class DelayCounter {
private:
  int trigger_after_frame_ = -1;
public:
  DelayCounter() {}

  void start(int frames);

  bool is_ready();
};

// Allows to cache results of some calculations for X frames.
// Remembers some value for X amount of frames, tells you if the value is still fresh
// and you can retrieve it or update it
template <typename Value>
class Memoize {
  Value value_;
  int last_frame_ = -1;
public:
  Memoize() : value_() {
  }

  bool value_up_to_date(int frames_ago) const {
    if (last_frame_ > 0 && BWAPI::Broodwar->getFrameCount() - last_frame_ > frames_ago) {
      return true;
    }
    return false;
  }

  const Value& value() const {
    return value_;
  }

  const Value& update(const Value& v) {
    last_frame_ = BWAPI::Broodwar->getFrameCount();
    value_ = v;
    return v;
  }
};

template<class Container, class Type>
bool contains(const Container& v, const Type& x) {
  //return std::end(v) != std::find(std::begin(v), std::end(v), x);
  return v.find(x) != v.end();
}

inline act::ActorId unit_actor_key(int unit_id) {
  return act::ActorId(ActorFlavour::Unit, unit_id);
}

} // ns rnp
