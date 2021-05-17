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
  if (player_rotation > 360) {
    player_rotation = 0;
  }

  // BeginShaderMode(shaders[PostproShader::FX_PREDATOR_VIEW]);
  DrawPolyLines(player.position, 3, PLAYER_RADIUS - 10, player_rotation - 90, GRAY);
  DrawPolyLines(player.position, 3, PLAYER_RADIUS - 9, player_rotation - 90, LIGHTGRAY);
  DrawPolyLines(player.position, 3, PLAYER_RADIUS - 10, player_rotation - 45, GRAY);
  DrawPolyLines(player.position, 3, PLAYER_RADIUS - 9, player_rotation - 45, LIGHTGRAY);

  if (player.shield > 0) {
    DrawPolyLines(player.position, 8, PLAYER_RADIUS + 2, player_rotation,
                  {255, 255, 255, 100});
    DrawPolyLines(player.position, 8, PLAYER_RADIUS + 3, player_rotation,
                  {255, 255, 255, 50});
    DrawPolyLines(player.position, 8, PLAYER_RADIUS + 4, player_rotation,
                  {255, 255, 255, 10});
  }

  DrawCircleLines(player.position.x, player.position.y, PLAYER_RADIUS - 5, ORANGE);
  DrawCircleLines(player.position.x, player.position.y, PLAYER_RADIUS - 4, YELLOW);
  DrawCircleLines(player.position.x, player.position.y, PLAYER_RADIUS - 6, RED);

  // EndShaderMode();
}

}  // namespace evs

#endif  // PLAYER_H
