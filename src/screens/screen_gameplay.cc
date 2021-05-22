#include <stddef.h>
#include <stdint.h>

#include <cmath>
#include <string>

#include "../entities/bullet.hh"
#include "../entities/enemy.hh"
#include "../entities/player.hh"
#include "../game.hh"
#include "../resources.hh"
#include "../utils/math.hh"
#include "../utils/settings.hh"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "screens.h"
#define MAX_STARS 250

// Spot data
typedef struct {
  Vector2 pos;
  Vector2 vel;
  float inner;
  float radius;

  // Shader locations
  unsigned int posLoc;
  unsigned int innerLoc;
  unsigned int radiusLoc;
} Spot;

// Stars in the star field have a position and velocity
typedef struct Star {
  Vector2 pos;
  Vector2 vel;
} Star;

//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------

// Gameplay screen global variables
static float score;
static GameWorld game_world;
static int total_enemies_spawned;
static float high_score;
Star stars[MAX_STARS] = {0};

//----------------------------------------------------------------------------------
// Gameplay Screen Functions Definition
//----------------------------------------------------------------------------------

void UpdateStar(Star *s);
void ResetStar(Star *s);

void reset_game_world() {
  score = 0;
  total_enemies_spawned = 0;
  frames_counter = 0;

  // screen center
  Vector2 player_pos = {.x = SCREEN_WIDTH / 2, .y = SCREEN_HEIGHT - 200};

  std::vector<Bullet> bullets;
  std::vector<Enemy> enemies;

  game_world = {
      .player = evs::create_player(player_pos),
      .enemies = enemies,
      .bullets = bullets,
      .state = WorldState::RUNNING,
      .wave_timer = 0.2f,
  };
}

// Gameplay Screen Initialization logic
void InitGameplayScreen(void) {
  frames_counter = 0;
  finishScreen = 0;
  high_score = evs::read_high_score();
  reset_game_world();
  game_world.state = WorldState::PAUSED;

  for (int n = 0; n < MAX_STARS; n++) ResetStar(&stars[n]);

  // Progress all the stars on, so they don't all start in the centre
  for (int m = 0; m < SCREEN_WIDTH / 2.0; m++) {
    for (int n = 0; n < MAX_STARS; n++) UpdateStar(&stars[n]);
  }

  PlayMusicStream(battle_music);
}

std::vector<Bullet> update_bullets(std::vector<Bullet> &bullets) {
  std::vector<Bullet> updated;
  for (auto bullet : bullets) {
    if (CheckCollisionPointRec(bullet.position, BULLET_BOUNDS)) {
      bullet.position.x += bullet.velocity.x;
      bullet.position.y += bullet.velocity.y;
      updated.push_back(bullet);
    }
  }

  return updated;
}

