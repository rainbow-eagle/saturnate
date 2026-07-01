#include <string.h>
#include <stdio.h>
#include <sl_def.h>
#include "print.h"
#include "../core/animation.h"

extern int snprintf(char *str, size_t size, const char *format, ...);

#define NEXT_DEBUG_Y (debugY = debugY>30? 0 : debugY + 1)

uint32_t debugY = 0;

void ttDebugPause(uint32_t frameCount) {
    for(uint32_t i = frameCount; i > 0; i--)
        slSynch();
}

void ttDebugPauseA() {
    bool sleep = TRUE;
    while(sleep) {
        slSynch();
        if(Per_Connect1)
            if(!(Smpc_Peripheral[0].push & PER_DGT_TA))
                sleep = FALSE;
    }
}

void ttDebugClear() {
    char msg[40] = "                                        ";
    for(int32_t y = 40; y >= 0; y--)
        slPrint(msg, slLocate(0, y));
    debugY = 0;
}

void ttDebugMsgXY(char* str, uint16_t x, uint16_t y) {
    char msg[40];
    snprintf(msg, sizeof(msg), "%-39s", str);
    slPrint(msg, slLocate(x,y));
}

void ttDebugMsgLn(char* str) {
    ttDebugMsgXY(str, 0, NEXT_DEBUG_Y);
}

void ttDebugIntXY(int32_t num, uint16_t x, uint16_t y) {
    char msg[20];
    snprintf(msg, sizeof(msg), "%-39d", num);
    slPrint(msg, slLocate(x, y));
}

void ttDebugIntLn(int32_t num) {
    ttDebugIntXY(num, 0, NEXT_DEBUG_Y);
}

void ttDebugFixedXY(FIXED num, uint16_t x, uint16_t y) {
    slPrintFX(num, slLocate(x, y));
}

void ttDebugFixedLn(FIXED num) {
    ttDebugFixedXY(num, 0, NEXT_DEBUG_Y);
}

void ttDebugAngleXY(ANGLE ang, uint16_t x, uint16_t y) {
    ttDebugIntXY(slAng2Dec(ang), x, y);
}

void ttDebugAngleLn(ANGLE ang) {
    ttDebugAngleXY(ang, 0, NEXT_DEBUG_Y);
}

void ttDebugMatrixXY(MATRIX matrix, uint16_t x, uint16_t y) {
    slPrintMatrix(matrix, slLocate(x, y));
}

void ttDebugMatrixLn(MATRIX matrix) {
    ttDebugMatrixXY(matrix, 0, debugY);
    debugY += 4;
}

// void ttDebugTransformMatrixXY(TTTransform* transform, uint16_t x, uint16_t y) {
//     	slPushUnitMatrix();
// 		{
//             slTranslate(transform->xOffset, transform->yOffset, transform->zOffset);
//             slRotX(transform->xAngle);
//             slRotY(transform->yAngle);
//             slRotZ(transform->zAngle);
//             MATRIX matrix;
//             slGetMatrix(matrix);
//             ttDebugMatrixXY(matrix, x, y);
// 		}
// 		slPopMatrix();
// }

// void ttDebugTransformMatrixLn(TTTransform* transform) {
//     ttDebugTransformMatrixXY(transform, 0, debugY);
//     debugY = debugY>26? 0 : debugY+4;
// }

// void ttDebugTransformXY(TTTransform* transform, uint16_t x, uint16_t y) {
//     ttDebugFixedLn(transform->xOffset);
//     ttDebugFixedLn(transform->yOffset);
//     ttDebugFixedLn(transform->zOffset);
//     ttDebugAngleLn(transform->xAngle);
//     ttDebugAngleLn(transform->yAngle);
//     ttDebugAngleLn(transform->zAngle);
//     ttDebugIntLn(transform->popCount);
// }

// void ttDebugTransformLn(TTTransform* transform) {
//     ttDebugTransformXY(transform, 0, debugY++);
// }
