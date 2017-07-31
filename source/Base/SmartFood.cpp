#include "SmartFood.h"

size_t SmartFood::next_id = 0;

SmartFood::SmartFood()
  : id(next_id++),
  cluster_id(0),
  position(argos::CVector2(0,0)),
  radius(0)
{}

SmartFood::SmartFood(argos::Real r)
  : id(next_id++),
  cluster_id(0),
  position(argos::CVector2(0,0)),
  radius(r)
{}

SmartFood::SmartFood(argos::Real r, const argos::CVector2 & p)
  : id(next_id++),
  cluster_id(0),
  position(argos::CVector2(p)),
  radius(r)
{}

SmartFood::SmartFood(argos::Real r, const argos::CVector2 & p, size_t cid)
  : id(next_id++),
  cluster_id(cid),
  position(argos::CVector2(p)),
  radius(r)
{}

const size_t & SmartFood::GetID() const {
  return id;
}

const size_t & SmartFood::GetClusterID() const {
  return cluster_id;
}

void SmartFood::SetID(size_t i) {
  id = i;
}

void SmartFood::SetClusterID(size_t ci) {
  cluster_id = ci;
}

void SmartFood::SetPosition(argos::CVector2 & pos) {
  position = pos;
}

argos::CVector2 & SmartFood::Position() {
  return position;
}

void SmartFood::SetRadius(argos::Real r) {
  radius = r;
}

argos::Real & SmartFood::Radius() {
  return radius;
}
