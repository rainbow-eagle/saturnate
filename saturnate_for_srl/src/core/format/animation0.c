#include <sl_def.h>
#include "animation0.h"
#include "../file.h"
#include "../memory.h"

void ttLoadMOTMetadata0(char* fileName, TTMOTFileMetadata0* metaData) {
    TTFileStream stream;
    ttOpenFileStream(fileName, &stream);
    TTMOTFileHeader animationSetSpec;

    ttReadBytes(&stream, sizeof(TTMOTFileHeader), (uint8_t*) &animationSetSpec);
    TTAnimationSpec0 animationSpecs[animationSetSpec.animationCount];
    uint32_t size = sizeof(TTAnimationSpec0) * animationSetSpec.animationCount;
    ttReadBytes(&stream, size, (uint8_t*) animationSpecs);
    uint32_t poseCount = 0;
    uint32_t i = animationSetSpec.animationCount;
    TTAnimationSpec0* animationSpecPtr = animationSpecs;
    do
        poseCount += animationSpecPtr++->totalPoseCount;
    while(--i);
    metaData->totalPoseCount = poseCount;
    metaData->animationCount = animationSetSpec.animationCount;
    metaData->meshCount = animationSetSpec.meshCount;

    ttCloseFileStream(&stream);
}

TTAnimationLoadStatus ttLoadMOTAnimation0(char* fileName, TTAnimationBuffer0* animationBuffer) {
    TTFileStream stream;
    ttOpenFileStream(fileName, &stream);
    TTMOTFileHeader* animationSetSpec = (TTMOTFileHeader*) animationBuffer->animationSetSpecPtr;
    TTAnimationSpec0* animationSpecs = (TTAnimationSpec0*) animationBuffer->animationSpecs;

    ttReadBytes(&stream, sizeof(TTMOTFileHeader), (uint8_t*) animationSetSpec);
    uint32_t animationCount = animationSetSpec->animationCount;
    #ifdef TT_DEBUG_MODE
        if(__builtin_expect(animationCount > animationBuffer->maxAnimationSpecCount, 0)) {
            ttCloseFileStream(&stream);
            return TT_ANIMATION_OVERFLOW;
        }
    #endif
    uint32_t size = sizeof(TTAnimationSpec0) * animationCount;
    ttReadBytes(&stream, size, (uint8_t*) animationSpecs);
    size = 0;
    uint32_t i = animationCount;
    do
        size += animationSpecs++->totalPoseCount;
    while(--i);
    #ifdef TT_DEBUG_MODE
         if(__builtin_expect(size * animationSetSpec->meshCount > animationBuffer->maxTransformCount, 0)) {
            ttCloseFileStream(&stream);
            return TT_TRANSFORM_OVERFLOW;
         }
    #endif
    size *= animationSetSpec->meshCount * sizeof(TTTransform);
    ttReadBytes(&stream, size, (uint8_t*) animationBuffer->animations);

    ttCloseFileStream(&stream);
    return OK;
}
