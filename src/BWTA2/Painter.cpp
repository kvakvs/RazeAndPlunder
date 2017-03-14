#ifdef DEBUG_DRAW
#include "Painter.h"
#include "GraphColoring.h"
#include "BaseLocationImpl.h"

namespace BWTA {

	std::vector<QColor> baseColors = { QColor(0, 114, 189), QColor(217, 83, 25), QColor(237, 177, 32),
		QColor(126, 47, 142), QColor(119, 172, 48), QColor(77, 190, 238), QColor(162, 20, 47) };

	std::vector<QColor> mapColors = { QColor(180, 180, 180), QColor(204, 193, 218), QColor(230, 185, 184),
		QColor(252, 216, 181), QColor(215, 228, 189), QColor(77, 190, 238), QColor(162, 20, 47) };

	Painter::Painter() :
		renderCounter(1)
	{
		image = QImage(MapData::mapWidthWalkRes, MapData::mapHeightWalkRes, QImage::Format_ARGB32_Premultiplied);
		painter.begin(&image);
		painter.setRenderHint(QPainter::Antialiasing);
	}

	void Painter::render(const std::string& label)
	{
		// save PNG
		std::string filename(BWTA_PATH);
		if (label.empty()) {
			filename += MapData::mapFileName + "-" + std::to_string(renderCounter) + ".png";
			renderCounter++;
		} else {
			filename += MapData::mapFileName + "-" + label + ".png";
		}

		image.save(filename.c_str(), "PNG");

		// restart device
		painter.end();
		image = QImage(MapData::mapWidthWalkRes, MapData::mapHeightWalkRes, QImage::Format_ARGB32_Premultiplied);
		painter.begin(&image);
		painter.setRenderHint(QPainter::Antialiasing);
	}

	void Painter::drawMapBorder() {
		QPen qp(QColor(0, 0, 0));
		qp.setWidth(2);
		painter.setPen(qp);
		painter.drawLine(0, 0, 0, MapData::walkability.getHeight() - 1);
		painter.drawLine(0, MapData::walkability.getHeight() - 1, MapData::walkability.getWidth() - 1, MapData::walkability.getHeight() - 1);
		painter.drawLine(MapData::walkability.getWidth() - 1, MapData::walkability.getHeight() - 1, MapData::walkability.getWidth() - 1, 0);
		painter.drawLine(MapData::walkability.getWidth() - 1, 0, 0, 0);
	}

	void Painter::drawPolygon(const Polygon& polygon, QColor color, double scale) {
		QVector<QPointF> qp;
		for (const auto &point : polygon) {
			qp.push_back(QPointF(point.x * scale, point.y * scale));
		}
//		for (size_t i = 0; i < polygon.size(); ++i) {
//			qp.push_back(QPointF(polygon[i].x * scale, polygon[i].y * scale));
//		}
		painter.setPen(QPen(Qt::black));
		painter.setBrush(QBrush(color));
		painter.drawPolygon(QPolygonF(qp));
	}

	void Painter::drawPolygons(const std::vector<Polygon>& polygons) {
		for (const auto& polygon : polygons) {
			drawPolygon(polygon, QColor(180, 180, 180));
			for (const auto& hole : polygon.getHoles()) {
				drawPolygon(*hole, QColor(255, 100, 255));
			}
		}
	}

	void Painter::drawPolygons(const std::vector<Polygon*>& polygons) {
		for (const auto& polygon : polygons) {
			drawPolygon(*polygon, QColor(180, 180, 180));
			for (const auto& hole : polygon->getHoles()) {
				drawPolygon(*hole, QColor(255, 100, 255));
			}
		}
	}

	void Painter::drawPolygons(const std::vector<BoostPolygon>& polygons) {
		for (auto& polygon : polygons) {
			QVector<QPointF> qp;

			for (const auto& point : polygon.outer()) {
				qp.push_back(QPointF(point.x(), point.y()));
			}

			painter.setPen(QPen(Qt::black));
			painter.setBrush(QBrush(QColor(180, 180, 180)));
			painter.drawPolygon(QPolygonF(qp));
		}
	}

