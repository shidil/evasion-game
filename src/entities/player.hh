#ifndef PLAYER_H
#define PLAYER_H
#include <raylib.h>

#include "../game.hh"
#include "../resources.hh"

static float player_rotation = 0;

namespace evs {
inline Player create_player(Vector2 pos) {
  Player player = {.position = pos,
                   .color = RED,
                   .state = ActorState::LIVE,
                   .shield = INITIAL_PLAYER_SHIELDS};

  return player;
}

inline void draw_player(Player player) {
  player_rotation += 1;
  // BeginShaderMode(shaders[PostproShader::FX_PREDATOR_VIEW]);
  DrawPolyLines(player.position, 3, PLAYER_RADIUS - 10 , player_rotation - 90, YELLOW);
  DrawPolyLines(player.position, 3, PLAYER_RADIUS - 10 , player_rotation - 45, YELLOW);
  DrawPolyLines(player.position, 7, PLAYER_RADIUS + 2 , player_rotation, WHITE);
  DrawCircleLines(player.position.x, player.position.y, PLAYER_RADIUS - 5, WHITE);

  // EndShaderMode();
}

}  // namespace evs

#endif  // PLAYER_H
