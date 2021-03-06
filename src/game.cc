#include "game.hh"

#include <iostream>

#include "raylib.h"
#include "resources.hh"
#include "screens/screens.h"  // NOTE: Defines global variable: currentScreen
#include "utils/data-loader.hh"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#define MAX_POSTPRO_SHADERS 0

GameScreen currentScreen = GameScreen::LOGO;
int frames_counter;
int finishScreen;

Camera2D camera = {0};

Font font = {0};
Music music = {0};
Sound fxCoin = {0};
Texture2D background;
Sound teleport_sfx;
Sound boom_sfx;
Sound shoot_sfx;
Sound hit_sfx;
Music battle_music;
Shader shaders[MAX_POSTPRO_SHADERS] = {};

//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------
const int screenWidth = 540;
const int screenHeight = 960;

// Required variables to manage screen transitions (fade-in, fade-out)
static float transAlpha = 0.0f;
static bool onTransition = false;
static bool transFadeOut = false;
static int transFromScreen = -1;
static int transToScreen = -1;

// NOTE: Some global variables that require to be visible for all screens,
// are defined in screens.h (i.e. currentScreen)

//----------------------------------------------------------------------------------
// Local Functions Declaration
//----------------------------------------------------------------------------------
static void ChangeToScreen(GameScreen screen);  // No transition effect

static void TransitionToScreen(GameScreen screen);
static void UpdateTransition(void);
static void DrawTransition(void);

static void UpdateDrawFrame(void);  // Update and Draw one frame

//----------------------------------------------------------------------------------
// Main entry point
//----------------------------------------------------------------------------------
int main(void) {
  // Initialization (Note windowTitle is unused on Android)
  //---------------------------------------------------------
  camera.rotation = 0.0f;
  camera.zoom = 1.0f;

  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(screenWidth, screenHeight, "Quantum Evasion");

  // Global data loading (assets that must be available in all screens, i.e. fonts)
  InitAudioDevice();

  background = evs::load_texture("bg-grid-dark.png");

  // NOTE: Defining 0 (NULL) for vertex shader forces usage of internal default vertex
  // load shaders
  // shaders[0] = evs::load_fragment_shader("bloom");

  // music = evs::load_music("ambient.ogg");
  font = GetFontDefault();
  fxCoin = evs::load_sound("coin.wav");
  teleport_sfx = evs::load_sound("teleport.ogg");
  boom_sfx = evs::load_sound("explosion.ogg");
  shoot_sfx = evs::load_sound("laserpew.ogg");
  hit_sfx = evs::load_sound("hit.ogg");

  battle_music = evs::load_music("battle-theme.ogg");
  battle_music.looping = true;
  SetMusicVolume(battle_music, 0.85f);
  SetSoundVolume(teleport_sfx, 0.1f);
  SetSoundVolume(shoot_sfx, 0.00f);
  SetSoundVolume(hit_sfx, 0.70f);
  SetSoundVolume(boom_sfx, 1.0f);

  // SetMusicVolume(music, 1.0f);
  // PlayMusicStream(music);

  // Setup and Init first screen
  // currentScreen = LOGO;
  // InitLogoScreen();
  currentScreen = GAMEPLAY;
  InitGameplayScreen();

#if defined(PLATFORM_WEB)
  emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
  SetTargetFPS(FRAME_RATE);  // Set our game to run at 60 frames-per-second
  //--------------------------------------------------------------------------------------

  // Main game loop
  while (!WindowShouldClose())  // Detect window close button or ESC key
  {
    UpdateDrawFrame();
  }
#endif

  // De-Initialization
  //--------------------------------------------------------------------------------------

  // Unload current screen data before closing
  switch (currentScreen) {
    case LOGO:
      UnloadLogoScreen();
      break;
    case TITLE:
      UnloadTitleScreen();
      break;
    case GAMEPLAY:
      UnloadGameplayScreen();
      break;
    case ENDING:
      UnloadEndingScreen();
      break;
    default:
      break;
  }

  // Unload all global loaded data (i.e. fonts) here!
  UnloadFont(font);
  // UnloadMusicStream(music);
  UnloadSound(fxCoin);
  UnloadMusicStream(battle_music);
  UnloadSound(teleport_sfx);
  UnloadSound(boom_sfx);
  UnloadSound(shoot_sfx);
  UnloadSound(hit_sfx);
  UnloadTexture(background);
  // Unload all postpro shaders
  for (int i = 0; i < MAX_POSTPRO_SHADERS; i++) UnloadShader(shaders[i]);

  CloseAudioDevice();  // Close audio context

  CloseWindow();  // Close window and OpenGL context
  //--------------------------------------------------------------------------------------

  return 0;
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------

// Change to next screen, no transition
static void ChangeToScreen(GameScreen screen) {
  // Unload current screen
  switch (currentScreen) {
    case LOGO:
      UnloadLogoScreen();
      break;
    case TITLE:
      UnloadTitleScreen();
      break;
    case GAMEPLAY:
      UnloadGameplayScreen();
      break;
    case ENDING:
      UnloadEndingScreen();
      break;
    default:
      break;
  }

  // Init next screen
  switch (screen) {
    case LOGO:
      InitLogoScreen();
      break;
    case TITLE:
      InitTitleScreen();
      break;
    case GAMEPLAY:
      InitGameplayScreen();
      break;
    case ENDING:
      InitEndingScreen();
      break;
    default:
      break;
  }

  currentScreen = screen;
}

// Define transition to next screen
static void TransitionToScreen(GameScreen screen) {
  onTransition = true;
  transFadeOut = false;
  transFromScreen = currentScreen;
  transToScreen = screen;
  transAlpha = 0.0f;
}

// Update transition effect
static void UpdateTransition(void) {
  if (!transFadeOut) {
    transAlpha += 0.05f;

    // NOTE: Due to float internal representation, condition jumps on 1.0f instead
    // of 1.05f For that reason we compare against 1.01f, to avoid last frame loading stop
    if (transAlpha > 1.01f) {
      transAlpha = 1.0f;

      // Unload current screen
      switch (transFromScreen) {
        case LOGO:
          UnloadLogoScreen();
          break;
        case TITLE:
          UnloadTitleScreen();
          break;
        case OPTIONS:
          UnloadOptionsScreen();
          break;
        case GAMEPLAY:
          UnloadGameplayScreen();
          break;
        case ENDING:
          UnloadEndingScreen();
          break;
        default:
          break;
      }

      // Load next screen
      switch (transToScreen) {
        case LOGO:
          InitLogoScreen();
          break;
        case TITLE:
          InitTitleScreen();
          break;
        case GAMEPLAY:
          InitGameplayScreen();
          break;
        case ENDING:
          InitEndingScreen();
          break;
        default:
          break;
      }

      currentScreen = static_cast<GameScreen>(transToScreen);

      // Activate fade out effect to next loaded screen
      transFadeOut = true;
    }
  } else  // Transition fade out logic
  {
    transAlpha -= 0.02f;

    if (transAlpha < -0.01f) {
      transAlpha = 0.0f;
      transFadeOut = false;
      onTransition = false;
      transFromScreen = -1;
      transToScreen = -1;
    }
  }
}

// Draw transition effect (full-screen rectangle)
static void DrawTransition(void) {
  DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, transAlpha));
}

