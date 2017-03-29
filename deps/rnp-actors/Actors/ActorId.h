#pragma once

#include <memory>
#include <vector>
#include <unordered_set>

namespace act {

class ActorId {
  union {
    uint64_t long_id_;
    struct {
      uint32_t flavour_; // use some enum to give actors various classes
      uint32_t id_;
    };
  };

public:
  // Special value for flavour, marking an invalid actor id
  static constexpr uint32_t BAD_VALUE = 0;

  struct Hasher {
    size_t operator () (const ActorId &aid) const {
      return std::hash<uint64_t>()(aid.long_id_);
    }
  };
  using Set = std::unordered_set<ActorId, Hasher>;
  using Vector = std::vector<ActorId>;

  ActorId(): flavour_(BAD_VALUE), id_(0) {
  }
  ActorId(const ActorId& other): long_id_(other.long_id_) {
  }

  template <typename Flavour>
  ActorId(Flavour flav, uint32_t id)
      : flavour_(static_cast<uint32_t>(flav))
      , id_(id) {
  }

  uint32_t flavour() const { return flavour_; }

  std::string string() const;

  bool is_valid() const {
    return flavour_ != BAD_VALUE;
  }

  bool operator < (const ActorId& other) const {
    return long_id_ < other.long_id_;
  }
  bool operator == (const ActorId& other) const {
    return long_id_ == other.long_id_;
  }  
  bool operator != (const ActorId& other) const {
    return long_id_ != other.long_id_;
  }
};

} // ns act
