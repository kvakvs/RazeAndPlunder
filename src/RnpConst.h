#pragma once

#include "Actors/Actor.h"

// This is used as a prefix to console/broodwar outputs
#define BOT_PREFIX "[Raze&Plunder] " 

// This is used as a debug prefix to console/broodwar outputs
#define BOT_PREFIX_DEBUG "[R&P DEBUG] " 

#define TOURNAMENT_NAME "SSCAIT 2017"
#define SPONSORS "the Sponsors!"
#define MINIMUM_COMMAND_OPTIMIZATION 1

// These are used as flavour field in ActorId, extending the range of ids
enum class ActorFlavour : uint32_t {
  BadId = act::ActorId::BAD_VALUE,
  Unit,
  Squad,
  Singleton
};
