#include <jo/sgl_prototypes.h>
#include "animation1.h"
#include "../../core/format/animation1.h"
#include "../model_loader.h"
#include "../../debug/print.h"

void ttInitAnimationPools1(TTAnimationPools1* pools, Uint8** nextFreeHRAMBytePtr, Uint8** nextFreeLRAMBytePtr, Uint32 maxAnimationSetCnt,
                            Uint32 maxMeshCnt, Uint32 maxPoseCnt, Uint32 maxAnimationCnt, TT_RAM_TYPE animationDataRAMType,
                            Uint32 maxTransformCnt, TT_RAM_TYPE transformDataRAMType,
                            Uint32 maxInstanceCnt, TT_RAM_TYPE instanceDataRAMType) {
    Uint8** transformRAMNextFreeBytePtr = ((transformDataRAMType == TT_HRAM)? nextFreeHRAMBytePtr : nextFreeLRAMBytePtr);
    pools->transformPool = TT_LALLOC(*transformRAMNextFreeBytePtr, maxTransformCnt * sizeof(TTBoneRotation));
    pools->rootBonePositionByPosePool = TT_LALLOC(*transformRAMNextFreeBytePtr, maxPoseCnt * sizeof(TTRootBonePosition));
    pools->bonePool = TT_LALLOC(*transformRAMNextFreeBytePtr, maxMeshCnt * sizeof(TTBone));

    Uint8** animationRAMNextFreeByte = ((animationDataRAMType == TT_HRAM)? nextFreeHRAMBytePtr : nextFreeLRAMBytePtr);
    pools->animationSpecPool = TT_LALLOC(*animationRAMNextFreeByte, maxAnimationCnt * sizeof(TTAnimationSpec1));
    pools->transformOffsetByAnimationPool = TT_LALLOC(*animationRAMNextFreeByte, maxAnimationCnt * sizeof(Uint16));
    pools->animationSetSpecPool = TT_LALLOC(*animationRAMNextFreeByte, maxAnimationSetCnt * sizeof(TTAnimationSetSpec1));
    pools->transformOffsetByAnimationSetPool = TT_LALLOC(*animationRAMNextFreeByte, maxAnimationSetCnt * sizeof(Uint16));
    pools->animationSpecOffsetByModelPool = TT_LALLOC(*animationRAMNextFreeByte, maxAnimationSetCnt * sizeof(Uint8));

    Uint8** instanceRAMNextFreeByte= ((instanceDataRAMType == TT_HRAM)? nextFreeHRAMBytePtr : nextFreeLRAMBytePtr);
    pools->animatedInstanceCountByModelPool = TT_LALLOC(*instanceRAMNextFreeByte, maxAnimationSetCnt * sizeof(Uint8));
    pools->animatedInstanceOffsetByModelPool = TT_LALLOC(*instanceRAMNextFreeByte, maxAnimationSetCnt * sizeof(Uint8));
    pools->animationStatePool = TT_LALLOC(*instanceRAMNextFreeByte, maxInstanceCnt * sizeof(TTAnimationState));
    pools->animatedInstanceCurrentPositionPool = TT_LALLOC(*instanceRAMNextFreeByte, maxInstanceCnt * sizeof(TTInstancePosition));
    pools->animatedInstanceCurrentPosePool = TT_LALLOC(*instanceRAMNextFreeByte, maxInstanceCnt * sizeof(TTBoneRotation*));
    pools->animatedInstanceCurrentRootBonePositionPool = TT_LALLOC(*instanceRAMNextFreeByte, maxInstanceCnt * sizeof(TTRootBonePosition*));

    pools->nextFreeTransform                   = pools->transformPool;
    pools->nextFreeTransformOffsetByAnimation  = pools->transformOffsetByAnimationPool;
    pools->nextFreeTransformOffsetByAnimationSet = pools->transformOffsetByAnimationSetPool;
    pools->nextFreeRootBonePositionByPose      = pools->rootBonePositionByPosePool;
    pools->nextFreeBone                        = pools->bonePool;
    pools->nextFreeAnimationSpec               = pools->animationSpecPool;
    pools->nextFreeAnimationSetSpec            = pools->animationSetSpecPool;
    pools->nextFreeAnimationSpecOffsetByModel  = pools->animationSpecOffsetByModelPool;

    pools->maxTransformCount      = maxTransformCnt;
    pools->maxAnimationSetCount   = maxAnimationSetCnt;
    pools->maxAnimationCount      = maxAnimationCnt;
    pools->maxPoseCount           = maxPoseCnt;
    pools->maxAnimatedInstanceCount = maxInstanceCnt;
    pools->maxMeshCount           = maxMeshCnt;
}

