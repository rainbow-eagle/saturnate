#ifndef MODEL_VIEWER_H
#define MODEL_VIEWER_H

#include <sl_def.h>
#include "game_state.h"


extern GameState modelViewerState;

void playModelViewerModel(uint32_t modelID);
void doNothing();

void initModelViewer();
GameStatus updateModelViewer();
void drawModelViewer();

#endif //MODEL_VIEWER_H