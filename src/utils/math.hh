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

inline Vector2 get_homing_velocity(Vector2 pos1, Vector2 pos2, int velocity) {
  auto angle = evs::coordinate_angle(pos1, pos2);
  return {(float)cos(angle) * velocity, (float)sin(angle) * velocity};
}
}  // namespace evs

#endif  // UTILS_MATH_H
