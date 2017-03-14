#ifdef DEBUG_DRAW
#include "Painter.h"
#endif

#include "LoadData.h"
#include "BaseLocationGenerator.h"
#include "PolygonGenerator.h"
#include "RegionGenerator.h"
#include "ClosestObjectMap.h"
#include "BWTA_Result.h"

namespace BWTA {
  void analyze_map();

  void analyze() {
    cleanMemory();
    Timer timer;

#ifndef OFFLINE
    loadMapFromBWAPI();
#endif

    loadMap(); // compute extra map info

    // Verify if "BWTA2" directory exists, and create it if it doesn't.
    std::string bwtaPath(BWTA_PATH);
    if (!boost::filesystem::exists(bwtaPath)) {
      boost::filesystem::path dir(bwtaPath);
      boost::filesystem::create_directories(dir);
    }

    std::string filename = bwtaPath + MapData::hash + ".bwta";

    if (isFileVersionCorrect(filename)) {
      LOG("Recognized map, loading map data...");

      timer.start();
      load_data(filename);
      LOG("Loaded map data in " << timer.stopAndGetTime() << " seconds");
    }
    else {
      LOG("Analyzing new map...");

      timer.start();
      analyze_map();
      LOG("Map analyzed in " << timer.stopAndGetTime() << " seconds");

      // 			save_data(filename);
      // 			LOG("Saved map data.");
    }

#ifndef OFFLINE
    attachResourcePointersToBaseLocations(BWTA_Result::baselocations);
#endif
  }

