#ifndef ANIMATION_H
#define ANIMATION_H

#include "../core/animation.h"

#include TT_INCLUDE_VERSION(format/animation, TT_MOT)
#define TTAnimationPools TT_APPEND_MOT_VERSION(TTAnimationPools)

#define TT_ALLOC_ANIMATION_POOLS(variableName, maxModelCnt, maxMeshCnt, maxPoseCnt, maxAnimationCnt, animationAttribute, maxTransformCnt, transformAttribute, maxInstanceCnt, instanceAttribute) \
    TT_APPEND_MOT_VERSION(TT_ALLOC_ANIMATION_POOLS_)(variableName, maxModelCnt, maxMeshCnt, maxPoseCnt, maxAnimationCnt, animationAttribute, maxTransformCnt, transformAttribute, maxInstanceCnt, instanceAttribute)

#define ttInitAnimationPools(aniamtionPools, nextFreeHRAMBytePtr, nextFreeLRAMBytePtr, maxAnimationSetCnt, maxMeshCnt, maxPoseCnt, \
        maxAnimationCnt, animationDataRAMType, maxTransformCnt, transformDataRAMType, maxInstanceCnt, instanceDataRAMType) \
    TT_APPEND_MOT_VERSION(ttInitAnimationPools)(aniamtionPools, nextFreeHRAMBytePtr, nextFreeLRAMBytePtr, maxAnimationSetCnt, maxMeshCnt, maxPoseCnt, \
        maxAnimationCnt, animationDataRAMType, maxTransformCnt, transformDataRAMType, maxInstanceCnt, instanceDataRAMType)
#define ttResetAnimationPools(animationPools) TT_APPEND_MOT_VERSION(ttResetAnimationPools)(animationPools)
#define ttLoadAnimationInPool(modelPools, animationPools, fileName) TT_APPEND_MOT_VERSION(ttLoadAnimationInPool)(modelPools, animationPools, fileName)
#define ttGetAnimationRAMUsageFromMetaData(pools, hRAMUsage, lRAMUsage, metaData, instanceCount) TT_APPEND_MOT_VERSION(ttGetAnimationRAMUsageFromMetaData)(pools, hRAMUsage, lRAMUsage, metaData, instanceCount)
#define ttGetCurrentAnimationRAMUsage(modelPools, animationPools, hRAMUsage, lRAMUsage) TT_APPEND_MOT_VERSION(ttGetCurrentAnimationRAMUsage)(modelPools, animationPools, hRAMUsage, lRAMUsage)
#define ttGetReservedAnimationRAMUsage(pools, hRAMUsage, lRAMUsage) TT_APPEND_MOT_VERSION(ttGetReservedAnimationRAMUsage)(pools, hRAMUsage, lRAMUsage)
#define ttGetModelAnimationCount(animationPools, modelID) animationPools.animationSetSpecPool[modelID].animationCount
#define ttGetIntanceCurrentPosition(animationPools, modelID, instanceID) animationPools.animatedInstanceCurrentPositionPool[animationPools.animatedInstanceOffsetByModelPool[modelID] + instanceID]
#define ttSetAnimatedInstancePositionInPool(animationPools, modelID, instanceID, transform) TT_APPEND_MOT_VERSION(ttSetAnimatedInstancePositionInPool)(animationPools, modelID, instanceID, transform)
#define ttSetAnimatedInstanceCountInPool(animationPools, animatedModelCount,instanceCounts) TT_APPEND_MOT_VERSION(ttSetAnimatedInstanceCountInPool)(animationPools, animatedModelCount,instanceCounts)
#define ttStartAnimationInPool(animationPools, modelID, instanceID, animationSpecID) TT_APPEND_MOT_VERSION(ttStartAnimationInPool)(animationPools, modelID, instanceID, animationSpecID)
#define ttPauseAnimationInPool(animationPools, modelID, instanceID) TT_APPEND_MOT_VERSION(ttPauseAnimationInPool)(animationPools, modelID, instanceID)
#define ttUnpauseAnimationInPool(animationPools, modelID, instanceID) TT_APPEND_MOT_VERSION(ttUnpauseAnimationInPool)(animationPools, modelID, instanceID)
#define ttIsAnimationPausedInPool(animationPools, modelID, instanceID) TT_APPEND_MOT_VERSION(ttIsAnimationPausedInPool)(animationPools, modelID, instanceID)
#define ttDeactivateInstanceInPool(animationPools, modelID, instanceID) TT_APPEND_MOT_VERSION(ttDeactivateInstanceInPool)(animationPools, modelID, instanceID)
#define ttIsInstanceActiveInPool(animationPools, modelID, instanceID) TT_APPEND_MOT_VERSION(ttIsInstanceActiveInPool)(animationPools, modelID, instanceID)
#define ttUpdateAnimationsInPool(animationPools) TT_APPEND_MOT_VERSION(ttUpdateAnimationsInPool)(animationPools)
#define ttDrawAnimatedModelsFromPool(modelPools, animationPools) TT_APPEND_MOT_VERSION(ttDrawAnimatedModelsFromPool)(modelPools, animationPools)


