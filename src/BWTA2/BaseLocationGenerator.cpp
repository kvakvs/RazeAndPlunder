#include "BaseLocationGenerator.h"

#include "BaseLocationImpl.h"
#include "RegionImpl.h"
#include "Heap.h"

namespace BWTA
{
	const int MIN_CLUSTER_DIST = 6; // minerals less than this distance will be grouped into the same cluster
	const size_t MIN_RESOURCES = 3; // a cluster with less than this will be discarded
	const int MAX_INFLUENCE_DISTANCE_RADIUS = 12; // max radius distance from a resource to place a base

	void calculateTileDistances(const RectangleArray<bool>& walkable,
		const BWAPI::TilePosition &start,
		const double& maxDistance,
		RectangleArray<double>& distanceMap)
	{
		Heap<BWAPI::TilePosition, double> heap(true);
		distanceMap.setTo(-1);
		heap.push(std::make_pair(start, 0));
		distanceMap[start.x][start.y] = 0;
		while (!heap.empty()) {
			BWAPI::TilePosition pos = heap.top().first;
			double distance = heap.top().second;
			heap.pop();
			int x = pos.x;
			int y = pos.y;
			if (distance > maxDistance && maxDistance > 0) break;
			int min_x = std::max(x - 1, 0);
			int max_x = std::min(x + 1, (int)walkable.getWidth() - 1);
			int min_y = std::max(y - 1, 0);
			int max_y = std::min(y + 1, (int)walkable.getHeight() - 1);
			for (int ix = min_x; ix <= max_x; ix++) {
				for (int iy = min_y; iy <= max_y; iy++) {
					double f = std::abs(ix - x) + std::abs(iy - y);
					if (f > 1) f = 1.4;
					double v = distance + f;
					if (distanceMap[ix][iy] > v) {
						heap.push(std::make_pair(BWAPI::TilePosition(x, y), v));
						distanceMap[ix][iy] = v;
					} else {
						if (distanceMap[ix][iy] == -1 && walkable[ix][iy]) {
							distanceMap[ix][iy] = v;
							heap.push(std::make_pair(BWAPI::TilePosition(ix, iy), v));
						}
					}
				}
			}
		}
	}

	// TODO review this method, probably we can optimize it
	void calculate_walk_distances_area(const BWAPI::Position &start
		, int width
		, int height
		, int max_distance
		, RectangleArray<int> &distance_map)
	{
		Heap< BWAPI::Position, int > heap(true);
		for (unsigned int x = 0; x < distance_map.getWidth(); x++) {
			for (unsigned int y = 0; y < distance_map.getHeight(); y++) {
				distance_map[x][y] = -1;
			}
		}
		int sx = (int)start.x;
		int sy = (int)start.y;
		for (int x = sx; x < sx + width; x++) {
			for (int y = sy; y<sy + height; y++) {
				heap.push(std::make_pair(BWAPI::Position(x, y), 0));
				distance_map[x][y] = 0;
			}
		}
		while (!heap.empty()) {
			BWAPI::Position pos = heap.top().first;
			int distance = heap.top().second;
			heap.pop();
			int x = (int)pos.x;
			int y = (int)pos.y;
			if (distance>max_distance && max_distance>0) break;
			int min_x = std::max(x - 1, 0);
			int max_x = std::min(x + 1, MapData::mapWidthWalkRes - 1);
			int min_y = std::max(y - 1, 0);
			int max_y = std::min(y + 1, MapData::mapHeightWalkRes - 1);
			for (int ix = min_x; ix <= max_x; ix++) {
				for (int iy = min_y; iy <= max_y; iy++) {
					int f = std::abs(ix - x) * 10 + std::abs(iy - y) * 10;
					if (f > 10) { f = 14; }
					int v = distance + f;
					if (distance_map[ix][iy] > v) {
						heap.push(std::make_pair(BWAPI::Position(x, y), v));
						distance_map[ix][iy] = v;
					} else {
						if (distance_map[ix][iy] == -1 && MapData::rawWalkability[ix][iy] == true) {
							distance_map[ix][iy] = v;
							heap.push(std::make_pair(BWAPI::Position(ix, iy), v));
						}
					}
				}
			}
		}
	}

	std::vector<int> findNeighbors(const std::vector<unitTypeTilePos_t>& resources, const unitTypeTilePos_t& resource)
	{
		int dist;
		std::vector<int> retKeys;
		for (size_t i = 0; i < resources.size(); ++i) {
			dist = resources.at(i).pos.getApproxDistance(resource.pos);
			if (dist <= MIN_CLUSTER_DIST) retKeys.emplace_back(i);
		}
		return retKeys;
	}

