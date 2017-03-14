#pragma once

#include <BWAPI.h>
#include <BWAPI/SetContainer.h>
#include "../MainAgents/BaseAgent.h"

using namespace BWAPI;


/** Author: Johan Hagelback (johan.hagelback@gmail.com)
 */
class Agentset : public SetContainer<BaseAgent*, std::hash<void*>> {
public:
	//static const Agentset none;
};
