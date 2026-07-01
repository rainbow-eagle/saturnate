#ifndef MODEL_VIEWER_H
#define MODEL_VIEWER_H

#include <jo/sgl_prototypes.h>
#include "game_state.h"


extern GameState modelViewerState;

void playModelViewerModel(Uint32 modelID);
void doNothing();

void initModelViewer();
GameStatus updateModelViewer();
void drawModelViewer();

#endif //MODEL_VIEWER_H