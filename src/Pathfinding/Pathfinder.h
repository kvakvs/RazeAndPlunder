#pragma once

#include "MainAgents/BaseAgent.h"
#include "PathObj.h"
#include "Utils/cthread.h"

class PathObjSet : public BWAPI::SetContainer<PathObj*, std::hash<void*>> {
public:

};

/** This class is used to find a path betweed two tiles in the game world. Currently it uses the 
 * A-star implementation in BWTA, but it can easily be changed to another algorithm if needed.
 *
 * The pathfinder is threaded, so agents have to request a path that is put in a queue. Agents have to
 * check the isReady() method to find out when the path finding is finished.
 *
 * Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class Pathfinder : public CThread {
  bool running;
  PathObjSet pathObj;
  bool isRunning();

private:
  PathObj* getPathObj(BWAPI::TilePosition start, BWAPI::TilePosition end);

public:
  Pathfinder();
  ~Pathfinder();

  // Returns the ground distance between two positions. 
  int getDistance(BWAPI::TilePosition start, BWAPI::TilePosition end);

  void requestPath(BWAPI::TilePosition start, BWAPI::TilePosition end);

  bool isReady(BWAPI::TilePosition start, BWAPI::TilePosition end);

  // Returns the path between two positions. 
  BWEM::CPPath getPath(BWAPI::TilePosition start, BWAPI::TilePosition end);

  // Stops the pathfinder thread. 
  void stop();

  // Thread update method. 
  unsigned long Process(void* parameter) override;
};

