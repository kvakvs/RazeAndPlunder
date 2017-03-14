#pragma once

#ifdef DEBUG_DRAW
#include <QtGui/QPainter>
#include <QtGui/QColor>
#include <QtGui/QGuiApplication>

#include "MapData.h"
#include "RegionGenerator.h"

namespace BWTA {
	class Painter {
	public:
		Painter();
		~Painter() {};
		void render(const std::string& label = std::string());
		void drawMapBorder();
		void drawPolygon(const Polygon& polygon, QColor color, double scale = 1.0);
	  	void drawPolygons(const std::vector<Polygon>& polygons);
		void drawPolygons(const std::vector<Polygon*>& polygons);
		void drawPolygons(const std::vector<BoostPolygon>& polygons);
		void drawRegions(std::vector<Region*> regions);
		void drawRegions2(std::vector<Region*> regions);
		void drawChokepoints(std::set<Chokepoint*> chokepoints);
		void drawHeatMap(RectangleArray<int> map, float maxValue);
		void drawClosestBaseLocationMap(RectangleArray<BaseLocation*> map, std::set<BaseLocation*> baseLocations);
		void drawClosestChokepointMap(RectangleArray<Chokepoint*> map, std::set<Chokepoint*> chokepoints);
		void drawEdges(std::vector<boost::polygon::voronoi_edge<double>> edges);
		void drawGraph(const RegionGraph& graph);
		void drawNodes(const RegionGraph& graph, const std::set<nodeID>& nodes, QColor color);
		void drawLines(std::map<nodeID, chokeSides_t> chokepointSides, QColor color);
		void drawLine(const BoostSegment& seg, QColor color);
		void drawText(int x, int y, std::string text);
		void drawBaseLocations(std::set<BaseLocation*> baseLocations);

	private:
		QImage image;
		QPainter painter;
		int renderCounter;

		void getHeatMapColor(float value, int &red, int &green, int &blue) const;
		static QColor hsl2rgb(double h, double sl, double l);
	};
}
#endif
