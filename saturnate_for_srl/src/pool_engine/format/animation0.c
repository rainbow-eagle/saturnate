#include <sl_def.h>
#include "animation0.h"
#include "../../core/memory.h"
#include "../../core/format/animation0.h"
#include "../model_loader.h"
#include "../../debug/print.h"

void ttInitAnimationPoolsX0(TTAnimationPools0* pools, uint8_t** nextFreeHRAMBytePtr, uint8_t** nextFreeLRAMBytePtr, uint32_t maxAnimationSetCount,
                            uint32_t maxMeshCount, uint32_t maxAnimationCount, TT_RAM_TYPE animationDataRAMType,
                            uint32_t maxTransformCount, TT_RAM_TYPE transformDataRAMType,
                            uint32_t maxInstanceCount, TT_RAM_TYPE instanceDataRAMType) {
    uint8_t** transformRAMNextFreeBytePtr = ((transformDataRAMType == TT_HRAM)? nextFreeHRAMBytePtr : nextFreeLRAMBytePtr);
    pools->transformPool = TT_LALLOC(*transformRAMNextFreeBytePtr, maxTransformCount * sizeof(TTTransform));
    uint8_t** animationRAMNextFreeByte = ((animationDataRAMType == TT_HRAM)? nextFreeHRAMBytePtr : nextFreeLRAMBytePtr);
    pools->animationSpecPool = TT_LALLOC(*animationRAMNextFreeByte, maxAnimationCount * sizeof(TTAnimationSpec1));
    pools->animationSetSpecPool = TT_LALLOC(*animationRAMNextFreeByte, maxAnimationSetCount * sizeof(TTAnimationSetSpec0));
    pools->animationSpecOffsetByModelPool = TT_LALLOC(*animationRAMNextFreeByte, maxAnimationSetCount * sizeof(uint8_t));
    uint8_t** instanceRAMNextFreeByte= ((instanceDataRAMType == TT_HRAM)? nextFreeHRAMBytePtr : nextFreeLRAMBytePtr);
    pools->animatedInstanceCountByModelPool = TT_LALLOC(*instanceRAMNextFreeByte, maxAnimationSetCount * sizeof(uint8_t));
    pools->animatedInstanceOffsetByModelPool = TT_LALLOC(*instanceRAMNextFreeByte, maxAnimationSetCount * sizeof(uint8_t));
    pools->animationStatePool = TT_LALLOC(*instanceRAMNextFreeByte, maxInstanceCount * sizeof(TTAnimationState));
    pools->animatedInstanceCurrentPositionPool = TT_LALLOC(*instanceRAMNextFreeByte, maxInstanceCount * sizeof(TTInstancePosition));
    pools->animatedInstanceCurrentPosePool = TT_LALLOC(*instanceRAMNextFreeByte, maxInstanceCount * sizeof(TTBoneRotation*));

    pools->nextFreeTransform                   = pools->transformPool;
    pools->nextFreeAnimationSpec               = pools->animationSpecPool;
    pools->nextFreeAnimationSetSpec            = pools->animationSetSpecPool;
    pools->nextFreeAnimationSpecOffsetByModel  = pools->animationSpecOffsetByModelPool;

    pools->maxTransformCount      = maxTransformCount;
    pools->maxAnimationSetCount   = maxAnimationSetCount;
    pools->maxAnimationCount      = maxAnimationCount;
    pools->maxAnimatedInstanceCount = maxInstanceCount;
}
void ttInitAnimationPools0(TTAnimationPools0* pools, uint8_t** nextFreeHRAMBytePtr, uint8_t** nextFreeLRAMBytePtr, uint32_t maxAnimationSetCount,
                            uint32_t maxMeshCount, uint32_t maxPoseCnt, uint32_t maxAnimationCount, TT_RAM_TYPE animationDataRAMType,
                            uint32_t maxTransformCount, TT_RAM_TYPE transformDataRAMType,
                            uint32_t maxInstanceCount, TT_RAM_TYPE instanceDataRAMType) {
    ttInitAnimationPoolsX0(pools, nextFreeHRAMBytePtr, nextFreeLRAMBytePtr, maxAnimationSetCount, maxMeshCount,
                            maxPoseCnt, animationDataRAMType, maxTransformCount, transformDataRAMType,
                            maxInstanceCount, instanceDataRAMType);
}