// Gameplay Screen Update logic
void UpdateGameplayScreen(void) {
  UpdateMusicStream(battle_music);

  auto is_game_paused = game_world.state == WorldState::PAUSED;
  auto is_game_running = game_world.state == WorldState::RUNNING;
  auto is_game_over = game_world.state == WorldState::GAME_OVER;

  frames_counter++;

  // basic score adding mechanism
  if (is_game_running) {
    score += 0.20f;
    if (game_world.wave_timer > 0) {
      game_world.wave_timer -= GetFrameTime();
      game_world.wave_timer = std::max(game_world.wave_timer, 0.0f);
    }
  }

  //----------------------------------------------------------------------------------
  // Input handling
  //----------------------------------------------------------------------------------
  auto current_gesture = GetGestureDetected();
  auto touch_position = GetTouchPosition(0);
  auto has_tapped = current_gesture == GESTURE_TAP;

  // Reset game on tap after game over screen shows
  if (is_game_over && has_tapped && frames_counter > 100) {
    reset_game_world();
  }

  // Start game after tapping on screen when in pause state
  if (is_game_paused && has_tapped) {
    game_world.state = WorldState::RUNNING;
  }

  // Tapping anywhere will teleport player to that position
  if (is_game_running && game_world.player.state == ActorState::LIVE && has_tapped) {
    PlaySoundMulti(teleport_sfx);
    game_world.player.position.x = touch_position.x;
    game_world.player.position.y = touch_position.y;
  }

  //----------------------------------------------------------------------------------
  // Collision detection and consequences
  //----------------------------------------------------------------------------------
  // If player collides with bullet, shield loss for player
  if (evs::check_bullet_collisions(game_world.player, game_world.bullets)) {
    game_world.player.shield -= 1;
    PlaySoundMulti(hit_sfx);
  }

  // Check if player is caught in blast radius of a homer enemy
  if (evs::check_homer_blast_collisions(game_world.player, game_world.enemies)) {
    game_world.player.shield -= 1;
    PlaySoundMulti(hit_sfx);
  }

  // Player collisions with enemies
  auto collided_enemies =
      evs::check_enemy_collisions(game_world.player, game_world.enemies);

  if (collided_enemies.size()) {
    for (auto idx : collided_enemies) {
      // If enemy is reloading, kill enemy, otherwise game over for player
      if (game_world.enemies[idx].state != ActorState::RELOADING) {
        game_world.player.shield -= 1;
        PlaySoundMulti(hit_sfx);
      }
      game_world.enemies[idx].state = ActorState::DEAD;
    }
  }

  // When player is out of shields and get hit, game over
  if (game_world.player.shield < 0 && game_world.player.state != ActorState::DEAD) {
    game_world.player.state = ActorState::DEAD;
    game_world.state = WorldState::GAME_OVER;
    frames_counter = 0;

    // Save score
    if (score > high_score) {
      evs::set_high_score(score);
      high_score = score;
    }
  }

  // Enemy-enemy collisions, both enemies die, player receives bonus score
  // collided_enemies = check_enemy_enemy_collisions(game_world.enemies);

  // if (collided_enemies.size()) {
  //   for (auto idx : collided_enemies) {
  //     game_world.enemies[idx].state = ActorState::DEAD;
  //     score += ENEMY_SELF_KILL_BONUS;
  //     // TODO: PlaySound(bonus_score_sfx);
  //   }
  // }

  //----------------------------------------------------------------------------------
  // Enemy behaviors
  //----------------------------------------------------------------------------------
  if (game_world.state == WorldState::RUNNING) {
    int enemy_count_now = game_world.enemies.size();
    bool should_spawn = game_world.wave_timer == 0 && enemy_count_now < MAX_ENEMIES;
    Enemy new_enemy;

    if (should_spawn) {
      new_enemy = evs::create_enemy(total_enemies_spawned);

      for (int i = 0; i < enemy_count_now; i++) {
        if (evs::distance_2d(game_world.enemies[i].position, new_enemy.position) <
            ENEMY_SPAWN_MIN_DISTANCE) {
          should_spawn = false;
        }
      }
    }

    // Spawn new enemies if there are less enemies than MAX_ENEMIES
    // and after a set amount of interval time
    if (should_spawn) {
      game_world.enemies.push_back(new_enemy);
      enemy_count_now += 1;
      total_enemies_spawned += 1;
      game_world.wave_timer = ENEMY_SPAWN_INTERVAL;
    }

    // update enemy, shoot, dash, follow
    for (int i = 0; i < enemy_count_now; i++) {
      Enemy *enemy = &game_world.enemies[i];

      // Dead enemies can't shoot or dash
      if (enemy->state == ActorState::DEAD) {
        continue;
      }

      // Enemy reload timer decrement if it is active
      if (enemy->timer >= 0) {
        enemy->timer -= GetFrameTime();

        if (enemy->timer <= 0) {
          // spawning enemies go live
          if (enemy->state == ActorState::SPAWNING) {
            enemy->state = ActorState::LIVE;
          }

          // shooters and dashers gets back to their business
          if (enemy->state == ActorState::RELOADING) {
            enemy->state = ActorState::LIVE;
          }
          // explode suicide homer
          if (enemy->state == ActorState::DESTRUCT) {
            enemy->state = ActorState::DEAD;
            // TODO: play explosion sound effect/trigger vfx
            PlaySoundMulti(boom_sfx);
            continue;
          }
        }
      }

      if (enemy->state != ActorState::LIVE) {
        continue;
      }

      switch (enemy->type) {
        case EnemyType::SHOOTER: {
          // enemy can shoot at set intervals (based on enemy.fire_rate)
          if (frames_counter == 0 || frames_counter % enemy->fire_rate == 0) {
            if (game_world.bullets.size() < MAX_BULLETS) {
              enemy->shots_fired += 1;
              PlaySoundMulti(shoot_sfx);
              game_world.bullets.push_back(evs::create_bullet(*enemy, game_world.player));
            }
            // Set enemy state to RELOADING after x amount of bullets
            if (enemy->shots_fired >= enemy->shots_per_round) {
              enemy->shots_fired = 0;
              enemy->state = ActorState::RELOADING;
              enemy->timer = ENEMY_timer;
            }
          }

          // Increase fire rate at set interval
          if (frames_counter % FIRE_RATE_RAMPUP_INTERVAL == 0) {
            enemy->fire_rate = std::max(enemy->fire_rate - 1, BULLET_FIRE_RATE_MAX);
          }
          break;
        }
        case EnemyType::DASHER: {
          // TODO: tweak bound rect, may be check enemy rect center point
          // inside dasher bounds?
          Rectangle enemy_rect = {
              .x = enemy->position.x - 10,
              .y = enemy->position.y - 10,
              .width = 20,
              .height = 20,
          };

          // skip enemy that is already dashing
          if (enemy->velocity.x == 0 && enemy->velocity.y == 0) {
            auto vel = evs::get_homing_velocity(game_world.player.position,
                                                enemy->position, DASHER_VELOCITY);
            enemy->velocity.x = vel.x;
            enemy->velocity.y = vel.y;
          }
          // check dasher bounds, if inside continue to move, or stop moving
          else if (!CheckCollisionRecs(DASHER_BOUNDS, enemy_rect)) {
            enemy->velocity.x = 0;
            enemy->velocity.y = 0;
            enemy->state = ActorState::RELOADING;
            enemy->timer = ENEMY_timer;
          }
          break;
        }
        case EnemyType::HOMING: {
          auto vel = evs::get_homing_velocity(game_world.player.position, enemy->position,
                                              HOMING_VELOCITY);
          enemy->velocity.x = vel.x;
          enemy->velocity.y = vel.y;

          // If homer is at a set distance from player, trigger explosion with a set blast
          // radius
          auto distance =
              std::abs(Vector2Distance(game_world.player.position, enemy->position));
          if (distance <= HOMER_BLAST_TRIGGER_DISTANCE) {
            enemy->state = ActorState::DESTRUCT;
            enemy->timer = ENEMY_timer;
            enemy->trail_pos.clear();
          }

          break;
        }
        default:
          break;
      }

      // store enemy current pos to it's trail
      if (enemy->trail_pos.size() == MAX_ENEMY_TRAIL) {
        enemy->trail_pos.erase(enemy->trail_pos.begin());
      }
      enemy->trail_pos.push_back(enemy->position);

      // Moves enemy with respect to it's velocity and direction
      enemy->position.x += enemy->velocity.x;
      enemy->position.y += enemy->velocity.y;

      enemy->rotation++;
      if (enemy->rotation > 360) {
        enemy->rotation = 0;
      }
    }

    // Remove dead enemies
    std::vector<Enemy> updated_list;
    for (auto enemy : game_world.enemies) {
      // TODO: remove enemy after a delay
      if (enemy.state != ActorState::DEAD) {
        updated_list.push_back(enemy);
      }
    }
    game_world.enemies = updated_list;

    // bullets update, remove out of screen bullets
    game_world.bullets = update_bullets(game_world.bullets);
  }

  // Move the stars, resetting them if the go offscreen
  for (int n = 0; n < MAX_STARS; n++) UpdateStar(&stars[n]);
}

