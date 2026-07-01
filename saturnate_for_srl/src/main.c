#include <stdio.h>
#include <string.h>
#include <sgl.h>
#include <sega_sys.h>
#include "core/memory.h"
#include "pool_engine/file.h"
#include "game_state.h"
#include "base_viewer.h"
#include "model_viewer.h"
#include "stress_test.h"
#include "debug/print.h"

GameState* currentState;

//tmp function, should be improved and stored better (and deal with framerate correction)
//Also, weird that I have to "know" about the textures in main.
void initSystem() {
    volatile uint8_t* areaCodePtr = (uint8_t*) TT_AREA_CODE_REGISTER;
    enum tvsz resolution;
    if(*areaCodePtr == SMPC_AREA_EU_PAL || *areaCodePtr == SMPC_AREA_AJ_PAL)
        resolution = TV_320x256;
    else
        resolution = TV_320x240;
    TEXTURE* textures = initPoolsAndGetTexturePtr();
    slInitSystem(resolution, textures, 1);
}

extern const void* SortList;
extern const void* Zbuffer;
extern const void* Pbuffer;
extern const void* CLOfstBuf;
extern const void* SpriteBuf;
extern const void* CommandBuf;

void ss_main(void) {
	initSystem();
	slZdspLevel(3);
	ttInitFileSystem();

// slPrintHex((uint32_t) SortList, slLocate(2, 10));
// slPrintHex((uint32_t) Zbuffer, slLocate(2, 11));
// slPrintHex((uint32_t) SpriteBuf, slLocate(2, 12));
// slPrintHex((uint32_t) Pbuffer, slLocate(2, 13));
// slPrintHex((uint32_t) CLOfstBuf, slLocate(2, 14));
// slPrintHex((uint32_t) CommandBuf, slLocate(2, 15));

    currentState = &modelViewerState;
    currentState->init();

	while(-1) {
        switch(currentState->update()) {
            case GAME_CONTINUE:
                currentState->draw();
                break;
            case GAME_NEXT:
                currentState = (currentState == &modelViewerState)? &stressTestState : &modelViewerState;
                currentState->init();
        }
		slSynch();
	}
}
