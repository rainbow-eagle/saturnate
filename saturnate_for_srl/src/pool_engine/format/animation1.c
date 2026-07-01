#include <sl_def.h>
#include "animation1.h"
#include "../../core/format/animation1.h"
#include "../model_loader.h"
#include "../../debug/print.h"

void ttInitAnimationPools1(TTAnimationPools1* pools, uint8_t** nextFreeHRAMBytePtr, uint8_t** nextFreeLRAMBytePtr, uint32_t maxAnimationSetCnt,
                            uint32_t maxMeshCnt, uint32_t maxPoseCnt, uint32_t maxAnimationCnt, TT_RAM_TYPE animationDataRAMType,
                            uint32_t maxTransformCnt, TT_RAM_TYPE transformDataRAMType,
                            uint32_t maxInstanceCnt, TT_RAM_TYPE instanceDataRAMType) {
    uint8_t** transformRAMNextFreeBytePtr = ((transformDataRAMType == TT_HRAM)? nextFreeHRAMBytePtr : nextFreeLRAMBytePtr);
    pools->transformPool = TT_LALLOC(*transformRAMNextFreeBytePtr, maxTransformCnt * sizeof(TTBoneRotation));
    pools->rootBonePositionByPosePool = TT_LALLOC(*transformRAMNextFreeBytePtr, maxPoseCnt * sizeof(TTRootBonePosition));
    pools->bonePool = TT_LALLOC(*transformRAMNextFreeBytePtr, maxMeshCnt * sizeof(TTBone));

    uint8_t** animationRAMNextFreeByte = ((animationDataRAMType == TT_HRAM)? nextFreeHRAMBytePtr : nextFreeLRAMBytePtr);
    pools->animationSpecPool = TT_LALLOC(*animationRAMNextFreeByte, maxAnimationCnt * sizeof(TTAnimationSpec1));
    pools->transformOffsetByAnimationPool = TT_LALLOC(*animationRAMNextFreeByte, maxAnimationCnt * sizeof(uint16_t));
    pools->animationSetSpecPool = TT_LALLOC(*animationRAMNextFreeByte, maxAnimationSetCnt * sizeof(TTAnimationSetSpec1));
    pools->transformOffsetByAnimationSetPool = TT_LALLOC(*animationRAMNextFreeByte, maxAnimationSetCnt * sizeof(uint16_t));
    pools->animationSpecOffsetByModelPool = TT_LALLOC(*animationRAMNextFreeByte, maxAnimationSetCnt * sizeof(uint8_t));

    uint8_t** instanceRAMNextFreeByte= ((instanceDataRAMType == TT_HRAM)? nextFreeHRAMBytePtr : nextFreeLRAMBytePtr);
    pools->animatedInstanceCountByModelPool = TT_LALLOC(*instanceRAMNextFreeByte, maxAnimationSetCnt * sizeof(uint8_t));
    pools->animatedInstanceOffsetByModelPool = TT_LALLOC(*instanceRAMNextFreeByte, maxAnimationSetCnt * sizeof(uint8_t));
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
        uint32_t animationSetSpecCount = animationPools->nextFreeAnimationSetSpec - animationPools->animationSetSpecPool;
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
        uint32_t accumulatedTransformOffset = 0;
        uint32_t meshCount = animationPools->nextFreeAnimationSetSpec->meshCount;
        for(uint32_t i = animationPools->nextFreeAnimationSetSpec->animationCount; i > 0; i--, loadedAnimationSpec++) {
            *animationPools->nextFreeTransformOffsetByAnimation++ = (uint16_t) accumulatedTransformOffset;
            accumulatedTransformOffset +=  meshCount * loadedAnimationSpec->totalPoseCount;
        }
        animationPools->nextFreeAnimationSpec = loadedAnimationSpec;

        TTAnimationSpec1 lastAnimationSpec = *(loadedAnimationSpec - 1);
        uint32_t addedPoseCount = lastAnimationSpec.poseOffset + lastAnimationSpec.totalPoseCount;
        animationPools->nextFreeRootBonePositionByPose += addedPoseCount;
        animationPools->nextFreeTransform += addedPoseCount * meshCount;//=addedTransformCount
        animationPools->nextFreeBone += meshCount;
        animationPools->nextFreeAnimationSetSpec++;
    }

    return returnValue;
}