void ttResetAnimationPools1(TTAnimationPools1* pools) {
    pools->nextFreeTransform = pools->transformPool;
    pools->nextFreeRootBonePositionByPose = pools->rootBonePositionByPosePool;
    pools->nextFreeBone = pools->bonePool;
    pools->nextFreeAnimationSpec = pools->animationSpecPool;
    pools->nextFreeAnimationSetSpec = pools->animationSetSpecPool;
    pools->nextFreeTransformOffsetByAnimation = pools->transformOffsetByAnimationPool;
    pools->nextFreeTransformOffsetByAnimationSet = pools->transformOffsetByAnimationSetPool;
    pools->nextFreeAnimationSpecOffsetByModel = pools->animationSpecOffsetByModelPool;
}

// requires "ttInitFile"
// animationTransforms is a 3 dimensional array : animationTransforms[animationCount][PoseCount (variable)][meshCount (not variable)]
 TTAnimationPoolLoadStatus ttLoadAnimationInPool1(TTModelPools* modelPools, TTAnimationPools1* animationPools, char* filename) {
    #ifdef TT_DEBUG_MODE
        Uint32 animationSetSpecCount = animationPools->nextFreeAnimationSetSpec - animationPools->animationSetSpecPool;
        if(__builtin_expect(animationSetSpecCount >= animationPools->maxAnimationSetCount, 0))
            return TT_ANIMATION_SET_POOL_OVERFLOW;
    #endif
    TTAnimationBuffer1 animationBuffer = {
        (TTMOTFileHeader*) animationPools->nextFreeAnimationSetSpec,
        animationPools->nextFreeAnimationSpec,
        animationPools->nextFreeBone,
        animationPools->nextFreeRootBonePositionByPose,
        animationPools->nextFreeTransform,
        animationPools->maxAnimationCount - (animationPools->nextFreeAnimationSpec - animationPools->animationSpecPool),
        modelPools->maxMeshCount - (animationPools->nextFreeBone - animationPools->bonePool),
        animationPools->maxPoseCount - (animationPools->nextFreeRootBonePositionByPose - animationPools->rootBonePositionByPosePool),
        animationPools->maxTransformCount - (animationPools->nextFreeTransform - animationPools->transformPool)
    };
    TTAnimationLoadStatus returnValue = ttLoadMOTAnimation1(filename, &animationBuffer);//it only works because I manually mapped the error code from core to pool
    if(__builtin_expect(!returnValue, 1)) {
        *animationPools->nextFreeAnimationSpecOffsetByModel = animationPools->nextFreeAnimationSpec - animationPools->animationSpecPool;
        animationPools->nextFreeAnimationSpecOffsetByModel++;
        *animationPools->nextFreeTransformOffsetByAnimationSet = animationPools->nextFreeTransform - animationPools->transformPool;
        animationPools->nextFreeTransformOffsetByAnimationSet++;
        animationPools->nextFreeAnimationSetSpec->poseOffset = animationPools->nextFreeRootBonePositionByPose - animationPools->rootBonePositionByPosePool;//=addedPoseCount

        TTAnimationSpec1* loadedAnimationSpec = animationPools->nextFreeAnimationSpec;
        Uint32 accumulatedTransformOffset = 0;
        Uint32 meshCount = animationPools->nextFreeAnimationSetSpec->meshCount;
        for(Uint32 i = animationPools->nextFreeAnimationSetSpec->animationCount; i > 0; i--, loadedAnimationSpec++) {
            *animationPools->nextFreeTransformOffsetByAnimation++ = (Uint16) accumulatedTransformOffset;
            accumulatedTransformOffset +=  meshCount * loadedAnimationSpec->totalPoseCount;
        }
        animationPools->nextFreeAnimationSpec = loadedAnimationSpec;

        TTAnimationSpec1 lastAnimationSpec = *(loadedAnimationSpec - 1);
        Uint32 addedPoseCount = lastAnimationSpec.poseOffset + lastAnimationSpec.totalPoseCount;
        animationPools->nextFreeRootBonePositionByPose += addedPoseCount;
        animationPools->nextFreeTransform += addedPoseCount * meshCount;//=addedTransformCount
        animationPools->nextFreeBone += meshCount;
        animationPools->nextFreeAnimationSetSpec++;
    }

    return returnValue;
}

