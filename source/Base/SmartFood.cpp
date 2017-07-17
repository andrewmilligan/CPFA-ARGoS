#include "SmartFood.h"

size_t SmartFood::next_id = 0;

SmartFood::SmartFood()
  : id(next_id++),
  cluster_id(0),
  position(argos::CVector2(0,0))
{}

SmartFood::SmartFood(const argos::CVector2 & p)
  : id(next_id++),
  cluster_id(0),
  position(argos::CVector2(p))
{}

SmartFood::SmartFood(size_t cid, const argos::CVector2 & p)
  : id(next_id++),
  cluster_id(cid),
  position(argos::CVector2(p))
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

argos::CVector2 & SmartFood::Position() {
  return position;
}
