#include <sl_def.h>
#include "animation1.h"
#include "../file.h"
#include "../memory.h"
#include "../../debug/print.h"

//It's actually exactly the same as MOT0, Spec1 instead of Spec0, but it doesn't change anything
void ttLoadMOTMetadata1(char* fileName, TTMOTFileMetadata1* metaData) {
    TTFileStream stream;
    ttOpenFileStream(fileName, &stream);
    TTMOTFileHeader animationSetSpec;

    ttReadBytes(&stream, sizeof(TTMOTFileHeader), (uint8_t*) &animationSetSpec);
    TTAnimationSpec1 animationSpecs[animationSetSpec.animationCount];
    uint32_t size = sizeof(TTAnimationSpec1) * animationSetSpec.animationCount;
    ttReadBytes(&stream, size, (uint8_t*) animationSpecs);
    uint32_t poseCount = 0;
    uint32_t i = animationSetSpec.animationCount;
    TTAnimationSpec1* animationSpecPtr = animationSpecs;
    do
        poseCount += animationSpecPtr++->totalPoseCount;
    while(--i);
    metaData->totalPoseCount = poseCount;
    metaData->animationCount = animationSetSpec.animationCount;
    metaData->meshCount = animationSetSpec.meshCount;

    ttCloseFileStream(&stream);
}

TTAnimationLoadStatus ttLoadMOTAnimation1(char* fileName, TTAnimationBuffer1* animationBuffer) {
    TTFileStream stream;
    ttOpenFileStream(fileName, &stream);
    TTMOTFileHeader* animationSetSpec = (TTMOTFileHeader*) animationBuffer->animationSetSpecPtr;

    ttReadBytes(&stream, sizeof(TTMOTFileHeader), (uint8_t*) animationSetSpec);
    uint32_t animationCount = animationSetSpec->animationCount;
    uint32_t meshCount = animationSetSpec->meshCount;
    #ifdef TT_DEBUG_MODE
        if(__builtin_expect(animationCount > animationBuffer->maxAnimationSpecCount, 0)) {
            ttCloseFileStream(&stream);
            return TT_ANIMATION_OVERFLOW;
        }
        if(__builtin_expect(meshCount > animationBuffer->maxBoneCount, 0)) {
            ttCloseFileStream(&stream);
            return TT_BONE_OVERFLOW;
        }
    #endif
    uint32_t size = sizeof(TTAnimationSpec1) * animationCount;
    TTAnimationSpec1* animationSpecs = (TTAnimationSpec1*) animationBuffer->animationSpecs;
    ttReadBytes(&stream, size, (uint8_t*) animationSpecs);
    size = sizeof(TTBone) * meshCount;
    ttReadBytes(&stream, size, (uint8_t*) animationBuffer->armature);
    size = 0;
    uint32_t i = animationCount;
    do
        size += animationSpecs++->totalPoseCount;
    while(--i);
    #ifdef TT_DEBUG_MODE
        if(__builtin_expect(size > animationBuffer->maxRootBonePositionByPoseCount, 0)) {
            ttCloseFileStream(&stream);
            return TT_POSE_OVERFLOW;
        }
        if(__builtin_expect(size * animationSetSpec->meshCount > animationBuffer->maxTransformCount, 0)) {
            ttCloseFileStream(&stream);
            return TT_TRANSFORM_OVERFLOW;
        }
    #endif

    ttReadBytes(&stream, size * sizeof(TTRootBonePosition), (uint8_t*) animationBuffer->rootBonePositionByPose);
    size *= animationSetSpec->meshCount * sizeof(TTBoneRotation);
    ttReadBytes(&stream, size, (uint8_t*) animationBuffer->animations);

    ttCloseFileStream(&stream);
    return OK;
}