void ttGetAnimationRAMUsage1(TTAnimationPools1* pools, Uint32* hRAMUsage, Uint32* lRAMUsage,
                                Uint32 animationSetCount, Uint32 animationCount, Uint32 meshCount,
                                Uint32 poseCount, Uint32 transformCount, Uint32 instanceCount) {
    Uint32 hRAMAccumulator = 0;
    Uint32 lRAMAccumulator = 0;

    _TT_LOCAL_ACCUMULATE_(animatedInstanceCurrentPosePool, instanceCount, sizeof(TTBoneRotation))
    _TT_LOCAL_ACCUMULATE_(animatedInstanceCurrentRootBonePositionPool, instanceCount, sizeof(TTRootBonePosition))
    _TT_LOCAL_ACCUMULATE_(animatedInstanceCurrentPositionPool, instanceCount, sizeof(TTInstancePosition))
    _TT_LOCAL_ACCUMULATE_(animationStatePool, instanceCount, sizeof(TTAnimationState))


    _TT_LOCAL_ACCUMULATE_(animationSpecOffsetByModelPool, animationSetCount, sizeof(Uint8))
    _TT_LOCAL_ACCUMULATE_(animatedInstanceCountByModelPool, animationSetCount, sizeof(Uint8))
    _TT_LOCAL_ACCUMULATE_(animatedInstanceOffsetByModelPool, animationSetCount, sizeof(Uint8))
    _TT_LOCAL_ACCUMULATE_(transformOffsetByAnimationSetPool, animationSetCount, sizeof(Uint16))
    _TT_LOCAL_ACCUMULATE_(animationSetSpecPool, animationSetCount, sizeof(TTAnimationSetSpec1))

    _TT_LOCAL_ACCUMULATE_(transformPool, transformCount, sizeof(TTBoneRotation))

    _TT_LOCAL_ACCUMULATE_(rootBonePositionByPosePool, poseCount, sizeof(TTRootBonePosition))

    _TT_LOCAL_ACCUMULATE_(bonePool, meshCount, sizeof(TTBone))

    _TT_LOCAL_ACCUMULATE_(animationSpecPool, animationCount, sizeof(TTAnimationSpec1))
    _TT_LOCAL_ACCUMULATE_(transformOffsetByAnimationPool, animationCount, sizeof(Uint16))

    *hRAMUsage = hRAMAccumulator;
    *lRAMUsage = lRAMAccumulator;
}

void ttGetCurrentAnimationRAMUsage1(TTModelPools* modelPools, TTAnimationPools1* animationPools, Uint32* hRAMUsage, Uint32* lRAMUsage) {
    Uint8* instanceCountByModelPtr = animationPools->animatedInstanceCountByModelPool;
    Uint32 totalInstanceCount = 0;
    for(Uint32 i = (modelPools->nextFreeModelSpec - modelPools->modelSpecPool); i > 0; i--)
        totalInstanceCount += *(instanceCountByModelPtr++);

    const Uint32 animationSetCount = animationPools->nextFreeAnimationSetSpec - animationPools->animationSetSpecPool;
    const Uint32 animationCount = animationPools->nextFreeAnimationSpec - animationPools->animationSpecPool;
    const Uint32 meshCount = animationPools->nextFreeBone - animationPools->bonePool;
    const Uint32 poseCount = animationPools->nextFreeRootBonePositionByPose - animationPools->rootBonePositionByPosePool;
    const Uint32 transformCount = animationPools->nextFreeTransform - animationPools->transformPool;

    ttGetAnimationRAMUsage1(animationPools, hRAMUsage, lRAMUsage, animationSetCount, animationCount,
                            meshCount, poseCount, transformCount, totalInstanceCount);
}

