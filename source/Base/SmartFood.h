#ifndef SMARTFOOD_H
#define SMARTFOOD_H

#include <argos3/core/utility/math/vector2.h>

class SmartFood {
  
  public:
    SmartFood();
    SmartFood(argos::Real r);
    SmartFood(argos::Real r, const argos::CVector2 & p);
    SmartFood(argos::Real r, const argos::CVector2 & p, size_t cid);

    const size_t & GetID() const;
    const size_t & GetClusterID() const;

    void SetID(size_t i);
    void SetClusterID(size_t ci);

    void SetPosition(argos::CVector2 & pos);
    argos::CVector2 & Position(); 

    void SetRadius(argos::Real r);
    argos::Real & Radius();


  private:
    size_t id;
    size_t cluster_id;
    argos::CVector2 position;

    argos::Real radius;

    static size_t next_id;

};

#endif /* SMARTFOOD_H */
