#ifndef TT_DEBUG_H
#define TT_DEBUG_H

#include <stdio.h>
#include "jo/sgl_prototypes.h"
#include "../core/animation.h"

void ttDebugMsgXY(char* msg, Uint16 x, Uint16 y);
void ttDebugMsgLn(char* msg);
void ttDebugIntXY(Sint32 num, Uint16 x, Uint16 y);
void ttDebugIntLn(Sint32 num);
void ttDebugFixedXY(FIXED num, Uint16 x, Uint16 y);
void ttDebugFixedLn(FIXED num);
void ttDebugAngleXY(ANGLE ang, Uint16 x, Uint16 y);
void ttDebugAngleLn(ANGLE ang);
void ttDebugMatrixXY(MATRIX matrix, Uint16 x, Uint16 y);
void ttDebugMatrixLn(MATRIX matrix);
void ttDebugTransformXY(TTTransform* transform, Uint16 x, Uint16 y);
void ttDebugTransformLn(TTTransform* transform);
void ttDebugTransformMatrixXY(TTTransform* transform, Uint16 x, Uint16 y);
void ttDebugTransformMatrixLn(TTTransform* transform);

#endif //TT_DEBUG_H
