#ifndef TT_DEBUG_PRINT_H
#define TT_DEBUG_PRINT_H

#include <stdio.h>
#include <sl_def.h>
#include "../core/animation.h"

void ttDebugPause(uint32_t frameCount);
void ttDebugPauseA();
void ttDebugClear();
void ttDebugMsgXY(char* msg, uint16_t x, uint16_t y);
void ttDebugMsgLn(char* msg);
void ttDebugIntXY(int32_t num, uint16_t x, uint16_t y);
void ttDebugIntLn(int32_t num);
void ttDebugFixedXY(FIXED num, uint16_t x, uint16_t y);
void ttDebugFixedLn(FIXED num);
void ttDebugAngleXY(ANGLE ang, uint16_t x, uint16_t y);
void ttDebugAngleLn(ANGLE ang);
void ttDebugMatrixXY(MATRIX matrix, uint16_t x, uint16_t y);
void ttDebugMatrixLn(MATRIX matrix);
// void ttDebugTransformXY(TTTransform* transform, uint16_t x, uint16_t y);
// void ttDebugTransformLn(TTTransform* transform);
// void ttDebugTransformMatrixXY(TTTransform* transform, uint16_t x, uint16_t y);
// void ttDebugTransformMatrixLn(TTTransform* transform);

#endif //TT_DEBUG_PRINT_H