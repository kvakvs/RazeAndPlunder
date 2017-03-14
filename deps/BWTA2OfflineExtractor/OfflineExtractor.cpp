#include "MapFileParser.h"

// TODO: support for folder  or patterns
// TODO: be able to change output folder
// TODO: set different levels of "verbose" (text logs, images, ...)
// TODO: include resources needed inside the EXE (tileset folder)

int main(int argc, char * argv[])
{
	// Check the number of parameters
	if (argc < 2) {
		// Tell the user how to run the program
		std::cerr << "Usage: " << argv[0] << " [mapFile]" << std::endl;
		return 1;
	}

	std::cout << "=======================" << std::endl;
	std::cout << "      OFFLINE BWTA     " << std::endl;
	std::cout << "=======================" << std::endl;

	// Off line map file parsing
	bool ok = BWTA::parseMapFile(argv[1]);
	if (!ok) return 1;

	// Normal procedure to analyze map
	std::cout << "All info loaded, analyzing map as regular..." << std::endl;
	BWTA::analyze();
	std::cout << "DONE" << std::endl;

	BWTA::cleanMemory();
	return 0;
}