void ttResetAnimationPools0(TTAnimationPools0* pools) {
    pools->nextFreeTransform = pools->transformPool;
    pools->nextFreeAnimationSpec = pools->animationSpecPool;
    pools->nextFreeAnimationSetSpec = pools->animationSetSpecPool;
    pools->nextFreeAnimationSpecOffsetByModel = pools->animationSpecOffsetByModelPool;
}

// requires "ttInitFile"
// animationTransforms is a 3 dimensional array : animationTransforms[animationCount][PoseCount (variable)][meshCount (not variable)]
 TTAnimationPoolLoadStatus ttLoadAnimationInPool0(TTModelPools* modelPools, TTAnimationPools0* animationPools, char* filename) {
    #ifdef TT_DEBUG_MODE
        uint32_t animationSetSpecCount = animationPools->nextFreeAnimationSetSpec - animationPools->animationSetSpecPool;
        if(__builtin_expect(animationSetSpecCount >= animationPools->maxAnimationSetCount, 0))
            return TT_ANIMATION_SET_POOL_OVERFLOW;
    #endif
    TTAnimationBuffer0 animationBuffer = {
        (TTMOTFileHeader*) animationPools->nextFreeAnimationSetSpec,
        animationPools->nextFreeAnimationSpec, animationPools->maxAnimationCount - (animationPools->nextFreeAnimationSpec - animationPools->animationSpecPool),
        animationPools->nextFreeTransform, animationPools->maxTransformCount - (animationPools->nextFreeTransform - animationPools->transformPool)
    };
    TTAnimationLoadStatus returnValue = ttLoadMOTAnimation0(filename, &animationBuffer);//it only works because I manually mapped the error code from core to pool
    if(__builtin_expect(!returnValue, 1)) {
        *animationPools->nextFreeAnimationSpecOffsetByModel = animationPools->nextFreeAnimationSpec - animationPools->animationSpecPool;
        animationPools->nextFreeAnimationSpecOffsetByModel++;
        animationPools->nextFreeAnimationSetSpec->transformOffset = animationPools->nextFreeTransform - animationPools->transformPool;
        TTAnimationSpec0 lastAnimationSpec = *(animationPools->nextFreeAnimationSpec + animationPools->nextFreeAnimationSetSpec->animationCount - 1);
        animationPools->nextFreeTransform += lastAnimationSpec.totalPoseCount * animationPools->nextFreeAnimationSetSpec->meshCount + lastAnimationSpec.transformOffset;
        animationPools->nextFreeAnimationSpec += animationPools->nextFreeAnimationSetSpec->animationCount;
        animationPools->nextFreeAnimationSetSpec++;
    }

    return returnValue;
}

void ttGetAnimationRAMUsage0(TTAnimationPools0* pools, uint32_t* hRAMUsage, uint32_t* lRAMUsage,
                                    uint32_t animationSetCount, uint32_t animationCount, uint32_t transformCount, uint32_t instanceCount) {
    uint32_t hRAMAccumulator = 0;
    uint32_t lRAMAccumulator = 0;

    _TT_LOCAL_ACCUMULATE_(animatedInstanceCurrentPosePool, instanceCount, sizeof(TTTransform))
    _TT_LOCAL_ACCUMULATE_(animatedInstanceCurrentPositionPool, instanceCount, sizeof(TTInstancePosition))
    _TT_LOCAL_ACCUMULATE_(animationStatePool, instanceCount, sizeof(TTAnimationState))

    _TT_LOCAL_ACCUMULATE_(animationSpecOffsetByModelPool, animationSetCount, sizeof(uint8_t))
    _TT_LOCAL_ACCUMULATE_(animatedInstanceCountByModelPool, animationSetCount, sizeof(uint8_t))
    _TT_LOCAL_ACCUMULATE_(animatedInstanceOffsetByModelPool, animationSetCount, sizeof(uint8_t))
    _TT_LOCAL_ACCUMULATE_(animationSetSpecPool, animationSetCount, sizeof(TTAnimationSetSpec0))

    _TT_LOCAL_ACCUMULATE_(transformPool, transformCount, sizeof(TTTransform))

    _TT_LOCAL_ACCUMULATE_(animationSpecPool, animationCount, sizeof(TTAnimationSpec0))

    *hRAMUsage = hRAMAccumulator;
    *lRAMUsage = lRAMAccumulator;
}

