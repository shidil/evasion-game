#ifndef ENEMY_H
#define ENEMY_H
#include <raylib.h>

#include "../game.hh"
#include "../screens/screens.h"

static EnemyType enemy_order[MAX_ENEMIES] = {
    EnemyType::DASHER, EnemyType::DASHER, EnemyType::DASHER,  EnemyType::SHOOTER,
    EnemyType::HOMING, EnemyType::HOMING, EnemyType::SHOOTER, EnemyType::DASHER,
    EnemyType::HOMING, EnemyType::SHOOTER};
static Color enemy_colors[3] = {DARKGREEN, BLUE, VIOLET};
static Color enemy_telegraph_colors[2] = {{235, 80, 80, 100}, {255, 0, 0, 80}};

namespace evs {
inline Enemy create_enemy(int total_spawned) {
  // Avoid overlapping enemy and player, as well as other enemies
  // Keep min x distance from other enemies and player
  // Some randonmess in fire rate and other timings
  // Enemy spawn probability
  float x, y;
  EnemyType type = enemy_order[total_spawned % MAX_ENEMIES];
  if (total_spawned == 0) {
    // First enemy is fixed
    x = (SCREEN_WIDTH / 2) + GetRandomValue(-100, 100);
    y = 100 + GetRandomValue(-25, 25);
  } else {
    // Spawn shooters close to edges
    if (type == EnemyType::SHOOTER || type == EnemyType::DASHER) {
      auto left_align = GetRandomValue(0, 1);
      x = left_align ? 50 : SCREEN_WIDTH - 50;
      y = GetRandomValue(50, SCREEN_HEIGHT - 50);
    } else {
      x = GetRandomValue(50, SCREEN_WIDTH - 50);
      y = GetRandomValue(50, SCREEN_HEIGHT - 50);
    }
  }

  return {
      .position = {x, y},
      .color = enemy_colors[GetRandomValue(0, 2)],
      .velocity = {0, 0},
      .type = type,
      .state = ActorState::SPAWNING,
      .fire_rate = BULLET_FIRE_RATE_MIN,
      .shots_fired = 0,
      .shots_per_round = RIFLE_SHOTS_PER_ROUND,
      .timer = ENEMY_SPAWN_TELEGRAPH,
      .trail_pos = {},
      .rotation = 0,
  };
}

inline std::vector<int> check_enemy_collisions(Player player,
                                               std::vector<Enemy> enemies) {
  std::vector<int> out;
  for (int i = 0; i < enemies.size(); i++) {
    if (enemies[i].state == ActorState::DEAD) {
      continue;
    }

    Rectangle enemy_rect = {
        .x = enemies[i].position.x - 10,
        .y = enemies[i].position.y - 10,
        .width = 20,
        .height = 20,
    };
    if (CheckCollisionCircleRec(player.position, PLAYER_RADIUS, enemy_rect)) {
      out.push_back(i);
    }
  }

  return out;
}

inline bool check_homer_blast_collisions(Player player, std::vector<Enemy> enemies) {
  for (int i = 0; i < enemies.size(); i++) {
    // skip non blast mode enemies. blast mode enemies will have state DESTRUCT
    if (enemies[i].state != ActorState::DESTRUCT || enemies[i].timer > 0) {
      continue;
    }

    // check if player hit box(circle) is colliding with blast/explosion circle
    if (CheckCollisionCircles(player.position, PLAYER_RADIUS, enemies[i].position,
                              HOMER_BLAST_RADIUS)) {
      return true;
    }
  }

  return false;
}

inline std::vector<int> check_enemy_enemy_collisions(std::vector<Enemy> enemies) {
  std::vector<int> out;
  int enemies_count = enemies.size();

  for (int i = 0; i < enemies_count; i++) {
    Rectangle enemy_rect_1 = {
        .x = enemies[i].position.x - 10,
        .y = enemies[i].position.y - 10,
        .width = 20,
        .height = 20,
    };
    for (int j = i + 1; j < enemies_count; j++) {
      Rectangle enemy_rect_2 = {
          .x = enemies[j].position.x - 10,
          .y = enemies[j].position.y - 10,
          .width = 20,
          .height = 20,
      };
      // if enemy i and j collides, they both die, bonus score!!
      if (CheckCollisionRecs(enemy_rect_1, enemy_rect_2)) {
        out.push_back(i);
        out.push_back(j);
      }
    }
  }

  return out;
}

Color homer_palette[1][3] = {{RED, BLUE, WHITE}};

void draw_homing_enemy(Enemy homer, Color color) {
  auto origin = homer.position;

  for (int i = 0; i < 3; i++) {
    auto col = homer_palette[0][i];
    Color fading_color = {col.r, col.g, col.b,
                          static_cast<unsigned char>(col.a / ((i + 1) * 1.5))};
    DrawCircleLines(origin.x, origin.y, HOMER_SIZE - 10 + i, fading_color);
    DrawPolyLines(origin, 4, HOMER_SIZE - i, 45 + homer.rotation, fading_color);
    DrawPolyLines(origin, 4, HOMER_SIZE - i, 90 + homer.rotation, fading_color);
    DrawPolyLines(origin, 4, HOMER_SIZE - i, 45 + homer.rotation, fading_color);
    DrawPolyLines(origin, 4, HOMER_SIZE - i, 90 + homer.rotation, fading_color);
    DrawPolyLines(origin, 4, HOMER_SIZE - i, 45 + homer.rotation, fading_color);
    DrawPolyLines(origin, 4, HOMER_SIZE - i, 90 + homer.rotation, fading_color);
  }
}

void draw_shooter_enemy(Enemy shooter, Vector2 aim, Color color) {}

void draw_dasher_enemy(Enemy dasher, Vector2 aim, Color color) {}

/**
 * Render enemy, different enemies are rendered with different shapes/textures.
 * @param { Enemy } enemy actor to render
 */
inline void draw_enemy(Enemy enemy, Vector2 aim) {
  Color color = enemy.color;
  if (enemy.state == ActorState::RELOADING) {
    color = GetRandomValue(0, 1) ? RED : color;
  }

  if (enemy.state == ActorState::SPAWNING) {
    color = enemy_telegraph_colors[GetRandomValue(0, 1)];
    color.a = 150;
    for (int i = 0; i < 4; i++) {
      color.a -= 50;
      DrawCircleLines(enemy.position.x, enemy.position.y, SHOOTER_SIZE - (i * 3), color);
    }
    return;
  }

  switch (enemy.type) {
    case EnemyType::HOMING:
      draw_homing_enemy(enemy, color);

      // draw a blast radius indicator as a circle based on current progress towars
      // blast from reload timer calculated as a percentage function
      if (enemy.timer > 0) {
        auto percentage = (1 - (enemy.timer / ENEMY_timer));
        auto blast_radi = percentage * HOMER_BLAST_RADIUS;
        DrawCircleLines(enemy.position.x, enemy.position.y, blast_radi, ORANGE);
      }
      break;
    case EnemyType::DASHER: {
      DrawRectangleLines(enemy.position.x - 10, enemy.position.y - 10, DASHER_SIZE,
                         DASHER_SIZE, color);
      if (enemy.state == ActorState::LIVE && enemy.velocity.x != 0 &&
          enemy.velocity.y != 0) {
        // Draw movement trail
        for (int i = MAX_ENEMY_TRAIL - 1; i >= 0; i -= 1) {
          auto trail_pos = enemy.trail_pos[i];
          color.a /= 2;
          auto width = DASHER_SIZE - MAX_ENEMY_TRAIL + i;
          DrawCircleLines(trail_pos.x, trail_pos.y, width / 2, color);
        }
      }
      break;
    }
    case EnemyType::SHOOTER: {
      // Draw a polygon, rotated 45deg
      DrawPolyLines(
          enemy.position, 3, SHOOTER_SIZE - 2, 270 - enemy.rotation + 0,
          {color.r, color.g, color.b, static_cast<unsigned char>(color.a - 50)});
      DrawPolyLines(
          enemy.position, 3, SHOOTER_SIZE - 1, 270 - enemy.rotation + 0,
          {color.r, color.g, color.b, static_cast<unsigned char>(color.a - 100)});
      DrawPolyLines(
          enemy.position, 3, SHOOTER_SIZE, 270 - enemy.rotation + 0,
          {color.r, color.g, color.b, static_cast<unsigned char>(color.a - 150)});

      DrawPolyLines(
          enemy.position, 3, SHOOTER_SIZE - 2, 270 - enemy.rotation - 180,
          {color.r, color.g, color.b, static_cast<unsigned char>(color.a - 50)});
      DrawPolyLines(
          enemy.position, 3, SHOOTER_SIZE - 1, 270 - enemy.rotation - 180,
          {color.r, color.g, color.b, static_cast<unsigned char>(color.a - 100)});
      DrawPolyLines(
          enemy.position, 3, SHOOTER_SIZE, 270 - enemy.rotation - 180,
          {color.r, color.g, color.b, static_cast<unsigned char>(color.a - 150)});
      break;
    }
    default:
      DrawRectangleLines(enemy.position.x - 10, enemy.position.y - 10, 20, 20, color);
      break;
  }
}

}  // namespace evs

#endif  // ENEMY_H
