#ifndef UTILS_MATH_H
#define UTILS_MATH_H
#include <raylib.h>

#include <cmath>

namespace evs {
inline float coordinate_angle(Vector2 pos1, Vector2 pos2) {
  return atan2(pos1.y - pos2.y, pos1.x - pos2.x);
}

inline float distance_2d(Vector2 pos1, Vector2 pos2) {
  float x1 = pos2.x - pos1.x;
  float y1 = pos2.y - pos1.y;
  return sqrt((x1 * x1) + (y1 * y1));
}
}  // namespace evs

#endif // UTILS_MATH_H
