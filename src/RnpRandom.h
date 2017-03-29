#pragma once

#include <random>

namespace rnp {

class Rand {
  static std::mt19937 generator_;
public:
  static std::mt19937& gen() {
    return generator_;
  }

  template <class Container>
  static auto choice(const Container& items) {
    std::uniform_int_distribution<int> dist(0, items.size() - 1);
    return items[dist(gen())];
  }

  template <class Container>
  static auto choice_set(const Container& items) {
    std::uniform_int_distribution<int> dist(0, items.size() - 1);
    auto iter = items.begin();
    for (int i = dist(gen()); i > 0; --i) { iter++; }
    return *iter;
  }
};

} // ns rnp
