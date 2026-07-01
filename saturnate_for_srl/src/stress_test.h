#ifndef STRESS_TEST_H
#define STRESS_TEST_H

#include <sl_def.h>
#include "game_state.h"

extern GameState stressTestState;

void playStressTestModel(uint32_t modelID);
void pasteCurrentInstance();

void initStressTest();
GameStatus updateStressTest();
void drawStressTest();

#endif //STRESS_TEST_H