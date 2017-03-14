#pragma once

#include "stdafx.h" // for polygon type
#include <BWTA/Polygon.h>

namespace BWTA {
  class PolygonImpl : public Polygon {
  public:
    PolygonImpl(); // TODO remove after fixing load_data
    explicit PolygonImpl(const BoostPolygon& boostPol, const int& scale = 1);
    explicit PolygonImpl(const Polygon& b);

    const double getArea() const override;
    const double getPerimeter() const override;
    const BWAPI::Position getCenter() const override;
    BWAPI::Position getNearestPoint(const BWAPI::Position& p) const override;

    const std::vector<Polygon*>& getHoles() const override {
      return holes;
    };
    void addHole(const PolygonImpl& h);

    bool isInside(const BWAPI::Position& pos) const override;
  };
}