void ttGetAnimationRAMUsage1(TTAnimationPools1* pools, uint32_t* hRAMUsage, uint32_t* lRAMUsage,
                                uint32_t animationSetCount, uint32_t animationCount, uint32_t meshCount,
                                uint32_t poseCount, uint32_t transformCount, uint32_t instanceCount) {
    uint32_t hRAMAccumulator = 0;
    uint32_t lRAMAccumulator = 0;

    _TT_LOCAL_ACCUMULATE_(animatedInstanceCurrentPosePool, instanceCount, sizeof(TTBoneRotation))
    _TT_LOCAL_ACCUMULATE_(animatedInstanceCurrentRootBonePositionPool, instanceCount, sizeof(TTRootBonePosition))
    _TT_LOCAL_ACCUMULATE_(animatedInstanceCurrentPositionPool, instanceCount, sizeof(TTInstancePosition))
    _TT_LOCAL_ACCUMULATE_(animationStatePool, instanceCount, sizeof(TTAnimationState))


    _TT_LOCAL_ACCUMULATE_(animationSpecOffsetByModelPool, animationSetCount, sizeof(uint8_t))
    _TT_LOCAL_ACCUMULATE_(animatedInstanceCountByModelPool, animationSetCount, sizeof(uint8_t))
    _TT_LOCAL_ACCUMULATE_(animatedInstanceOffsetByModelPool, animationSetCount, sizeof(uint8_t))
    _TT_LOCAL_ACCUMULATE_(transformOffsetByAnimationSetPool, animationSetCount, sizeof(uint16_t))
    _TT_LOCAL_ACCUMULATE_(animationSetSpecPool, animationSetCount, sizeof(TTAnimationSetSpec1))

    _TT_LOCAL_ACCUMULATE_(transformPool, transformCount, sizeof(TTBoneRotation))

    _TT_LOCAL_ACCUMULATE_(rootBonePositionByPosePool, poseCount, sizeof(TTRootBonePosition))

    _TT_LOCAL_ACCUMULATE_(bonePool, meshCount, sizeof(TTBone))

    _TT_LOCAL_ACCUMULATE_(animationSpecPool, animationCount, sizeof(TTAnimationSpec1))
    _TT_LOCAL_ACCUMULATE_(transformOffsetByAnimationPool, animationCount, sizeof(uint16_t))

    *hRAMUsage = hRAMAccumulator;
    *lRAMUsage = lRAMAccumulator;
}

void ttGetCurrentAnimationRAMUsage1(TTModelPools* modelPools, TTAnimationPools1* animationPools, uint32_t* hRAMUsage, uint32_t* lRAMUsage) {
    uint8_t* instanceCountByModelPtr = animationPools->animatedInstanceCountByModelPool;
    uint32_t totalInstanceCount = 0;
    for(uint32_t i = (modelPools->nextFreeModelSpec - modelPools->modelSpecPool); i > 0; i--)
        totalInstanceCount += *(instanceCountByModelPtr++);

    const uint32_t animationSetCount = animationPools->nextFreeAnimationSetSpec - animationPools->animationSetSpecPool;
    const uint32_t animationCount = animationPools->nextFreeAnimationSpec - animationPools->animationSpecPool;
    const uint32_t meshCount = animationPools->nextFreeBone - animationPools->bonePool;
    const uint32_t poseCount = animationPools->nextFreeRootBonePositionByPose - animationPools->rootBonePositionByPosePool;
    const uint32_t transformCount = animationPools->nextFreeTransform - animationPools->transformPool;

    ttGetAnimationRAMUsage1(animationPools, hRAMUsage, lRAMUsage, animationSetCount, animationCount,
                            meshCount, poseCount, transformCount, totalInstanceCount);
}

