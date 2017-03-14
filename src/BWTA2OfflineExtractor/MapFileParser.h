#pragma once

#include <windows.h>
#include <direct.h>
#include <bitset>
#include <StormLib.h>
#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>

#include "BWTA2/MapData.h"
#include "BWTA2/TileType.h"

#include "sha1.h"
#include "TileSet.h"
#include "MiniTileFlags.h"

namespace BWTA
{
	const std::string tileSetName[9] = {
		"badlands",
		"platform",
		"install",
		"ashworld",
		"jungle",
		"desert",
		"ice",
		"twilight",
		"Unknown"
	};

	unsigned char* extractCHKfile(const char* archive, DWORD* dataSize);
	void printCHKchunks(unsigned char* CHKdata, DWORD size);
	unsigned char* getChunkPointer(unsigned char* desiredChunk, unsigned char* CHKdata, DWORD size, DWORD* desiredChunkLength);

	void getDimensions(unsigned char* CHKdata, DWORD size, unsigned int& width, unsigned int& height);
	void getUnits(unsigned char* CHKdata, DWORD size);
	void getDoodads(unsigned char* CHKdata, DWORD size);
	unsigned int getTileset(unsigned char* CHKdata, DWORD size);
	unsigned char* getFileBuffer(const char* filename);

	void setOfflineWalkability(RectangleArray<bool> &walkability);
	void setOfflineBuildability(RectangleArray<bool> &buildability);

	bool parseMapFile(const char* mapFilePath);
}
