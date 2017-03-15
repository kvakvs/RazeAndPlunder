#include "Pathfinder.h"
#include "../Managers/ExplorationManager.h"
#include "Utils/Profiler.h"

using namespace BWAPI;

Pathfinder::Pathfinder() {
  running = true;
  CreateThread();
}

Pathfinder::~Pathfinder() {
  running = false;

  for (auto& p : pathObj) {
    delete p;
  }
}

PathObj* Pathfinder::getPathObj(TilePosition start, TilePosition end) {
  for (auto& p : pathObj) {
    if (p->matches(start, end)) {
      return p;
    }
  }
  return nullptr;
}

int Pathfinder::getDistance(TilePosition start, TilePosition end) {
  PathObj* obj = getPathObj(start, end);
  if (obj != nullptr) {
    if (obj->isFinished()) {
      return obj->getPath().size();
    }
  }
  return -1;
}

void Pathfinder::requestPath(TilePosition start, TilePosition end) {
  PathObj* obj = getPathObj(start, end);
  if (obj == nullptr) {
    obj = new PathObj(start, end);
    pathObj.insert(obj);
  }
}

bool Pathfinder::isReady(TilePosition start, TilePosition end) {
  PathObj* obj = getPathObj(start, end);
  if (obj != nullptr) {
    return obj->isFinished();
  }
  return false;
}

BWEM::CPPath Pathfinder::getPath(TilePosition start, TilePosition end) {
  PathObj* obj = getPathObj(start, end);
  if (obj != nullptr) {
    if (obj->isFinished()) {
      return obj->getPath();
    }
  }
  return BWEM::CPPath();
}

void Pathfinder::stop() {
  running = false;
}

bool Pathfinder::isRunning() {
  if (not Broodwar->isInGame()) running = false;
  return running;
}

unsigned long Pathfinder::Process(void* parameter) {
  while (running) {
    if (not isRunning()) return 0;
    for (auto& p : pathObj) {
      if (not isRunning()) return 0;
      if (not p->isFinished()) {
        p->calculatePath();
      }
    }
    Sleep(5);
  }

  return 0;
}