Color border_colors[2] = {{10, 19, 47, 200}, {11, 12, 32, 200}};

// Gameplay Screen Draw logic
void DrawGameplayScreen(void) {
  ClearBackground({4, 10, 14, 255});

  // BeginShaderMode(shaders[PostproShader::FX_BLOOM]);
  DrawTexture(background, 0, 0, (Color){255, 255, 255, 25});
  // EndShaderMode();

  // Draw stars and bobs
  for (int n = 0; n < MAX_STARS; n++) {
    // Single pixel is just too small these days!
    DrawRectangle((int)stars[n].pos.x, (int)stars[n].pos.y, 2, 2, {255, 255, 255, 25});
  }

  //----------------------------------------------------------------------------------
  // Draw game world
  //----------------------------------------------------------------------------------

  // debug dasher bounds/wall
  unsigned char alpha = 50;
  for (int i = 1; i < 5; i++) {
    auto gap = 10 + (i * 5) + (frames_counter % 2);
    Color color = border_colors[i % 2];

    alpha += (i * 20);
    color.a = alpha;
    DrawRectangleLines(gap, gap, SCREEN_WIDTH - (gap * 2), SCREEN_HEIGHT - (gap * 2),
                       color);
    DrawRectangleLines(gap - 1, gap - 1, SCREEN_WIDTH - (gap * 2),
                       SCREEN_HEIGHT - (gap * 2), color);
    // DrawRectangleLinesEx(DASHER_BOUNDS, 1, {100, 100, 100, 25});
    // DrawRectangleLinesEx(DASHER_BOUNDS, 2, {100, 100, 100, 10});
    // DrawRectangleLinesEx(DASHER_BOUNDS, 3, {100, 100, 100, 5});
  }

  // bullets/objects
  for (int i = 0; i < game_world.bullets.size(); i++) {
    evs::draw_bullet(game_world.bullets[i]);
  }

  // actors and enemies
  if (game_world.state != WorldState::GAME_OVER) {
    evs::draw_player(game_world.player);
  }

  for (int i = 0; i < game_world.enemies.size(); i++) {
    evs::draw_enemy(game_world.enemies[i], game_world.player.position);
  }

  // particle effects

  // game over
  if (game_world.player.state == DEAD) {
    DrawText("GAME OVER", (SCREEN_WIDTH / 2) - 115, (SCREEN_HEIGHT / 2) - 200, 40,
             ORANGE);
    if (score >= high_score) {
      DrawText("HIGH SCORE REACHED!", (SCREEN_WIDTH / 2) - 170, (SCREEN_HEIGHT / 2) - 135,
               30, PURPLE);
    }

    if (frames_counter > 100) {
      DrawText("TAP TO PLAY AGAIN", (SCREEN_WIDTH / 2) - 100, (SCREEN_HEIGHT / 2) + 100,
               20, YELLOW);
    }
  }

  // Logo and pause screen
  if (game_world.state == WorldState::PAUSED) {
    DrawText("QUANTUM EVASION", (SCREEN_WIDTH / 2) - 150, (SCREEN_HEIGHT / 2) - 200, 30,
             PURPLE);
    DrawText("TAP TO PLAY", (SCREEN_WIDTH / 2) - 75, (SCREEN_HEIGHT / 2) + 0, 20, YELLOW);

    // Show high score before starting game
    if (score == 0 && high_score > 0) {
      std::string score_text = "Highest Score: ";
      score_text.append(TextFormat("%02.00f", high_score));
      DrawText(score_text.data(), 50, SCREEN_HEIGHT - 50, 14, GRAY);
    }
  }

  // shield count
  if (game_world.state == WorldState::RUNNING) {
    std::string shield_string = "Shields: ";
    shield_string.append(std::to_string(std::max(0, game_world.player.shield)));
    DrawText(shield_string.data(), 20, 10, 20, GRAY);
  }

  // Score
  if (game_world.state != WorldState::PAUSED) {
    std::string score_text = "Score: ";
    score_text.append(TextFormat("%02.00f", score));
    DrawText(score_text.data(), SCREEN_WIDTH - 120, 10, 20, ORANGE);
  }

  // DrawFPS(10, 10);
}

// Gameplay Screen Unload logic
void UnloadGameplayScreen(void) {}

// Gameplay Screen should finish?
int FinishGameplayScreen(void) { return finishScreen; }

void ResetStar(Star *s) {
  s->pos = (Vector2){(float)GetRandomValue(0, SCREEN_WIDTH),
                     (float)GetRandomValue(0, SCREEN_HEIGHT)};

  do {
    s->vel.x = (float)GetRandomValue(-200, 200) / 100.0f;
    s->vel.y = (float)GetRandomValue(-200, 200) / 100.0f;

  } while (!(fabs(s->vel.x) + (fabs(s->vel.y) > 1)));

  s->pos = Vector2Add(s->pos, Vector2Multiply(s->vel, (Vector2){8.0f, 8.0f}));
}

void UpdateStar(Star *s) {
  s->pos = Vector2Add(s->pos, s->vel);

  if ((s->pos.x < 0) || (s->pos.x > GetScreenWidth()) || (s->pos.y < 0) ||
      (s->pos.y > GetScreenHeight())) {
    ResetStar(s);
  }
}
