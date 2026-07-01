#include <jo/sgl_prototypes.h>
#include "animation0.h"
#include "../file.h"
#include "../memory.h"

void ttLoadMOTMetadata0(char* fileName, TTMOTFileMetadata0* metaData) {
    TTFileStream stream;
    ttOpenFileStream(fileName, &stream);
    TTMOTFileHeader animationSetSpec;

    ttReadBytes(&stream, sizeof(TTMOTFileHeader), (Uint8*) &animationSetSpec);
    TTAnimationSpec0 animationSpecs[animationSetSpec.animationCount];
    Uint32 size = sizeof(TTAnimationSpec0) * animationSetSpec.animationCount;
    ttReadBytes(&stream, size, (Uint8*) animationSpecs);
    Uint32 poseCount = 0;
    Uint32 i = animationSetSpec.animationCount;
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

    ttReadBytes(&stream, sizeof(TTMOTFileHeader), (Uint8*) animationSetSpec);
    Uint32 animationCount = animationSetSpec->animationCount;
    #ifdef TT_DEBUG_MODE
        if(__builtin_expect(animationCount > animationBuffer->maxAnimationSpecCount, 0)) {
            ttCloseFileStream(&stream);
            return TT_ANIMATION_OVERFLOW;
        }
    #endif
    Uint32 size = sizeof(TTAnimationSpec0) * animationCount;
    ttReadBytes(&stream, size, (Uint8*) animationSpecs);
    size = 0;
    Uint32 i = animationCount;
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
    ttReadBytes(&stream, size, (Uint8*) animationBuffer->animations);

    ttCloseFileStream(&stream);
    return OK;
}