  void analyze_map() {
#ifdef DEBUG_DRAW
    int argc = 1;
    char* argv = "0";
    QGuiApplication a(argc, &argv); // needed for print text (init fonts)
    Painter painter;
#endif
    Timer timer;
    timer.start();

    std::vector<BoostPolygon> boostPolygons;
    BWTA_Result::obstacleLabelMap.resize(MapData::walkability.getWidth(), MapData::walkability.getHeight());
    BWTA_Result::obstacleLabelMap.setTo(0);
    generatePolygons(boostPolygons, BWTA_Result::obstacleLabelMap);

    // translate Boost polygons to BWTA polygons
    for (const auto& pol : boostPolygons) {
      Polygon* bwtaPol = new PolygonImpl(pol);
      BWTA_Result::unwalkablePolygons.push_back(bwtaPol);
    }

    LOG(" [Detected polygons in " << timer.stopAndGetTime() << " seconds]");
#ifdef DEBUG_DRAW
    painter.drawPolygons(BWTA_Result::unwalkablePolygons);
    painter.render("01-Polygons");
    // Prints each polygon individually to debug 
    // for (auto tmpPol : polygons) {
    // painter.drawPolygon(tmpPol, QColor(180, 180, 180));
    // painter.render();
    // }
#endif
    timer.start();

    RegionGraph graph;
    bgi::rtree<BoostSegmentI, bgi::quadratic<16>> rtree;
    generateVoronoid(BWTA_Result::unwalkablePolygons, BWTA_Result::obstacleLabelMap, graph, rtree);

    LOG(" [Computed Voronoi in " << timer.stopAndGetTime() << " seconds]");
#ifdef DEBUG_DRAW
		painter.drawPolygons(BWTA_Result::unwalkablePolygons);
		painter.drawGraph(graph);
		painter.render("02-Voronoi");
#endif
    timer.start();

    pruneGraph(graph);

    LOG(" [Pruned Voronoi in " << timer.stopAndGetTime() << " seconds]");
#ifdef DEBUG_DRAW
		painter.drawPolygons(BWTA_Result::unwalkablePolygons);
		painter.drawGraph(graph);
		painter.render("03-VoronoiPruned");
#endif
    timer.start();

    detectNodes(graph, BWTA_Result::unwalkablePolygons);

    LOG(" [Identified region/chokepoints nodes in " << timer.stopAndGetTime() << " seconds]");
#ifdef DEBUG_DRAW
		painter.drawPolygons(BWTA_Result::unwalkablePolygons);
		painter.drawGraph(graph);
		painter.drawNodes(graph, graph.regionNodes, Qt::blue);
		painter.drawNodes(graph, graph.chokeNodes, Qt::red);
		painter.render("05-NodesDetected");
#endif
    timer.start();

    RegionGraph graphSimplified;
    simplifyGraph(graph, graphSimplified);

    LOG(" [Simplified graph in " << timer.stopAndGetTime() << " seconds]");
#ifdef DEBUG_DRAW
		painter.drawPolygons(BWTA_Result::unwalkablePolygons);
		painter.drawGraph(graphSimplified);
		painter.drawNodes(graphSimplified, graphSimplified.regionNodes, Qt::blue);
		painter.drawNodes(graphSimplified, graphSimplified.chokeNodes, Qt::red);
		painter.render("06-GraphPruned");
#endif
    timer.start();

    mergeRegionNodes(graphSimplified);

    LOG(" [Merged consecutive region nodes in " << timer.stopAndGetTime() << " seconds]");
#ifdef DEBUG_DRAW
		painter.drawPolygons(BWTA_Result::unwalkablePolygons);
		painter.drawGraph(graphSimplified);
		painter.drawNodes(graphSimplified, graphSimplified.regionNodes, Qt::blue);
		painter.drawNodes(graphSimplified, graphSimplified.chokeNodes, Qt::red);
		painter.render("07-GraphMerged");
#endif
    timer.start();

    std::map<nodeID, chokeSides_t> chokepointSides;
    getChokepointSides(graphSimplified, rtree, chokepointSides);

    LOG(" [Chokepoints sides computed in " << timer.stopAndGetTime() << " seconds]");
#ifdef DEBUG_DRAW
		painter.drawPolygons(BWTA_Result::unwalkablePolygons);
		painter.drawGraph(graphSimplified);
		painter.drawNodes(graphSimplified, graphSimplified.regionNodes, Qt::blue);
		painter.drawLines(chokepointSides, Qt::red);
		painter.render("08-WallOffChokepoints");
#endif
    timer.start();

    std::vector<BoostPolygon> polReg;
    createRegionsFromGraph(boostPolygons, BWTA_Result::obstacleLabelMap, graphSimplified, chokepointSides,
                           BWTA_Result::regions, BWTA_Result::chokepoints, polReg);

    LOG(" [Created BWTA regions/chokepoints in " << timer.stopAndGetTime() << " seconds]");
#ifdef DEBUG_DRAW
		painter.drawPolygons(BWTA_Result::unwalkablePolygons);
    // 		painter.drawRegions(BWTA_Result::regions); // uses another graph coloring
		painter.drawRegions2(BWTA_Result::regions);
		painter.drawChokepoints(BWTA_Result::chokepoints);
		painter.render("09-Regions");
#endif
    timer.start();

    detectBaseLocations(BWTA_Result::baselocations);
    // 		for (auto i : BWTA_Result::baselocations) {
    // 			log("BaseLocation at Position " << i->getPosition() << " Tile " << i->getTilePosition());
    // 		}

    LOG(" [Calculated base locations in " << timer.stopAndGetTime() << " seconds]");
    timer.start();

    computeAllClosestObjectMaps();

    LOG(" [Calculated closest maps in " << timer.stopAndGetTime() << " seconds]");
    timer.start();

    calculateBaseLocationProperties();
    // 		log("Debug BaseLocationProperties");
    // 		for (const auto& base : BWTA_Result::baselocations) {
    // 			BaseLocationImpl* baseI = (BaseLocationImpl*)base;
    // 			log("Base Position" << baseI->getTilePosition() << ",isIsland:" << baseI->isIsland()
    // 				<< ",isStartLocation:" << baseI->isStartLocation()
    // 				<< ",regionCenter" << baseI->getRegion()->getCenter());
    // 		}

    LOG(" [Calculated base location properties in " << timer.stopAndGetTime() << " seconds]");
#ifdef DEBUG_DRAW
    //		painter.drawClosestBaseLocationMap(BWTA_Result::getBaseLocationW, BWTA_Result::baselocations);
    //		painter.render("ClosestBaseLocationMap");
    //		painter.drawClosestChokepointMap(BWTA_Result::getChokepointW, BWTA_Result::chokepoints);
    //		painter.render("ClosestChokepointMap");
		painter.drawPolygons(BWTA_Result::unwalkablePolygons);
		painter.drawRegions2(BWTA_Result::regions);
		painter.drawChokepoints(BWTA_Result::chokepoints);
		painter.drawBaseLocations(BWTA_Result::baselocations);
		painter.render("10-Final");
#endif

  }

}