//returns -1 in case of instance count overflow
int32_t ttSetAnimatedInstanceCountInPool1(TTAnimationPools1* pools, uint8_t animatedModelCount, uint8_t instanceCounts[animatedModelCount]) {
    uint8_t* instanceCountPtr = pools->animatedInstanceCountByModelPool;
    uint8_t* instanceOffsetPtr = pools->animatedInstanceOffsetByModelPool;

    uint32_t accumulatedOffset = 0;
    uint32_t i = (uint32_t) animatedModelCount;
    do {
        uint32_t currentCount = (uint32_t) *instanceCounts++;
        *instanceOffsetPtr++ = (uint8_t) accumulatedOffset;
        *instanceCountPtr++ = (uint8_t) currentCount;
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

void ttStartAnimationInPool1(TTAnimationPools1* pools, uint8_t modelID, uint8_t instanceID, uint8_t animationSpecID) {
    uint32_t specOffset = pools->animationSpecOffsetByModelPool[modelID];
    uint32_t instanceOffset = pools->animatedInstanceOffsetByModelPool[modelID];
    TTAnimationSpec1* animationSpec = pools->animationSpecPool + specOffset + animationSpecID;
    TTAnimationState* animationState = pools->animationStatePool + instanceOffset + instanceID;
    animationState->frameCountBeforeNextPose = animationState->totalFrameCountBetweenPoses = animationSpec->totalFrameCountBetweenPoses;
    animationState->totalPoseCount = animationSpec->totalPoseCount;
    animationState->currentPoseID = animationSpec->totalPoseCount - 1;

    TTAnimationSetSpec1 animationSetSpec = pools->animationSetSpecPool[modelID];
    uint32_t instanceGlobalID = instanceOffset + instanceID;
    uint32_t poseGlobalID = animationSetSpec.poseOffset + animationSpec->poseOffset + animationState->currentPoseID;
    pools->animatedInstanceCurrentRootBonePositionPool[instanceGlobalID] = pools->rootBonePositionByPosePool + poseGlobalID;
    uint32_t transformGlobalID = pools->transformOffsetByAnimationSetPool[modelID] + pools->transformOffsetByAnimationPool[specOffset + animationSpecID]
                            + animationState->currentPoseID * animationSetSpec.meshCount;
    pools->animatedInstanceCurrentPosePool[instanceGlobalID] = pools->transformPool + transformGlobalID;
}

void ttPauseAnimationInPool1(TTAnimationPools1* pools, uint8_t modelID, uint8_t instanceID) {
    uint32_t instanceOffset = pools->animatedInstanceOffsetByModelPool[modelID];
    TTAnimationState* animationState = pools->animationStatePool + instanceOffset + instanceID;
    animationState->frameCountBeforeNextPose = -1;
}


void ttUnpauseAnimationInPool1(TTAnimationPools1* pools, uint8_t modelID, uint8_t instanceID) {
    uint32_t instanceOffset = pools->animatedInstanceOffsetByModelPool[modelID];
    TTAnimationState* animationState = pools->animationStatePool + instanceOffset + instanceID;
    animationState->frameCountBeforeNextPose = animationState->totalFrameCountBetweenPoses;
}

bool ttIsAnimationPausedInPool1(TTAnimationPools1* pools, uint8_t modelID, uint8_t instanceID) {
    uint32_t instanceOffset = pools->animatedInstanceOffsetByModelPool[modelID];
    TTAnimationState* animationState = pools->animationStatePool + instanceOffset + instanceID;
    return animationState->frameCountBeforeNextPose == -1;
}

void ttDeactivateInstanceInPool1(TTAnimationPools1* pools, uint8_t modelID, uint8_t instanceID) {
    uint32_t instanceOffset = pools->animatedInstanceOffsetByModelPool[modelID]; //3 first lines are from ttPauseAnimationInPool
    TTAnimationState* animationState = pools->animationStatePool + instanceOffset + instanceID;
    animationState->frameCountBeforeNextPose = -1;
    pools->animatedInstanceCurrentPosePool[instanceOffset + instanceID] = NULL;
}

bool ttIsInstanceActiveInPool1(TTAnimationPools1* pools, uint8_t modelID, uint8_t instanceID) {//lines from ttIsAnimationPausedInPool
    uint32_t instanceOffset = pools->animatedInstanceOffsetByModelPool[modelID];
    TTAnimationState* animationState = pools->animationStatePool + instanceOffset + instanceID;
    return animationState->frameCountBeforeNextPose == -1 && pools->animatedInstanceCurrentPosePool[instanceOffset + instanceID] == NULL;
}

void ttUpdateAnimationsInPool1(TTAnimationPools1* pools) {
    TTAnimationSetSpec1* animationSetSpec = pools->animationSetSpecPool;
    const uint8_t* restrict instanceCount = pools->animatedInstanceCountByModelPool;
    TTAnimationState* animationStatePtr = pools->animationStatePool;
    TTBoneRotation** currentPosePtr = pools->animatedInstanceCurrentPosePool;
    TTRootBonePosition** currentRootBonePositionPtr = pools->animatedInstanceCurrentRootBonePositionPool;
    for(int i = pools->nextFreeAnimationSetSpec - pools->animationSetSpecPool; i > 0; i--, animationSetSpec++, instanceCount++) {
        const uint32_t meshCount = (uint32_t) animationSetSpec->meshCount;
        for(int j = *instanceCount; j > 0; j--, animationStatePtr++, currentPosePtr++, currentRootBonePositionPtr++) {
            if(__builtin_expect(animationStatePtr->frameCountBeforeNextPose != -1 && --(animationStatePtr->frameCountBeforeNextPose) < 0, 1)) {
                animationStatePtr->frameCountBeforeNextPose = animationStatePtr->totalFrameCountBetweenPoses;
                if(__builtin_expect(--(animationStatePtr->currentPoseID) < 0, 0)) {
                    uint32_t lastPoseID = animationStatePtr->totalPoseCount - 1;
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
    const uint8_t* restrict instanceCount = animationPools->animatedInstanceCountByModelPool;
    TTInstancePosition* restrict position = animationPools->animatedInstanceCurrentPositionPool;
    TTBoneRotation** restrict currentPose = animationPools->animatedInstanceCurrentPosePool;
    TTRootBonePosition** restrict currentRootBonePosition = animationPools->animatedInstanceCurrentRootBonePositionPool;
    // je suppose que le tableau de models et d'animationSet corespondent (même taille, même entity au même indice), forcer ça by design
    for(int i = modelPools->nextFreeModelSpec - modelPools->modelSpecPool; i > 0; i--, modelSpec++, currentArmature += animationSetSpec->meshCount, animationSetSpec++, instanceCount++) {
        PDATA* restrict localModel = modelPools->meshPool + modelSpec->meshID;
        const uint32_t localMeshCount = (uint32_t) animationSetSpec->meshCount;
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
