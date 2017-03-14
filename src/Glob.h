#pragma once
#include <memory>

class Commander;
class Profiler;

namespace rnp {

  // Globally visible functions which access main class singleton
  std::shared_ptr<Commander> commander();
  Profiler* profiler();

} // ns rnp