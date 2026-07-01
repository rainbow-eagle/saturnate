#ifndef STRESS_TEST_H
#define STRESS_TEST_H

#include <jo/sgl_prototypes.h>
#include "game_state.h"

extern GameState stressTestState;

void playStressTestModel(Uint32 modelID);
void pasteCurrentInstance();

void initStressTest();
GameStatus updateStressTest();
void drawStressTest();

#endif //STRESS_TEST_H
