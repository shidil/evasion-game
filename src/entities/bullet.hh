#ifndef BULLET_H
#define BULLET_H
#include <raylib.h>

#include "../game.hh"
#include "../utils/math.hh"

namespace evs {
inline Bullet create_bullet(Enemy enemy, Player player) {
  Bullet bullet = {
      .position = enemy.position,
      .color = BLACK,
      .velocity =
          evs::get_homing_velocity(player.position, enemy.position, BULLET_VELOCITY),
  };

  return bullet;
}

inline bool check_bullet_collisions(Player player, std::vector<Bullet> &bullets) {
  for (int i = 0; i < bullets.size(); i++) {
    if (bullets[i].state == ActorState::DEAD) {
      continue;
    }

    if (CheckCollisionCircles(player.position, PLAYER_RADIUS, bullets[i].position,
                              BULLET_RADIUS)) {
      bullets[i].state = ActorState::DEAD;
      return true;
    }
  }

  return false;
}

}  // namespace evs

#endif  // BULLET_H