	void Painter::drawRegions(std::vector<Region*> regions) {
		static bool colored = false;
		if (!colored) {
			regionColoring();
			colored = true;
		}
		for (const auto& r : regions) {
			drawPolygon(r->getPolygon(), mapColors.at(r->getColorLabel()), 0.125);
		}
	}

	void Painter::drawRegions2(std::vector<Region*> regions) {
		static bool colored = false;
		if (!colored) {
			regionColoringHUE();
			colored = true;
		}
		for (const auto& r : regions) {
			drawPolygon(r->getPolygon(), hsl2rgb(r->getHUE(), 1.0, 0.75), 0.125);
		}
	}

	QColor Painter::hsl2rgb(double h, double sl, double l)
	{
		double v;
		// default to gray
		double r = 1, g = 1, b = 1;
		v = (l <= 0.5) ? (l * (1.0 + sl)) : (l + sl - l * sl);
		if (v > 0) {
			double m;
			double sv;
			int sextant;
			double fract, vsf, mid1, mid2;
			m = l + l - v;
			sv = (v - m) / v;
			h *= 6.0;
			sextant = static_cast<int>(h);
			fract = h - sextant;
			vsf = v * sv * fract;
			mid1 = m + vsf;
			mid2 = v - vsf;
			switch (sextant) {
			case 0:
				r = v; g = mid1; b = m;
				break;
			case 1:
				r = mid2; g = v; b = m;
				break;
			case 2:
				r = m; g = v; b = mid1;
				break;
			case 3:
				r = m; g = mid2; b = v;
				break;
			case 4:
				r = mid1; g = m; b = v;
				break;
			case 5:
				r = v; g = m; b = mid2;
				break;
			}
		}
		return QColor(r*255.0, g*255.0, b*255.0);
	}

	void Painter::drawChokepoints(std::set<Chokepoint*> chokepoints) {
		QPen qp(QColor(255, 0, 0));
		qp.setWidth(3);
		painter.setPen(qp);
		for (const auto& c : chokepoints) {
			auto &sides = c->getSides();
			BWAPI::WalkPosition pos1(sides.first);
			BWAPI::WalkPosition pos2(sides.second);
			painter.drawLine(pos1.x, pos1.y, pos2.x, pos2.y);
		}
	}

	void Painter::getHeatMapColor(float value, int &red, int &green, int &blue) const
	{
		const int NUM_COLORS = 3;
		static float color[NUM_COLORS][3] = { { 255, 0, 0 }, { 0, 255, 0 }, { 0, 0, 255 } };
		// a static array of 3 colors:  (red, green, blue)

		int idx1;        // |-- our desired color will be between these two indexes in "color"
		int idx2;        // |
		float fractBetween = 0;  // fraction between "idx1" and "idx2" where our value is

		if (value <= 0)			{ idx1 = idx2 = 0; }				// accounts for an input <=0
		else if (value >= 1)	{ idx1 = idx2 = NUM_COLORS - 1; }	// accounts for an input >=0
		else {
			value = value * (NUM_COLORS - 1);	// will multiply value by 3
			idx1 = static_cast<int>(std::floor(value));	// our desired color will be after this index
			idx2 = idx1 + 1;					// ... and before this index (inclusive)
			fractBetween = value - float(idx1); // distance between the two indexes (0-1)
		}

		red   = static_cast<int>((color[idx2][0] - color[idx1][0])*fractBetween + color[idx1][0]);
		green = static_cast<int>((color[idx2][1] - color[idx1][1])*fractBetween + color[idx1][1]);
		blue  = static_cast<int>((color[idx2][2] - color[idx1][2])*fractBetween + color[idx1][2]);
	}

	void Painter::drawHeatMap(RectangleArray<int> map, float maxValue)
	{
		int red, green, blue;
		QColor heatColor;
		for (unsigned int x = 0; x < map.getWidth(); ++x) {
			for (unsigned int y = 0; y < map.getHeight(); ++y) {
				float normalized = static_cast<float>(map[x][y]) / maxValue;
				getHeatMapColor(normalized, red, green, blue);
				heatColor = QColor(red, green, blue);
				painter.setPen(QPen(heatColor));
				painter.setBrush(QBrush(heatColor));
				painter.drawEllipse(x, y, 1, 1);
			}
		}
	}

