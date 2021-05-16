#ifndef EVASION_H
#define EVASION_H

#include <raylib.h>

#include <vector>

#define SCREEN_WIDTH 540
#define SCREEN_HEIGHT 960
#define FRAME_RATE 60

#define PLAYER_RADIUS 20
#define INITIAL_PLAYER_SHIELDS 3

#define ENEMY_KILL_BONUS 50
#define ENEMY_SELF_KILL_BONUS 100

#define ENEMY_SPAWN_INTERVAL 5

#define BULLET_RADIUS 3
#define BULLET_FIRE_RATE_MIN 20
#define BULLET_FIRE_RATE_MAX 10
#define MAX_BULLETS 100
#define RIFLE_SHOTS_PER_ROUND 25
#define BAZOOKA_SHOTS_PER_ROUND 1
#define BULLET_VELOCITY 5
#define FIRE_RATE_RAMPUP_INTERVAL 300

#define MAX_ENEMIES 10
#define MAX_ENEMY_TRAIL 10

#define DASHER_VELOCITY 8
#define HOMING_VELOCITY 2
#define ENEMY_RELOAD_TIMER 1.5f

#define HOMER_BLAST_RADIUS 60
#define HOMER_BLAST_TRIGGER_DISTANCE 50

#define DASHER_BOUNDS \
  CLITERAL(Rectangle) { 50, 50, SCREEN_WIDTH - 100, SCREEN_HEIGHT - 100 }
#define BULLET_BOUNDS \
  CLITERAL(Rectangle) { -50, -50, SCREEN_WIDTH + 100, SCREEN_HEIGHT + 100 }

enum EnemyType {
  SHOOTER,
  DASHER,
  HOMING,
};

enum ActorState {
  LIVE,
  DEAD,
  RELOADING,
  DESTRUCT,
};

enum WorldState {
  RUNNING,
  PAUSED,
  GAME_OVER,
};

typedef struct {
  Vector2 position;
  Color color;
  Vector2 velocity;
  ActorState state;
} Bullet;

typedef struct {
  Vector2 position;
  Color color;
  Vector2 velocity;
  EnemyType type;
  ActorState state;
  int fire_rate;
  int shots_fired;
  int shots_per_round;
  float reload_timer;
  std::vector<Vector2> trail_pos;
} Enemy;

typedef struct {
  Vector2 position;
  Color color;
  ActorState state;
  int shield;
} Player;

typedef struct {
  Player player;
  std::vector<Enemy> enemies;
  std::vector<Bullet> bullets;
  WorldState state;
} GameWorld;

#endif // EVASION_H