//returns -1 in case of instance count overflow
Sint32 ttSetAnimatedInstanceCountInPool1(TTAnimationPools1* pools, Uint8 animatedModelCount, Uint8 instanceCounts[animatedModelCount]) {
    Uint8* instanceCountPtr = pools->animatedInstanceCountByModelPool;
    Uint8* instanceOffsetPtr = pools->animatedInstanceOffsetByModelPool;

    Uint32 accumulatedOffset = 0;
    Uint32 i = (Uint32) animatedModelCount;
    do {
        Uint32 currentCount = (Uint32) *instanceCounts++;
        *instanceOffsetPtr++ = (Uint8) accumulatedOffset;
        *instanceCountPtr++ = (Uint8) currentCount;
        accumulatedOffset += currentCount;
    }
    while(--i > 0);

    #ifdef TT_DEBUG_MODE
        if(accumulatedOffset >= pools->maxAnimatedInstanceCount)
            return -1; //ANIMATED INSTANCE OVERFLOW
    #endif

    TTAnimationState* animationStatePtr = pools->animationStatePool;
    TTBoneRotation** currentPosePtr = pools->animatedInstanceCurrentPosePool;
    for(i = pools->maxAnimatedInstanceCount; i > 0; i--, animationStatePtr++) {
        animationStatePtr->frameCountBeforeNextPose = -1; // -1 means inactive, do not treat
        *currentPosePtr++ = NULL;
    }
}

void ttStartAnimationInPool1(TTAnimationPools1* pools, Uint8 modelID, Uint8 instanceID, Uint8 animationSpecID) {
    Uint32 specOffset = pools->animationSpecOffsetByModelPool[modelID];
    Uint32 instanceOffset = pools->animatedInstanceOffsetByModelPool[modelID];
    TTAnimationSpec1* animationSpec = pools->animationSpecPool + specOffset + animationSpecID;
    TTAnimationState* animationState = pools->animationStatePool + instanceOffset + instanceID;
    animationState->frameCountBeforeNextPose = animationState->totalFrameCountBetweenPoses = animationSpec->totalFrameCountBetweenPoses;
    animationState->totalPoseCount = animationSpec->totalPoseCount;
    animationState->currentPoseID = animationSpec->totalPoseCount - 1;

    TTAnimationSetSpec1 animationSetSpec = pools->animationSetSpecPool[modelID];
    Uint32 instanceGlobalID = instanceOffset + instanceID;
    Uint32 poseGlobalID = animationSetSpec.poseOffset + animationSpec->poseOffset + animationState->currentPoseID;
    pools->animatedInstanceCurrentRootBonePositionPool[instanceGlobalID] = pools->rootBonePositionByPosePool + poseGlobalID;
    Uint32 transformGlobalID = pools->transformOffsetByAnimationSetPool[modelID] + pools->transformOffsetByAnimationPool[specOffset + animationSpecID]
                            + animationState->currentPoseID * animationSetSpec.meshCount;
    pools->animatedInstanceCurrentPosePool[instanceGlobalID] = pools->transformPool + transformGlobalID;
}

void ttPauseAnimationInPool1(TTAnimationPools1* pools, Uint8 modelID, Uint8 instanceID) {
    Uint32 instanceOffset = pools->animatedInstanceOffsetByModelPool[modelID];
    TTAnimationState* animationState = pools->animationStatePool + instanceOffset + instanceID;
    animationState->frameCountBeforeNextPose = -1;
}


void ttUnpauseAnimationInPool1(TTAnimationPools1* pools, Uint8 modelID, Uint8 instanceID) {
    Uint32 instanceOffset = pools->animatedInstanceOffsetByModelPool[modelID];
    TTAnimationState* animationState = pools->animationStatePool + instanceOffset + instanceID;
    animationState->frameCountBeforeNextPose = animationState->totalFrameCountBetweenPoses;
}

Bool ttIsAnimationPausedInPool1(TTAnimationPools1* pools, Uint8 modelID, Uint8 instanceID) {
    Uint32 instanceOffset = pools->animatedInstanceOffsetByModelPool[modelID];
    TTAnimationState* animationState = pools->animationStatePool + instanceOffset + instanceID;
    return animationState->frameCountBeforeNextPose == -1;
}

void ttDeactivateInstanceInPool1(TTAnimationPools1* pools, Uint8 modelID, Uint8 instanceID) {
    Uint32 instanceOffset = pools->animatedInstanceOffsetByModelPool[modelID]; //3 first lines are from ttPauseAnimationInPool
    TTAnimationState* animationState = pools->animationStatePool + instanceOffset + instanceID;
    animationState->frameCountBeforeNextPose = -1;
    pools->animatedInstanceCurrentPosePool[instanceOffset + instanceID] = NULL;
}