void ttGetCurrentAnimationRAMUsage0(TTModelPools* modelPools, TTAnimationPools0* animationPools, uint32_t* hRAMUsage, uint32_t* lRAMUsage) {
    uint8_t* instanceCountByModelPtr = animationPools->animatedInstanceCountByModelPool;
    uint32_t totalInstanceCount = 0;
    for(uint32_t i = (modelPools->nextFreeModelSpec - modelPools->modelSpecPool); i > 0; i--)
        totalInstanceCount += *(instanceCountByModelPtr++);

    const uint32_t animationSetCount = animationPools->nextFreeAnimationSetSpec - animationPools->animationSetSpecPool;
    const uint32_t transformCount = animationPools->nextFreeTransform - animationPools->transformPool;
    const uint32_t animationCount = animationPools->nextFreeAnimationSpec - animationPools->animationSpecPool;

    ttGetAnimationRAMUsage0(animationPools, hRAMUsage, lRAMUsage, animationSetCount, animationCount, transformCount, totalInstanceCount);
}

//L'idée est de décider du nombre max d'instances d'un model animé avant le lancement d'un nivau/section or whatever
//returns -1 in case of instance count overflow
int32_t ttSetAnimatedInstanceCountInPool0(TTAnimationPools0* pools, uint8_t animatedModelCount, uint8_t instanceCounts[animatedModelCount]) {
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
    TTTransform** currentPosePtr = pools->animatedInstanceCurrentPosePool;
    for(i = pools->maxAnimatedInstanceCount; i > 0; i--, animationStatePtr++) {
        animationStatePtr->frameCountBeforeNextPose = -1; // -1 means inactive, do not treat
        *currentPosePtr++ = NULL;
    }
}

void ttStartAnimationInPool0(TTAnimationPools0* pools, uint8_t modelID, uint8_t instanceID, uint8_t animationSpecID) {
    uint32_t specOffset = pools->animationSpecOffsetByModelPool[modelID];
    uint32_t instanceOffset = pools->animatedInstanceOffsetByModelPool[modelID];
    TTAnimationSpec0* animationSpec = pools->animationSpecPool + specOffset + animationSpecID;
    TTAnimationState* animationState = pools->animationStatePool + instanceOffset + instanceID;
    animationState->frameCountBeforeNextPose = animationState->totalFrameCountBetweenPoses = animationSpec->totalFrameCountBetweenPoses;
    animationState->totalPoseCount = animationSpec->totalPoseCount;
    animationState->currentPoseID = animationSpec->totalPoseCount - 1;

    TTAnimationSetSpec0 animationSetSpec = pools->animationSetSpecPool[modelID];
    uint32_t instanceGlobalID = instanceOffset + instanceID;
    uint32_t transformGlobalID = animationSetSpec.transformOffset +  animationSpec->transformOffset
                              + animationState->currentPoseID * animationSetSpec.meshCount;
    pools->animatedInstanceCurrentPosePool[instanceGlobalID] = pools->transformPool + transformGlobalID;
}

//same as MOT1 except poolType
void ttPauseAnimationInPool0(TTAnimationPools0* pools, uint8_t modelID, uint8_t instanceID) {
    uint32_t instanceOffset = pools->animatedInstanceOffsetByModelPool[modelID];
    TTAnimationState* animationState = pools->animationStatePool + instanceOffset + instanceID;
    animationState->frameCountBeforeNextPose = -1;
}

//same as MOT1 except poolType
void ttUnpauseAnimationInPool0(TTAnimationPools0* pools, uint8_t modelID, uint8_t instanceID) {
    uint32_t instanceOffset = pools->animatedInstanceOffsetByModelPool[modelID];
    TTAnimationState* animationState = pools->animationStatePool + instanceOffset + instanceID;
    animationState->frameCountBeforeNextPose = animationState->totalFrameCountBetweenPoses;
}

//same as MOT1 except poolType
bool ttIsAnimationPausedInPool0(TTAnimationPools0* pools, uint8_t modelID, uint8_t instanceID) {
    uint32_t instanceOffset = pools->animatedInstanceOffsetByModelPool[modelID];
    TTAnimationState* animationState = pools->animationStatePool + instanceOffset + instanceID;
    return animationState->frameCountBeforeNextPose == -1;
}