	void Painter::drawClosestBaseLocationMap(RectangleArray<BaseLocation*> map, std::set<BaseLocation*> baseLocations)
	{
		LOG("Drawing closest BaseLocation for " << baseLocations.size() << " bases");
		// assign a color to each BaseLocation
		std::vector<QColor> baseColors = { QColor(0, 114, 189), QColor(217, 83, 25), QColor(237, 177, 32)
			, QColor(126, 47, 142), QColor(119, 172, 48), QColor(77, 190, 238), QColor(162, 20, 47) };

		std::map<BaseLocation*, QColor> baseToColor;
		baseToColor[nullptr] = QColor(180, 180, 180);
		int i = 0;
		for (const auto& baseLocation : baseLocations) {
			i = i % baseColors.size();
			baseToColor[baseLocation] = baseColors.at(i);
			i++;
		}

		// draw BaseLocation closest map
		for (unsigned int x = 0; x < map.getWidth(); ++x) {
			for (unsigned int y = 0; y < map.getHeight(); ++y) {
				painter.setPen(QPen(baseToColor[map[x][y]]));
				painter.setBrush(QBrush(baseToColor[map[x][y]]));
				painter.drawEllipse(x, y, 1, 1);
			}
		}

		// draw BaseLocation origin
		QColor color(0, 0, 0);
		painter.setPen(QPen(color));
		painter.setBrush(QBrush(color));
		for (const auto& base : baseLocations) {
			painter.drawEllipse(base->getTilePosition().x * 4 - 6, base->getTilePosition().y * 4 - 6, 12, 12);
		}
	}

	void Painter::drawClosestChokepointMap(RectangleArray<Chokepoint*> map, std::set<Chokepoint*> chokepoints)
	{
		LOG("Drawing closest Chokepoint for " << chokepoints.size() << " chokepoints");
		// assign a color to each Chokepoint
		std::map<Chokepoint*, QColor> chokeToColor;
		chokeToColor[nullptr] = QColor(180, 180, 180);
		int i = 0;
		for (const auto& chokepoint : chokepoints) {
			i = i % baseColors.size();
			chokeToColor[chokepoint] = baseColors.at(i);
			i++;
		}

		// draw Chokepoint closest map
		for (unsigned int x = 0; x < map.getWidth(); ++x) {
			for (unsigned int y = 0; y < map.getHeight(); ++y) {
				painter.setPen(QPen(chokeToColor[map[x][y]]));
				painter.setBrush(QBrush(chokeToColor[map[x][y]]));
				painter.drawEllipse(x, y, 1, 1);
			}
		}

		// draw Chokepoint origin
		QColor color(0, 0, 0);
		painter.setPen(QPen(color));
		painter.setBrush(QBrush(color));
		for (const auto& chokepoint : chokepoints) {
			painter.drawEllipse(chokepoint->getCenter().x / 8, chokepoint->getCenter().y / 8, 12, 12);
		}
	}

	void Painter::drawEdges(std::vector<boost::polygon::voronoi_edge<double>> edges)
	{
		for (auto it = edges.begin(); it != edges.end(); ++it) {
			if (!it->is_primary()) {
				continue;
			}
			if (it->color() == 1) {
				QPen qp(QColor(255, 0, 0));
				qp.setWidth(2);
				painter.setPen(qp);
			} else {
				QPen qp(QColor(0, 0, 255));
				qp.setWidth(2);
				painter.setPen(qp);
			}
			if (!it->is_finite()) {
// 				clip_infinite_edge(*it, &samples);
			} else {
				painter.drawLine(it->vertex0()->x(), it->vertex0()->y(), 
					it->vertex1()->x(), it->vertex1()->y());
// 				if (it->is_curved()) {
// 					sample_curved_edge(*it, &samples);
// 				}
			}
		}
	}

