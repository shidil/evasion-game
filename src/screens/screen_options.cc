#include "../resources.hh"
#include "raylib.h"
#include "screens.h"

//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
// Options Screen Functions Definition
//----------------------------------------------------------------------------------

// Options Screen Initialization logic
void InitOptionsScreen(void) {
  // TODO: Initialize OPTIONS screen variables here!
  frames_counter = 0;
  finishScreen = 0;
}

// Options Screen Update logic
void UpdateOptionsScreen(void) {
  // TODO: Update OPTIONS screen variables here!
}

// Options Screen Draw logic
void DrawOptionsScreen(void) {
  // TODO: Draw OPTIONS screen here!
}

// Options Screen Unload logic
void UnloadOptionsScreen(void) {
  // TODO: Unload OPTIONS screen variables here!
}

// Options Screen should finish?
int FinishOptionsScreen(void) { return finishScreen; }