//Use StartAnimation to reverse a deactivation
void ttDeactivateInstanceInPool0(TTAnimationPools0* pools, uint8_t modelID, uint8_t instanceID) {
    uint32_t instanceOffset = pools->animatedInstanceOffsetByModelPool[modelID]; //3 first lines are from ttPauseAnimationInPool
    TTAnimationState* animationState = pools->animationStatePool + instanceOffset + instanceID;
    animationState->frameCountBeforeNextPose = -1;
    pools->animatedInstanceCurrentPosePool[instanceOffset + instanceID] = NULL;
}

bool ttIsInstanceActiveInPool0(TTAnimationPools0* pools, uint8_t modelID, uint8_t instanceID) {//lines from ttIsAnimationPausedInPool
    uint32_t instanceOffset = pools->animatedInstanceOffsetByModelPool[modelID];
    TTAnimationState* animationState = pools->animationStatePool + instanceOffset + instanceID;
    return animationState->frameCountBeforeNextPose == -1 && pools->animatedInstanceCurrentPosePool[instanceOffset + instanceID] == NULL;
}

//Code heavily based on core/animation.c/ttUpdateAnimation but rewritten here to optimize it for pools
void ttUpdateAnimationsInPool0(TTAnimationPools0* pools) {
    TTAnimationSetSpec0* animationSetSpec = pools->animationSetSpecPool;
    const uint8_t* restrict instanceCount = pools->animatedInstanceCountByModelPool;
    TTAnimationState* animationStatePtr = pools->animationStatePool;
    TTTransform** currentPosePtr = pools->animatedInstanceCurrentPosePool;
    for(int i = pools->nextFreeAnimationSetSpec - pools->animationSetSpecPool; i > 0; i--, animationSetSpec++, instanceCount++) {
        const uint32_t meshCount = (uint32_t) animationSetSpec->meshCount;
        for(int j = *instanceCount; j > 0; j--, animationStatePtr++, currentPosePtr++) {
            if(__builtin_expect(animationStatePtr->frameCountBeforeNextPose != -1 && --(animationStatePtr->frameCountBeforeNextPose) < 0, 1)) {
                animationStatePtr->frameCountBeforeNextPose = animationStatePtr->totalFrameCountBetweenPoses;
                if(__builtin_expect(--(animationStatePtr->currentPoseID) < 0, 0)) {
                    animationStatePtr->currentPoseID = animationStatePtr->totalPoseCount - 1;
                    (*currentPosePtr) += (animationStatePtr->totalPoseCount-1)*meshCount;
                } else
                    (*currentPosePtr)-=meshCount;
            }
        }
    }
}

void ttDrawAnimatedModelsFromPool0(TTModelPools* modelPools, TTAnimationPools0* animationPools) {
    const TTModelSpec* restrict modelSpec = modelPools->modelSpecPool;
    const TTAnimationSetSpec0* restrict animationSetSpec = animationPools->animationSetSpecPool;
    const uint8_t* restrict instanceCount = animationPools->animatedInstanceCountByModelPool;
    TTInstancePosition* restrict position = animationPools->animatedInstanceCurrentPositionPool;
    TTTransform** restrict currentPose = animationPools->animatedInstanceCurrentPosePool;
    // je suppose que le tableau de models et d'animationSet corespondent (même taille, même entity au même indice), forcer ça by design
    for(int i = modelPools->nextFreeModelSpec - modelPools->modelSpecPool; i > 0; i--, modelSpec++, animationSetSpec++, instanceCount++) {
        PDATA* restrict localModel = modelPools->meshPool + modelSpec->meshID;
        const uint32_t localMeshCount = (uint32_t) animationSetSpec->meshCount;
        for(int j = *instanceCount; j > 0; j--, position++, currentPose++) {
            if(__builtin_expect(*currentPose != NULL, 1)) {
                slPushMatrix();
                slTranslate(position->xOffset, position->yOffset, position->zOffset);
                slRotX(position->xAngle);
                slRotY(position->yAngle);
                slRotZ(position->zAngle);
                ttDrawAnimatedModelAtPose0(localModel, *currentPose, localMeshCount);
                slPopMatrix();
            }
        }
    }
}
