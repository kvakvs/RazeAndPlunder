#include "Glob.h"
#include "RnpBot.h"
#include "Utils/Profiler.h"

namespace rnp {

  std::shared_ptr<Commander> commander() {
    return RnpBot::singleton()->commander_;
  }

  Profiler* profiler() {
    return RnpBot::singleton()->profiler_.get();
  }

} // ns rnp
