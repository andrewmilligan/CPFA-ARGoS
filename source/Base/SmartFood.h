#ifndef SMARTFOOD_H
#define SMARTFOOD_H

#include <argos3/core/utility/math/vector2.h>

class SmartFood {
  
  public:
    SmartFood();
    SmartFood(const argos::CVector2 & p);
    SmartFood(size_t cid, const argos::CVector2 & p);

    const size_t & GetID() const;
    const size_t & GetClusterID() const;

    void SetID(size_t i);
    void SetClusterID(size_t ci);

    argos::CVector2 & Position(); 


  private:
    size_t id;
    size_t cluster_id;
    argos::CVector2 position;

    static size_t next_id;

};

#endif /* SMARTFOOD_H */
