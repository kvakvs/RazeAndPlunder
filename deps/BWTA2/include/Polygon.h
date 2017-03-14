#pragma once
#include <BWAPI.h>
#include <vector>

namespace BWTA {
  class Polygon : public std::vector<BWAPI::Position> {
  public:
    virtual ~Polygon() {
      for (const auto& h : holes) delete h;
    };

    virtual const double getArea() const = 0;
    virtual const double getPerimeter() const = 0;
    virtual const BWAPI::Position getCenter() const = 0;
    virtual BWAPI::Position getNearestPoint(const BWAPI::Position& p) const = 0;
    virtual const std::vector<Polygon*>& getHoles() const = 0;

    virtual bool isInside(const BWAPI::Position& pos) const = 0;

  protected:
    std::vector<Polygon*> holes;
  };
}
