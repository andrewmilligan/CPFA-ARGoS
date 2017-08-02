#include "SectorFood.h"

SectorFood::SectorFood()
  : lower_left(0,0),
  upper_right(0,0),
  seed_probability(0)
{
  UpdateCorners();
}

SectorFood::SectorFood(argos::CVector2 lleft, argos::CVector2 uright)
  : lower_left(lleft),
  upper_right(uright),
  seed_probability(0)
{
  UpdateCorners();
}

SectorFood::SectorFood(argos::CVector2 lleft, argos::CVector2 uright, argos::Real p)
  : lower_left(lleft),
  upper_right(uright),
  seed_probability(p)
{
  UpdateCorners();
}

bool SectorFood::ContainsPoint(const argos::CVector2& pos) const {
  return (((pos.GetX() < GetXMax()) && (pos.GetX() >= GetXMin())) &&
        ((pos.GetY() < GetYMax()) && (pos.GetY() >= GetYMin())));
}

const argos::Real& SectorFood::GetProbability() const {
  return seed_probability;
}

void SectorFood::SetProbability(argos::Real p) {
  seed_probability = p;
}

argos::Real SectorFood::GetXMin() const {
  return lower_left.GetX();
}
argos::Real SectorFood::GetXMax() const {
  return upper_right.GetX();
}
argos::Real SectorFood::GetYMin() const {
  return lower_left.GetY();
}
argos::Real SectorFood::GetYMax() const {
  return upper_right.GetY();
}

void SectorFood::SetXMin(argos::Real xmin) {
  lower_left = argos::CVector2(xmin, lower_left.GetY());
  UpdateCorners();
}
void SectorFood::SetXMax(argos::Real xmax) {
  upper_right = argos::CVector2(xmax, upper_right.GetY());
  UpdateCorners();
}
void SectorFood::SetYMin(argos::Real ymin) {
  lower_left = argos::CVector2(lower_left.GetX(), ymin);
  UpdateCorners();
}
void SectorFood::SetYMax(argos::Real ymax) {
  upper_right = argos::CVector2(upper_right.GetX(), ymax);
  UpdateCorners();
}


const argos::CVector2& SectorFood::GetLowerLeft() const {
  return lower_left;
}
const argos::CVector2& SectorFood::GetUpperRight() const {
  return upper_right;
}

void SectorFood::SetLowerLeft(argos::CVector2 lleft) {
  lower_left = lleft;
  UpdateCorners();
}
void SectorFood::SetUpperRight(argos::CVector2 uright) {
  upper_right = uright;
  UpdateCorners();
}

void SectorFood::UpdateCorners() {
  corners.clear();
  corners.push_back(lower_left);
  corners.push_back(argos::CVector2(lower_left.GetX(), upper_right.GetY()));
  corners.push_back(upper_right);
  corners.push_back(argos::CVector2(upper_right.GetX(), lower_left.GetY()));

  argos::Real x, y;
  x = (lower_left.GetX() + upper_right.GetX()) / 2.0;
  y = (lower_left.GetY() + upper_right.GetY()) / 2.0;
  position = argos::CVector2(x, y);
}
const std::vector<argos::CVector2>& SectorFood::GetCorners() const {
  return corners;
}
const argos::CVector2& SectorFood::Position() const {
  return position;
}
