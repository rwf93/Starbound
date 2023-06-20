#ifndef STAR_COORD_SYS_H
#define STAR_COORD_SYS_H

#include "StarMathCommon.hpp"
#include "StarVector3.hpp"

namespace Star {

class CoordinateSystem3 {
public:
  static inline CoordinateSystem3 opengl() {
      return CoordinateSystem3(-3, 1, -2);
  }
  static inline CoordinateSystem3 northEastDown() {
      return CoordinateSystem3(1, 2, 3);
  }
  static inline CoordinateSystem3 eastNorthUp() {
      return CoordinateSystem3(2, 1, -3);
  }

  static inline Vec3D getDirection(int v) {
  Vec3D dir(0, 0, 0);
      if(v < 0)
          dir[-v - 1] = -1.0f;
      else
          dir[v - 1] = 1.0f;
      return dir;
  }

  inline CoordinateSystem3(int forward, int right, int down) {
    if((forward < -3 || forward > 3 || forward == 0) ||
       (right < -3 || right > 3 || right == 0) ||
       (down < -3 || down > 3 || down == 0)) {
         throw MathException("Value out of range [-3, -1], [1, -3] on "
           "CoordinateSystem3 constructor");
    }
    m_forward = forward;
    m_right = right;
    m_down = down;
  }

  inline Vec3D getForward() const { return getDirection(m_forward); }
  inline Vec3D getRight() const { return getDirection(m_right); }
  inline Vec3D getDown() const { return getDirection(m_down); }

  size_t getForwardIndex() const { return abs(m_forward) - 1; }
  size_t getRightIndex() const { return abs(m_right) - 1; }
  size_t getDownIndex() const { return abs(m_down) - 1; }

  int getForwardSign() const { return m_forward / abs(m_forward); }
  int getRightSign() const { return m_right / abs(m_right); }
  int getDownSign() const { return m_down / abs(m_down); }

  bool operator==(CoordinateSystem3 const& ref) const {
      return m_forward == ref.m_forward &&
          m_right == ref.m_right &&
          m_down == ref.m_down;
  }

private:
  int m_forward;
  int m_right;
  int m_down;
};

}

#endif
