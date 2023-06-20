#ifndef STAR_CIRCLE_HPP
#define STAR_CIRCLE_HPP

#include "StarVector.hpp"
#include "StarLine.hpp"

namespace Star {

template<typename T>
class Circle {
public:
  typedef Vector<T, 2> Vec2T;
  typedef Line<T, 2> LineT;
  Circle();
  Circle(Vec2T const& pos, T radius);

  bool intersects(Vec2T const& point);
  Vec2T pointAtAngle(T angle);
  T angleNearestTo(Vec2T const& point);
  Vec2T pointNearestTo(Vec2T const& point);

private:
  T m_radius;
  Vec2T m_position;
};

template<typename T>
Circle<T>::Circle() 
  : m_radius(0) { }

template<typename T>
Circle<T>::Circle(Vec2T const& pos, T radius) 
  : m_radius(radius),
    m_position(pos) { }

template<typename T>
bool Circle<T>::intersects(Vec2T const& point) {
  T distSqu = (point - m_position).magnitudeSquared();
  return distSqu <= m_radius * m_radius;
}

template<typename T>
auto Circle<T>::pointAtAngle(T angle) -> Vec2T {
  auto fromCenter = Vec2T::withAngle(angle, m_radius);
  return fromCenter + m_position;
}

template<typename T>
auto Circle<T>::angleNearestTo(Vec2T const& point) -> T {
  return (point - m_position).angle();
}

template<typename T>
auto Circle<T>::pointNearestTo(Vec2T const& point) -> Vec2T {
  T angle = angleNearestTo(point);
  return pointAtAngle(angle);
}

typedef Circle<float> CirF;
typedef Circle<double> CirD;

}

#endif