	void Painter::drawGraph(const RegionGraph& graph)
	{
		QPen qp(QColor(0, 0, 255));
		qp.setWidth(2);
		painter.setPen(qp);

		// container to mark visited nodes
		std::vector<bool> visited;
		visited.resize(graph.nodes.size());

		std::queue<nodeID> nodeToPrint;
		// find first node with children
		for (size_t id = 0; id < graph.adjacencyList.size(); ++id) {
			if (!graph.adjacencyList.at(id).empty()) {
				nodeToPrint.push(id);
				visited.at(id) = true;
			}
		}
		

		while (!nodeToPrint.empty()) {
			// pop first element
			nodeID v0 = nodeToPrint.front();
			nodeToPrint.pop();

			// draw point if it is an leaf node
// 			if (graph.adjacencyList.at(v0).size() == 1) {
// 				painter.drawEllipse(graph.nodes.at(v0).x - 6, graph.nodes.at(v0).y - 6, 12, 12);
// 				nodeID v1 = *graph.adjacencyList.at(v0).begin();
// 				LOG("Leaf dist: " << graph.minDistToObstacle.at(v0) << " - parent: " << graph.minDistToObstacle.at(v1));
// 			}

			// draw all edges of node
			for (const auto& v1 : graph.adjacencyList.at(v0)) {
				painter.drawLine(graph.nodes.at(v0).x, graph.nodes.at(v0).y,
					graph.nodes.at(v1).x, graph.nodes.at(v1).y);

				if (!visited.at(v1)) {
					nodeToPrint.push(v1);
					visited.at(v1) = true;
				}
			}
		}
	}

	void Painter::drawNodes(const RegionGraph& graph, const std::set<nodeID>& nodes, QColor color) 
	{
		painter.setPen(QPen(color));
		painter.setBrush(QBrush(color));
		for (const auto& v0 : nodes) {
			painter.drawEllipse(graph.nodes.at(v0).x - 3, graph.nodes.at(v0).y - 3, 6, 6);
		}
	}

	void Painter::drawLines(std::map<nodeID, chokeSides_t> chokepointSides, QColor color)
	{
		painter.setPen(QPen(color));
		painter.setBrush(QBrush(color));
		for (const auto& chokeSides : chokepointSides) {
			painter.drawLine(chokeSides.second.side1.x, chokeSides.second.side1.y,
				chokeSides.second.side2.x, chokeSides.second.side2.y);
		}
	}

	void Painter::drawLine(const BoostSegment& seg, QColor color) {
		QPen qp(color);
		qp.setWidth(2);
		painter.setPen(qp);
		painter.setBrush(QBrush(color));
		painter.drawLine(seg.first.x(), seg.first.y(), seg.second.x(), seg.second.y());
	}

	void Painter::drawText(int x, int y, std::string text) {
		painter.setFont(QFont("Tahoma", 8));
		QRect rect = image.rect();
		rect.setLeft(5);
		painter.drawText(rect, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, QString::fromStdString(text));
	}

	void Painter::drawBaseLocations(std::set<BaseLocation*> baseLocations) {
		painter.setPen(QPen(Qt::blue));
		painter.setBrush(Qt::NoBrush);
		int baseWidth = 4*4;
		int baseHeight = 3*4;
		int minWidth = 2*4;
		int minHeight = 1*4;
		int vesWidth = 4*4;
		int vesHeight = 3*4;

		for (const auto& base : baseLocations) {
			int x = base->getTilePosition().x * 4;
			int y = base->getTilePosition().y * 4;
			if (base->isStartLocation()) painter.fillRect(x, y, baseWidth, baseHeight, Qt::red);
			painter.drawRect(x, y, baseWidth, baseHeight);
			BaseLocationImpl* b = dynamic_cast<BaseLocationImpl*>(base);
			for (const auto& r : b->resources) {
				if (r.type == BWAPI::UnitTypes::Resource_Vespene_Geyser) {
					painter.drawLine((base->getTilePosition().x * 4)+8, (base->getTilePosition().y * 4)+6,
						(r.pos.x * 4)+8, (r.pos.y * 4)+6);
					painter.fillRect(r.pos.x * 4, r.pos.y * 4, vesWidth, vesHeight, Qt::green);
				} else {
					painter.drawLine((base->getTilePosition().x * 4)+8, (base->getTilePosition().y * 4)+6,
						(r.pos.x * 4)+4, (r.pos.y * 4)+2);
					painter.fillRect(r.pos.x * 4, r.pos.y * 4, minWidth, minHeight, Qt::cyan);
				}
				
			}
		}
	}
}
#endif