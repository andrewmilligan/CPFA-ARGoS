#ifndef SECTORFOOD_H
#define SECTORFOOD_H

#include <argos3/core/utility/math/vector2.h>
#include <vector>

// The SectorFood must be a rectangle. Create with lower left and upper right
// corner coords
class SectorFood {
  public:
    SectorFood();
    SectorFood(argos::CVector2 lleft, argos::CVector2 uright);
    SectorFood(argos::CVector2 lleft, argos::CVector2 uright, argos::Real p);

    bool ContainsPoint(const argos::CVector2& pos) const; 

    const argos::Real& GetProbability() const;

    void SetProbability(argos::Real p);

    argos::Real GetXMin() const;
    argos::Real GetXMax() const;
    argos::Real GetYMin() const;
    argos::Real GetYMax() const;

    void SetXMin(argos::Real xmin);
    void SetXMax(argos::Real xmax);
    void SetYMin(argos::Real ymin);
    void SetYMax(argos::Real ymax);

    const argos::CVector2& GetLowerLeft() const;
    const argos::CVector2& GetUpperRight() const;

    void SetLowerLeft(argos::CVector2 lleft);
    void SetUpperRight(argos::CVector2 uright);

    const std::vector<argos::CVector2>& GetCorners() const;
    const argos::CVector2& Position() const;

  private:
    void UpdateCorners();

    argos::CVector2 lower_left;
    argos::CVector2 upper_right;
    std::vector<argos::CVector2> corners;
    argos::CVector2 position;
    argos::Real seed_probability;
};


#endif
