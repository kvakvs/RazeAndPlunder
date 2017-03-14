#include "Pathfinder.h"
#include "../Managers/ExplorationManager.h"
#include "../Utils/Profiler.h"
#include <fstream>

Pathfinder* Pathfinder::instance = nullptr;

Pathfinder::Pathfinder()
{
	running = true;
	CreateThread();
}

Pathfinder::~Pathfinder()
{
	running = false;
	
	for (auto &p : pathObj)
	{	
		delete p;
	}
	instance = nullptr;
}

Pathfinder* Pathfinder::getInstance()
{
	if (instance == nullptr)
	{
		instance = new Pathfinder();
	}
	return instance;
}

PathObj* Pathfinder::getPathObj(TilePosition start, TilePosition end)
{
	for (auto &p : pathObj)
	{
		if (p->matches(start, end))
		{
			return p;
		}
	}
	return nullptr;
}

int Pathfinder::getDistance(TilePosition start, TilePosition end)
{
	PathObj* obj = getPathObj(start, end);
	if (obj != nullptr)
	{
		if (obj->isFinished())
		{
			return obj->getPath().size();
		}
	}
	return -1;
}

void Pathfinder::requestPath(TilePosition start, TilePosition end)
{
	PathObj* obj = getPathObj(start, end);
	if (obj == nullptr) 
	{
		obj = new PathObj(start, end);
		pathObj.insert(obj);
	}
}

bool Pathfinder::isReady(TilePosition start, TilePosition end)
{
	PathObj* obj = getPathObj(start, end);
	if (obj != nullptr)
	{
		return obj->isFinished();
	}
	return false;
}

vector<TilePosition> Pathfinder::getPath(TilePosition start, TilePosition end)
{
	PathObj* obj = getPathObj(start, end);
	if (obj != nullptr)
	{
		if (obj->isFinished())
		{
			return obj->getPath();
		}
	}
	return vector<TilePosition>();
}

void Pathfinder::stop()
{
	running = false;
}

bool Pathfinder::isRunning()
{
	if (!Broodwar->isInGame()) running = false;
	return running;
}

unsigned long Pathfinder::Process (void* parameter)
{
	while (running)
	{
		if (!isRunning()) return 0;
		for (auto &p : pathObj)
		{
			if (!isRunning()) return 0;
			if (!p->isFinished())
			{
				p->calculatePath();
			}
		}
		Sleep(5);
	}

	return 0;
}

