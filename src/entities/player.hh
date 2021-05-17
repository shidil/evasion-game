#ifndef PLAYER_H
#define PLAYER_H
#include <raylib.h>

#include "../game.hh"

namespace evs {
inline Player create_player(Vector2 pos) {
  Player player = {.position = pos,
                   .color = RED,
                   .state = ActorState::LIVE,
                   .shield = INITIAL_PLAYER_SHIELDS};

  return player;
}

inline void draw_player(Player player) {
  DrawCircleLines(player.position.x, player.position.y, PLAYER_RADIUS, player.color);
}

}  // namespace evs

#endif  // PLAYER_H