// Update and draw game frame
static void UpdateDrawFrame(void) {
  // Update
  //----------------------------------------------------------------------------------
  UpdateMusicStream(music);  // NOTE: Music keeps playing between screens

  if (!onTransition) {
    switch (currentScreen) {
      case LOGO: {
        UpdateLogoScreen();

        if (FinishLogoScreen()) TransitionToScreen(TITLE);

      } break;
      case TITLE: {
        UpdateTitleScreen();

        if (FinishTitleScreen() == 1)
          TransitionToScreen(OPTIONS);
        else if (FinishTitleScreen() == 2)
          TransitionToScreen(GAMEPLAY);

      } break;
      case OPTIONS: {
        UpdateOptionsScreen();

        if (FinishOptionsScreen()) TransitionToScreen(TITLE);

      } break;
      case GAMEPLAY: {
        UpdateGameplayScreen();

        if (FinishGameplayScreen() == 1) TransitionToScreen(ENDING);
        // else if (FinishGameplayScreen() == 2) TransitionToScreen(TITLE);

      } break;
      case ENDING: {
        UpdateEndingScreen();

        if (FinishEndingScreen() == 1) TransitionToScreen(TITLE);

      } break;
      default:
        break;
    }
  } else
    UpdateTransition();  // Update transition (fade-in, fade-out)
  //----------------------------------------------------------------------------------

  // Draw
  //----------------------------------------------------------------------------------
  BeginDrawing();
  BeginMode2D(camera);

  ClearBackground(RAYWHITE);

  switch (currentScreen) {
    case LOGO:
      DrawLogoScreen();
      break;
    case TITLE:
      DrawTitleScreen();
      break;
    case OPTIONS:
      DrawOptionsScreen();
      break;
    case GAMEPLAY:
      DrawGameplayScreen();
      break;
    case ENDING:
      DrawEndingScreen();
      break;
    default:
      break;
  }

  // Draw full screen rectangle in front of everything
  if (onTransition) DrawTransition();

  // DrawFPS(10, 10);
  EndMode2D();
  EndDrawing();
  //----------------------------------------------------------------------------------
}