Bool ttIsInstanceActiveInPool1(TTAnimationPools1* pools, Uint8 modelID, Uint8 instanceID) {//lines from ttIsAnimationPausedInPool
    Uint32 instanceOffset = pools->animatedInstanceOffsetByModelPool[modelID];
    TTAnimationState* animationState = pools->animationStatePool + instanceOffset + instanceID;
    return animationState->frameCountBeforeNextPose == -1 && pools->animatedInstanceCurrentPosePool[instanceOffset + instanceID] == NULL;
}

void ttUpdateAnimationsInPool1(TTAnimationPools1* pools) {
    TTAnimationSetSpec1* animationSetSpec = pools->animationSetSpecPool;
    const Uint8* restrict instanceCount = pools->animatedInstanceCountByModelPool;
    TTAnimationState* animationStatePtr = pools->animationStatePool;
    TTBoneRotation** currentPosePtr = pools->animatedInstanceCurrentPosePool;
    TTRootBonePosition** currentRootBonePositionPtr = pools->animatedInstanceCurrentRootBonePositionPool;
    for(int i = pools->nextFreeAnimationSetSpec - pools->animationSetSpecPool; i > 0; i--, animationSetSpec++, instanceCount++) {
        const Uint32 meshCount = (Uint32) animationSetSpec->meshCount;
        for(int j = *instanceCount; j > 0; j--, animationStatePtr++, currentPosePtr++, currentRootBonePositionPtr++) {
            if(__builtin_expect(animationStatePtr->frameCountBeforeNextPose != -1 && --(animationStatePtr->frameCountBeforeNextPose) < 0, 1)) {
                animationStatePtr->frameCountBeforeNextPose = animationStatePtr->totalFrameCountBetweenPoses;
                if(__builtin_expect(--(animationStatePtr->currentPoseID) < 0, 0)) {
                    Uint32 lastPoseID = animationStatePtr->totalPoseCount - 1;
                    animationStatePtr->currentPoseID = lastPoseID;
                    (*currentPosePtr) += lastPoseID * meshCount;
                    (*currentRootBonePositionPtr) += lastPoseID;
                } else {
                    (*currentPosePtr) -= meshCount;
                    (*currentRootBonePositionPtr) -= 1;
                }
            }
        }
    }
}

//Will eventually also use a modelPool
void ttDrawAnimatedModelsFromPool1(TTModelPools* modelPools, TTAnimationPools1* animationPools) {
    const TTModelSpec* restrict modelSpec = modelPools->modelSpecPool;
    const TTAnimationSetSpec1* restrict animationSetSpec = animationPools->animationSetSpecPool;
    TTBone* restrict currentArmature = animationPools->bonePool;
    const Uint8* restrict instanceCount = animationPools->animatedInstanceCountByModelPool;
    TTInstancePosition* restrict position = animationPools->animatedInstanceCurrentPositionPool;
    TTBoneRotation** restrict currentPose = animationPools->animatedInstanceCurrentPosePool;
    TTRootBonePosition** restrict currentRootBonePosition = animationPools->animatedInstanceCurrentRootBonePositionPool;
    // je suppose que le tableau de models et d'animationSet corespondent (même taille, même entity au même indice), forcer ça by design
    for(int i = modelPools->nextFreeModelSpec - modelPools->modelSpecPool; i > 0; i--, modelSpec++, currentArmature += animationSetSpec->meshCount, animationSetSpec++, instanceCount++) {
        PDATA* restrict localModel = modelPools->meshPool + modelSpec->meshID;
        const Uint32 localMeshCount = (Uint32) animationSetSpec->meshCount;
        for(int j = *instanceCount; j > 0; j--, position++, currentPose++, currentRootBonePosition++) {
            if(__builtin_expect(*currentPose != NULL, 1)) {
                slPushMatrix();
                slTranslate(position->xOffset, position->yOffset, position->zOffset);
                slRotX(position->xAngle);
                slRotY(position->yAngle);
                slRotZ(position->zAngle);
                ttDrawAnimatedModelAtPose1(localModel, currentArmature, *currentPose, *currentRootBonePosition, localMeshCount);
                slPopMatrix();
            }
        }
    }
}
