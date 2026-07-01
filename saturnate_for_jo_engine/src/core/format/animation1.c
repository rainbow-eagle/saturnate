#include <jo/sgl_prototypes.h>
#include "animation1.h"
#include "../file.h"
#include "../memory.h"
#include "../../debug/print.h"

//It's actually exactly the same as MOT0, Spec1 instead of Spec0, but it doesn't change anything
void ttLoadMOTMetadata1(char* fileName, TTMOTFileMetadata1* metaData) {
    TTFileStream stream;
    ttOpenFileStream(fileName, &stream);
    TTMOTFileHeader animationSetSpec;

    ttReadBytes(&stream, sizeof(TTMOTFileHeader), (Uint8*) &animationSetSpec);
    TTAnimationSpec1 animationSpecs[animationSetSpec.animationCount];
    Uint32 size = sizeof(TTAnimationSpec1) * animationSetSpec.animationCount;
    ttReadBytes(&stream, size, (Uint8*) animationSpecs);
    Uint32 poseCount = 0;
    Uint32 i = animationSetSpec.animationCount;
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

    ttReadBytes(&stream, sizeof(TTMOTFileHeader), (Uint8*) animationSetSpec);
    Uint32 animationCount = animationSetSpec->animationCount;
    Uint32 meshCount = animationSetSpec->meshCount;
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
    Uint32 size = sizeof(TTAnimationSpec1) * animationCount;
    TTAnimationSpec1* animationSpecs = (TTAnimationSpec1*) animationBuffer->animationSpecs;
    ttReadBytes(&stream, size, (Uint8*) animationSpecs);
    size = sizeof(TTBone) * meshCount;
    ttReadBytes(&stream, size, (Uint8*) animationBuffer->armature);
    size = 0;
    Uint32 i = animationCount;
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

    ttReadBytes(&stream, size * sizeof(TTRootBonePosition), (Uint8*) animationBuffer->rootBonePositionByPose);
    size *= animationSetSpec->meshCount * sizeof(TTBoneRotation);
    ttReadBytes(&stream, size, (Uint8*) animationBuffer->animations);

    ttCloseFileStream(&stream);
    return OK;
}
