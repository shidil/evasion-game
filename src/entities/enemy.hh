#ifndef ENEMY_H
#define ENEMY_H
#include <raylib.h>

#include "../game.hh"

static EnemyType enemy_order[MAX_ENEMIES] = {
    EnemyType::DASHER,  EnemyType::DASHER, EnemyType::DASHER, EnemyType::HOMING,
    EnemyType::SHOOTER, EnemyType::HOMING, EnemyType::HOMING, EnemyType::DASHER,
    EnemyType::SHOOTER, EnemyType::DASHER};
static Color enemy_colors[3] = {DARKGREEN, BLUE, VIOLET};

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
      .state = ActorState::LIVE,
      .fire_rate = BULLET_FIRE_RATE_MIN,
      .shots_fired = 0,
      .shots_per_round = RIFLE_SHOTS_PER_ROUND,
      .reload_timer = 0,
      .trail_pos = {},
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
    if (enemies[i].state != ActorState::DESTRUCT || enemies[i].reload_timer > 0) {
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

/**
 * Render enemy, different enemies are rendered with different shapes/textures.
 * @param { Enemy } enemy actor to render
 */
inline void draw_enemy(Enemy enemy) {
  Color color = enemy.color;
  if (enemy.state == ActorState::RELOADING) {
    color = GetRandomValue(0, 1) ? RED : color;
  }

  switch (enemy.type) {
    case EnemyType::HOMING:
      DrawRectangleLines(enemy.position.x - 10, enemy.position.y - 10, 20, 20, color);
      if (enemy.reload_timer > 0) {
        // draw a blast radius indicator as a circle based on current progress towars
        // blast from reload timer calculated as a percentage function
        auto blast_radi =
            (1 - (enemy.reload_timer / ENEMY_RELOAD_TIMER)) * HOMER_BLAST_RADIUS;
        DrawCircleLines(enemy.position.x, enemy.position.y, blast_radi, ORANGE);
      }
      break;
    case EnemyType::DASHER: {
      DrawRectangleLines(enemy.position.x - 10, enemy.position.y - 10, 20, 20, color);
      break;
    }
    default:
      DrawRectangleLines(enemy.position.x - 10, enemy.position.y - 10, 20, 20, color);
      break;
  }

  if (enemy.state == ActorState::LIVE && enemy.velocity.x != 0 && enemy.velocity.y != 0) {
    // Draw movement trail
    for (int i = MAX_ENEMY_TRAIL - 1; i >= 0; i -= 1) {
      auto trail_pos = enemy.trail_pos[i];
      color.a /= 2;
      auto width = 20 - MAX_ENEMY_TRAIL + i;
      DrawRectangleLines(trail_pos.x - (width / 2), trail_pos.y - (width / 2), width,
                         width, color);
    }
  }
}

}  // namespace evs

#endif  // ENEMY_H