	void detectBaseLocations(std::set<BaseLocation*>& baseLocations)
	{
		Timer timer;
		timer.start();

		// 1) cluster resources using DBSCAN algorithm
		// ===========================================================================
		std::vector< std::vector< unitTypeTilePos_t > > clusters;
		std::vector<bool> clustered(MapData::resources.size());
		std::vector<bool> visited(MapData::resources.size());
		std::vector<int> neighbors;
		std::vector<int> neighbors_;
		int c = -1;

		// for each unvisited point P in dataset
		for (size_t i = 0; i < MapData::resources.size(); ++i) {
			if (!visited.at(i)) {
				visited[i] = true; // mark P as visited
				neighbors = findNeighbors(MapData::resources, MapData::resources.at(i));
				if (neighbors.size() >= MIN_RESOURCES) {
					clusters.emplace_back();
					++c;
					// add P to cluster c
					clusters.at(c).emplace_back(MapData::resources.at(i));
					clustered[i] = true;
					// for each point P' in neighborPts
					for (size_t j = 0; j < neighbors.size(); ++j) {
						// if P' is not visited
						if (!visited.at(neighbors.at(j))) {
							visited[neighbors.at(j)] = true; // mark P' as visited
							neighbors_ = findNeighbors(MapData::resources, MapData::resources.at(neighbors.at(j)));
							if (neighbors_.size() >= MIN_RESOURCES) {
								neighbors.insert(neighbors.end(), neighbors_.begin(), neighbors_.end());
							}
						}
						// if P' is not yet a member of any cluster
						if (!clustered.at(neighbors.at(j))) {
							// add P' to cluster c
							clusters.at(c).emplace_back(MapData::resources.at(neighbors.at(j)));
							clustered[neighbors.at(j)] = true;
						}
					}
				}
			}
		}
		LOG(" - Found " << clusters.size() << " resource clusters in " << timer.stopAndGetTime() << " seconds");
		timer.start();

		// 2) compute a buildable map where a resource depot can be build (4x3 tiles)
		// ===========================================================================

		RectangleArray<bool> baseBuildMap(MapData::mapWidthTileRes, MapData::mapHeightTileRes);
		// baseBuildMap[x][y] is true if build_map[ix][iy] is true for all x<=ix<x+4 and all y<=yi<+3
		// 4 and 3 are the tile width and height of a command center/nexus/hatchery
		for (unsigned int x = 0; x < MapData::buildability.getWidth(); ++x) {
			for (unsigned int y = 0; y < MapData::buildability.getHeight(); ++y) {
				unsigned int maxX = std::min(x + 4, MapData::buildability.getWidth());
				unsigned int maxY = std::min(y + 3, MapData::buildability.getHeight());
				baseBuildMap[x][y] = true;
				for (unsigned int ix = x; ix < maxX; ++ix) {
					for (unsigned int iy = y; iy < maxY; ++iy) {
						// Set baseBuildMap[x][y] to false if buildability[ix][iy] is false
						baseBuildMap[x][y] &= MapData::buildability[ix][iy];
					}
				}

				// If this tile is too close to the bottom or right of the map, set it to false
				if (x + 4 > MapData::buildability.getWidth() || y + 3 > MapData::buildability.getHeight()) {
					baseBuildMap[x][y] = false;
				}
			}
		}
		// Set build tiles too close to resources in any cluster to false in baseBuildMap
		for (const auto& cluster : clusters) {
			for (const auto& resource : cluster) {
				int x1 = resource.pos.x - 6;
				int y1 = resource.pos.y - 5;
				int x2 = resource.pos.x + resource.type.tileWidth() + 2;
				int y2 = resource.pos.y + resource.type.tileHeight() + 2;
				baseBuildMap.setRectangleTo(x1, y1, x2, y2, false);
			}
		}
		LOG(" - baseBuildMap computed in " << timer.stopAndGetTime() << " seconds");
		timer.start();

		// 3) with the clusters and baseBuildMap, we will try to find a base location for each cluster
		// ===========================================================================
		
		RectangleArray<double> tileScores(MapData::mapWidthTileRes, MapData::mapHeightTileRes);
		RectangleArray<double> distanceMap(MapData::mapWidthTileRes, MapData::mapHeightTileRes);

		for (size_t i = 0; i < clusters.size(); ++i) {
			tileScores.setTo(0);
			int clusterMaxX = 0;
			int clusterMinX = (int)baseBuildMap.getWidth() - 1;
			int clusterMaxY = 0;
			int clusterMinY = (int)baseBuildMap.getHeight() - 1;
			for (size_t j = 0; j < clusters[i].size(); ++j) {
				//this will flood the nearby tiles with their distances to the current mineral
				calculateTileDistances(MapData::lowResWalkability, clusters[i][j].pos, (double)MAX_INFLUENCE_DISTANCE_RADIUS, distanceMap);
				int x = clusters[i][j].pos.x;
				int y = clusters[i][j].pos.y;
				int min_x = std::max(x - MAX_INFLUENCE_DISTANCE_RADIUS, 0);
				int max_x = std::min(x + MAX_INFLUENCE_DISTANCE_RADIUS, (int)baseBuildMap.getWidth() - 1);
				int min_y = std::max(y - MAX_INFLUENCE_DISTANCE_RADIUS, 0);
				int max_y = std::min(y + MAX_INFLUENCE_DISTANCE_RADIUS, (int)baseBuildMap.getHeight() - 1);
				for (int ix = min_x; ix <= max_x; ix++) {
					for (int iy = min_y; iy <= max_y; iy++) {
						if (baseBuildMap[ix][iy] && distanceMap[ix][iy] >= 0) {
							tileScores[ix][iy] += 20.0 - distanceMap[ix][iy];
						}
					}
				}
				// update the bounding box to search best score
				clusterMaxX = std::max(max_x, clusterMaxX);
				clusterMinX = std::min(min_x, clusterMinX);
				clusterMaxY = std::max(max_y, clusterMaxY);
				clusterMinY = std::min(min_y, clusterMinY);
			}
			BWAPI::TilePosition bestTile(-1, -1);
			double maxScore = 0;
			for (int x = clusterMinX; x < clusterMaxX; ++x) {
				for (int y = clusterMinY; y < clusterMaxY; ++y) {
					if (tileScores[x][y] > maxScore) {
						maxScore = tileScores[x][y];
						bestTile = BWAPI::TilePosition(x, y);
					}
				}
			}
			if (maxScore > 0) {
				baseLocations.insert(new BaseLocationImpl(bestTile, clusters[i]));
			}
		}
		LOG(" - Best baseLocations computed in " << timer.stopAndGetTime() << " seconds");
	}