// #ifdef TT_MOT_000_000

//     #include "format/animation0.h"
//     #define TTAnimationPools TTAnimationPools0

//     #define TT_ALLOC_ANIMATION_POOLS(variableName, maxModelCnt, maxMeshCnt, maxPoseCnt, maxAnimationCnt, animationAttribute, maxTransformCnt, transformAttribute, maxInstanceCnt, instanceAttribute) \
//         TT_ALLOC_ANIMATION_POOLS_0(variableName, maxModelCnt, maxMeshCnt, maxAnimationCnt, animationAttribute, maxTransformCnt, transformAttribute, maxInstanceCnt, instanceAttribute)

//     #define ttResetAnimationPools(animationPools) ttResetAnimationPools0(animationPools)
//     #define ttLoadAnimationInPool(modelPools, animationPools, fileName) ttLoadAnimationInPool0(modelPools, animationPools, fileName)
//     #define ttGetAnimationRAMUsageFromMetaData(pools, hRAMUsage, lRAMUsage, metaData, instanceCount) ttGetAnimationRAMUsageFromMetaData0(pools, hRAMUsage, lRAMUsage, metaData, instanceCount)
//     #define ttGetCurrentAnimationRAMUsage(modelPools, animationPools, hRAMUsage, lRAMUsage) ttGetCurrentAnimationRAMUsage0(modelPools, animationPools, hRAMUsage, lRAMUsage)
//     #define ttGetReservedAnimationRAMUsage(pools, hRAMUsage, lRAMUsage) ttGetReservedAnimationRAMUsage0(pools, hRAMUsage, lRAMUsage)
//     #define ttGetModelAnimationCount(animationPools, modelID) animationPools.animationSetSpecPool[modelID].animationCount
//     #define ttGetIntanceCurrentPosition(animationPools, modelID, instanceID) animationPools.animatedInstanceCurrentPositionPool[animationPools.animatedInstanceOffsetByModelPool[modelID] + instanceID]
//     #define ttSetAnimatedInstancePositionInPool(animationPools, modelID, instanceID, transform) ttSetAnimatedInstancePositionInPool0(animationPools, modelID, instanceID, transform)
//     #define ttSetAnimatedInstanceCountInPool(animationPools, animatedModelCount,instanceCounts) ttSetAnimatedInstanceCountInPool0(animationPools, animatedModelCount,instanceCounts)
//     #define ttStartAnimationInPool(animationPools, modelID, instanceID, animationSpecID) ttStartAnimationInPool0(animationPools, modelID, instanceID, animationSpecID)
//     #define ttPauseAnimationInPool(animationPools, modelID, instanceID) ttPauseAnimationInPool0(animationPools, modelID, instanceID)
//     #define ttUnpauseAnimationInPool(animationPools, modelID, instanceID) ttUnpauseAnimationInPool0(animationPools, modelID, instanceID)
//     #define ttIsAnimationPausedInPool(animationPools, modelID, instanceID) ttIsAnimationPausedInPool0(animationPools, modelID, instanceID)
//     #define ttDeactivateInstanceInPool(animationPools, modelID, instanceID) ttDeactivateInstanceInPool0(animationPools, modelID, instanceID)
//     #define ttIsInstanceActiveInPool(animationPools, modelID, instanceID) ttIsInstanceActiveInPool0(animationPools, modelID, instanceID)
//     #define ttUpdateAnimationsInPool(animationPools) ttUpdateAnimationsInPool0(animationPools)
//     #define ttDrawAnimatedModelsFromPool(modelPools, animationPools) ttDrawAnimatedModelsFromPool0(modelPools, animationPools)

