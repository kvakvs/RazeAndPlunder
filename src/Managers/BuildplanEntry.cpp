#include "BuildplanEntry.h"

using namespace BWAPI;

BuildplanEntry::BuildplanEntry(UnitType cType, int cSupply)
    : type_(Type::BUILDING), unit_type_(cType), upgrade_type_()
    , tech_type_()
    , supply_(cSupply) {
}

BuildplanEntry::BuildplanEntry(UpgradeType cType, int cSupply)
    : type_(Type::UPGRADE), unit_type_(), upgrade_type_(cType)
    , tech_type_()
    , supply_(cSupply) {
}

BuildplanEntry::BuildplanEntry(TechType cType, int cSupply)
    : type_(Type::TECH), unit_type_(), upgrade_type_()
    , tech_type_(cType)
    , supply_(cSupply)
{
}

std::string BuildplanEntry::to_string() const {
  switch (type_) {
  case Type::BUILDING: return "<building " + unit_type().toString() + ">";
  case Type::UPGRADE: return "<upgrade " + upgrade_type().toString() + ">";
  case Type::TECH: return "<tech " + tech_type().toString() + ">";
  default: return "<buildplan entry ?>";
  }
}

void Buildplan::drop(size_t i) {
  auto iter = plan_.begin() + i;
  rnp::log()->trace("buildplan: drop {}", iter->to_string());
  plan_.erase(iter);
}

void Buildplan::for_each(size_t supply, EntryHandler on_unit, 
                         EntryHandler on_upgrade, EntryHandler on_tech) {
  for (size_t i = 0; i < plan_.size(); i++) {
    if (supply >= plan_[i].trigger_at_supply()) {
      switch (plan_[i].type()) {
      case BuildplanEntry::Type::BUILDING:
        if (on_unit(plan_[i])) { drop(i); }
        i--;
        break;

      case BuildplanEntry::Type::UPGRADE:
        if (on_upgrade(plan_[i])) { drop(i); }
        i--;
        break;

      case BuildplanEntry::Type::TECH:
        if (on_tech(plan_[i])) { drop(i); }
        i--;
        break;
      } // switch
    } // if trigger at supply
  }
}

void Buildplan::debug_print_info() const {
  auto tot_lines = std::max<size_t>(1, std::min<size_t>(4, plan_.size()));

  Broodwar->drawBoxScreen(298, 25, 482, 41 + tot_lines * 16, Colors::Black, true);
  Broodwar->drawTextScreen(300, 25, "\x03Strategy Plan");
  Broodwar->drawLineScreen(300, 39, 480, 39, Colors::Orange);
  
  int no = 0;
  auto max = std::min<size_t>(4, plan_.size());

  for (size_t i = 0; i < max; i++) {
    std::string name = "";
    switch (plan_[i].type()) {
    case BuildplanEntry::Type::BUILDING:
      name = plan_[i].unit_type().getName();
      break;
    case BuildplanEntry::Type::UPGRADE:
      name = plan_[i].upgrade_type().getName();
      break;
    case BuildplanEntry::Type::TECH:
      name = plan_[i].tech_type().getName();
      break;
    }
    name = rnp::remove_race(name);

    std::stringstream ss;
    ss << name;
    ss << " \x0F(at ";
    ss << plan_[i].trigger_at_supply();
    ss << ")";

    Broodwar->drawTextScreen(300, 40 + no * 16, ss.str().c_str());
    no++;
  }
  if (no == 0) no++;
  Broodwar->drawLineScreen(300, 40 + no * 16, 480, 40 + no * 16, Colors::Orange);
}