	//attach resource pointers to base locations based on proximity (walk distance)
	void attachResourcePointersToBaseLocations(std::set< BWTA::BaseLocation* > &baseLocations)
	{
		RectangleArray<int> distanceMap(MapData::mapWidthWalkRes, MapData::mapHeightWalkRes);
		for (std::set<BWTA::BaseLocation*>::iterator i = baseLocations.begin(); i != baseLocations.end(); i++) {
			BWAPI::Position p((*i)->getTilePosition().x * 4, (*i)->getTilePosition().y * 4);
			calculate_walk_distances_area(p, 16, 12, 10 * 4 * 10, distanceMap);
			BWTA::BaseLocationImpl* ii = dynamic_cast<BWTA::BaseLocationImpl*>(*i);
			
			for (auto geyser : BWAPI::Broodwar->getStaticGeysers()) {
				int x = geyser->getInitialTilePosition().x * 4 + 8;
				int y = geyser->getInitialTilePosition().y * 4 + 4;
				if (distanceMap[x][y] >= 0 && distanceMap[x][y] <= 4 * 10 * 10) {
					ii->geysers.insert(geyser);
				}
			}
			
			for (auto mineral : BWAPI::Broodwar->getStaticMinerals()) {
				int x = mineral->getInitialTilePosition().x * 4 + 4;
				int y = mineral->getInitialTilePosition().y * 4 + 2;
				if (distanceMap[x][y] >= 0 && distanceMap[x][y] <= 4 * 10 * 10) {
					ii->staticMinerals.insert(mineral);
				}
			}
		}
	}

	void calculateBaseLocationProperties()
	{
		RectangleArray<double> distanceMap; 
		for (auto& base : BWTA_Result::baselocations) {
			BaseLocationImpl* baseI = (BaseLocationImpl*)base;

			// TODO this can be optimized only computing the distance between reachable base locations
			BWAPI::TilePosition baseTile = base->getTilePosition();
			getGroundDistanceMap(baseTile, distanceMap);
			// assume the base location is an island unless we can walk from this base location to another base location
			for (const auto& base2 : BWTA_Result::baselocations) {
				if (base == base2) {
					baseI->groundDistances[base2] = 0;
					baseI->airDistances[base2] = 0;
				} else {
					BWAPI::TilePosition base2Tile = base2->getTilePosition();
					if (baseI->_isIsland && isConnected(baseTile, base2Tile)) {
						baseI->_isIsland = false;
					}
					baseI->groundDistances[base2] = distanceMap[base2Tile.x][base2Tile.y];
					baseI->airDistances[base2] = baseTile.getDistance(base2Tile);
				}
			}

			// look if this base location is a start location
			for (const auto& startLocation : MapData::startLocations) {
				int distance = startLocation.getApproxDistance(base->getTilePosition());
				if (distance < 10) {
					baseI->_isStartLocation = true;
					BWTA_Result::startlocations.insert(base);
					break;
				}
			}

			// find what region this base location is in and tell that region about the base location
			BWAPI::WalkPosition baseWalkPos(base->getPosition());
			int baseRegionLabel = BWTA_Result::regionLabelMap[baseWalkPos.x][baseWalkPos.y];
			for (const auto& r : BWTA_Result::regions) {
				if (r->getLabel() == baseRegionLabel) { // TODO I need a vector to map labelId to Region*
					baseI->region = r;
					((RegionImpl*)r)->baseLocations.insert(base);
					break;
				}
			}

		}
	}
}