// #elif defined(TT_MOT_001_000) //TT_MOT_000_000 ends

//     #include "format/animation1.h"
//     #define TTAnimationPools TTAnimationPools1

//     #define TT_ALLOC_ANIMATION_POOLS(variableName, maxModelCnt, maxMeshCnt, maxPoseCnt, maxAnimationCnt, animationAttribute, maxTransformCnt, transformAttribute, maxInstanceCnt, instanceAttribute) \
//         TT_ALLOC_ANIMATION_POOLS_1(variableName, maxModelCnt, maxMeshCnt, maxPoseCnt, maxAnimationCnt, animationAttribute, maxTransformCnt, transformAttribute, maxInstanceCnt, instanceAttribute)

//     #define ttResetAnimationPools(animationPools) ttResetAnimationPools1(animationPools)
//     #define ttLoadAnimationInPool(modelPools, animationPools, fileName) ttLoadAnimationInPool1(modelPools, animationPools, fileName)
//     #define ttGetAnimationRAMUsageFromMetaData(pools, hRAMUsage, lRAMUsage, metaData, instanceCount) ttGetAnimationRAMUsageFromMetaData1(pools, hRAMUsage, lRAMUsage, metaData, instanceCount)
//     #define ttGetCurrentAnimationRAMUsage(modelPools, animationPools, hRAMUsage, lRAMUsage) ttGetCurrentAnimationRAMUsage1(modelPools, animationPools, hRAMUsage, lRAMUsage)
//     #define ttGetReservedAnimationRAMUsage(animationPools, hRAMUsage, lRAMUsage) ttGetReservedAnimationRAMUsage1(animationPools, hRAMUsage, lRAMUsage)
//     #define ttGetModelAnimationCount(animationPools, modelID) animationPools.animationSetSpecPool[modelID].animationCount
//     #define ttGetIntanceCurrentPosition(animationPools, modelID, instanceID) animationPools.animatedInstanceCurrentPositionPool[animationPools.animatedInstanceOffsetByModelPool[modelID] + instanceID]
//     #define ttSetAnimatedInstancePositionInPool(animationPools, modelID, instanceID, transform) ttSetAnimatedInstancePositionInPool1(animationPools, modelID, instanceID, transform)
//     #define ttSetAnimatedInstanceCountInPool(animationPools, animatedModelCount,instanceCounts) ttSetAnimatedInstanceCountInPool1(animationPools, animatedModelCount,instanceCounts)
//     #define ttStartAnimationInPool(animationPools, modelID, instanceID, animationSpecID) ttStartAnimationInPool1(animationPools, modelID, instanceID, animationSpecID)
//     #define ttPauseAnimationInPool(animationPools, modelID, instanceID) ttPauseAnimationInPool1(animationPools, modelID, instanceID)
//     #define ttUnpauseAnimationInPool(animationPools, modelID, instanceID) ttUnpauseAnimationInPool1(animationPools, modelID, instanceID)
//     #define ttIsAnimationPausedInPool(animationPools, modelID, instanceID) ttIsAnimationPausedInPool1(animationPools, modelID, instanceID)
//     #define ttDeactivateInstanceInPool(animationPools, modelID, instanceID) ttDeactivateInstanceInPool1(animationPools, modelID, instanceID)
//     #define ttIsInstanceActiveInPool(animationPools, modelID, instanceID) ttIsInstanceActiveInPool1(animationPools, modelID, instanceID)
//     #define ttUpdateAnimationsInPool(animationPools) ttUpdateAnimationsInPool1(animationPools)
//     #define ttDrawAnimatedModelsFromPool(modelPools, animationPools) ttDrawAnimatedModelsFromPool1(modelPools, animationPools)

// #endif //TT_MOT_001_000

#endif //ANIMATION_H