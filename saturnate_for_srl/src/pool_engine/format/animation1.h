#ifndef TT_POOL_ANIMATION1_H
#define TT_POOL_ANIMATION1_H

#include <sl_def.h>
#include "animation_common.h"
#include "../../core/format/animation1.h"
#include "../../core/memory.h"
#include "../model_loader.h"

/**
 * @brief Structure representing several animations for a model.
 *
 * The data layout is intended to corresponds to the header of the .MOT file format used by this library,
 * that way a TTMOTFileHeader can be easily casted into this strucure.
 * Only the file version fields are replaced by a transformOffset.
 * 
 * @warning Do not edit manually.
 */
typedef struct {
    uint16_t poseOffset; /** Global index of the first pose of this animation set, i.e sum of all the preceeding animation set's pose count. */
    uint8_t animationCount; /** Number of animations in this set. */
    uint8_t meshCount; /** Number of meshes composing the animated model */
} TTAnimationSetSpec1;

typedef struct {
    TTBoneRotation** animatedInstanceCurrentPosePool;
    TTRootBonePosition** animatedInstanceCurrentRootBonePositionPool;
    TTInstancePosition* animatedInstanceCurrentPositionPool;

    uint8_t* animationSpecOffsetByModelPool;
    uint8_t* nextFreeAnimationSpecOffsetByModel;
    uint8_t* animatedInstanceCountByModelPool;
    uint8_t* animatedInstanceOffsetByModelPool;
    TTAnimationState* animationStatePool;

    uint16_t* transformOffsetByAnimationPool;
    uint16_t* nextFreeTransformOffsetByAnimation;
    uint16_t* transformOffsetByAnimationSetPool;
    uint16_t* nextFreeTransformOffsetByAnimationSet; 

    TTBoneRotation* transformPool;
    TTBoneRotation* nextFreeTransform;
    TTRootBonePosition* rootBonePositionByPosePool;
    TTRootBonePosition* nextFreeRootBonePositionByPose;
    TTBone* bonePool;
    TTBone* nextFreeBone;
    TTAnimationSpec1* animationSpecPool;
    TTAnimationSpec1* nextFreeAnimationSpec;
    TTAnimationSetSpec1* animationSetSpecPool;
    TTAnimationSetSpec1* nextFreeAnimationSetSpec;

    int32_t maxTransformCount;
    int32_t maxAnimationCount;
    int32_t maxAnimationSetCount;
    int32_t maxPoseCount;
    int32_t maxAnimatedInstanceCount;
    int32_t maxMeshCount;//adapte la création du struct due à l'ajout de ce champ
} TTAnimationPools1;

// #define TT_ANIMATION_RAM_USAGE_1(animationSetCount, animationCount, transformCount) (animationSetCount * sizeof(TTAnimationSetSpec1) + sizeof(uint8_t)*3 + animationCount * sizeof(TTAnimationSpec1) + transformCount * sizeof(TTTransform))
#define TT_INSTANCE_RAM_USAGE_1() (sizeof(TTAnimationState) + sizeof(TTInstancePosition) + sizeof(TTTransform*))
#define TT_ANIMATION_RAM_USAGE_1(animationSetCount,  meshCount, animationCount, poseCount, instanceCount) \
    ((meshCount) * (poseCount) * sizeof(TTBoneRotation) +  \
    + (meshCount) * (sizeof(TTBone)) \
    + (poseCount) * (sizeof(TTRootBonePosition)) \
    + (animationCount) * (sizeof(uint16_t) + sizeof(TTAnimationSpec1)) \
    + (animationSetCount) * (sizeof(uint8_t) * 3 + sizeof(uint16_t) + sizeof(TTAnimationSetSpec1)) \
    + (instanceCount) * (sizeof(TTRootBonePosition) + sizeof(TTInstancePosition))
#define TT_ANIMATION_POOL_RESERVED_RAM_USAGE_1(animationPool) \
    (TT_ANIMATION_RAM_USAGE_1(animationPool->maxAnimationSetCount, ??, animationPool->maxAnimationCount, \
        animationPool->maxPoseCount, animationPool->maxAnimatedInstanceCount))

void ttInitAnimationPools1(TTAnimationPools1* pools, uint8_t** nextFreeHRAMBytePtr, uint8_t** nextFreeLRAMBytePtr, uint32_t maxAnimationSetCnt,
                            uint32_t maxMeshCnt, uint32_t maxPoseCnt, uint32_t maxAnimationCnt, TT_RAM_TYPE animationDataRAMType,
                            uint32_t maxTransformCnt, TT_RAM_TYPE transformDataRAMType,
                            uint32_t maxInstanceCnt, TT_RAM_TYPE instanceDataRAMType);
void ttResetAnimationPools1(TTAnimationPools1* pools);
TTAnimationPoolLoadStatus ttLoadAnimationInPool1(TTModelPools* modelPools, TTAnimationPools1* animationPools, char* filename);
void ttGetAnimationRAMUsage1(TTAnimationPools1* pools, uint32_t* hRAMUsage, uint32_t* lRAMUsage,
                                uint32_t animationSetCount, uint32_t animationCount, uint32_t meshCount,
                                uint32_t poseCount, uint32_t transformCount, uint32_t instanceCount);
void ttGetCurrentAnimationRAMUsage1(TTModelPools* modelPools, TTAnimationPools1* animationPools, uint32_t* hRAMUsage, uint32_t* lRAMUsage);
int32_t ttSetAnimatedInstanceCountInPool1(TTAnimationPools1* pools, uint8_t animatedModelCount, uint8_t instanceCounts[animatedModelCount]);
void ttStartAnimationInPool1(TTAnimationPools1* pools, uint8_t modelID, uint8_t instanceID, uint8_t animationSpecID);
void ttPauseAnimationInPool1(TTAnimationPools1* pools, uint8_t modelID, uint8_t instanceID);
void ttUnpauseAnimationInPool1(TTAnimationPools1* pools, uint8_t modelID, uint8_t instanceID);
bool ttIsAnimationPausedInPool1(TTAnimationPools1* pools, uint8_t modelID, uint8_t instanceID);
void ttDeactivateInstanceInPool1(TTAnimationPools1* pools, uint8_t modelID, uint8_t instanceID);
bool ttIsInstanceActiveInPool1(TTAnimationPools1* pools, uint8_t modelID, uint8_t instanceID);
void ttUpdateAnimationsInPool1(TTAnimationPools1* pools);
void ttDrawAnimatedModelsFromPool1(TTModelPools* modelPools, TTAnimationPools1* pools);

static inline __attribute__((always_inline)) void ttGetAnimationRAMUsageFromMetaData1(TTAnimationPools1* pools, uint32_t* hRAMUsage, uint32_t* lRAMUsage, TTMOTFileMetadata1* metaData, uint32_t instanceCount) {
    ttGetAnimationRAMUsage1(pools, hRAMUsage, lRAMUsage, 1, metaData->animationCount, metaData->meshCount,
                            metaData->totalPoseCount, metaData->meshCount * metaData->totalPoseCount, instanceCount);
}

static inline __attribute__((always_inline)) void ttGetReservedAnimationRAMUsage1(TTAnimationPools1* pools, uint32_t* hRAMUsage, uint32_t* lRAMUsage) {
    ttGetAnimationRAMUsage1(pools, hRAMUsage, lRAMUsage, pools->maxAnimationSetCount, pools->maxAnimationCount,
        pools->maxMeshCount, pools->maxPoseCount, pools->maxTransformCount, pools->maxAnimatedInstanceCount);
}

static inline __attribute__((always_inline)) void ttSetAnimatedInstancePositionInPool1(TTAnimationPools1* pools, uint8_t modelID, uint8_t instanceID, TTInstancePosition transform) {
    pools->animatedInstanceCurrentPositionPool[pools->animatedInstanceOffsetByModelPool[modelID] + instanceID] = transform;
}

#define TT_ALLOC_ANIMATION_POOLS_1(variableName, maxAnimationSetCnt, maxMeshCnt, maxPoseCnt, maxAnimationCnt, animationAttribute, maxTransformCnt, transformAttribute, maxInstanceCnt, instanceAttribute) \
    TTBoneRotation variableName##_transformPool[maxTransformCnt] __attribute__((aligned(4))) transformAttribute; \
    TTAnimationSpec1 variableName##_animationSpecPool[maxAnimationCnt] __attribute__((aligned(4))) animationAttribute; \
    uint16_t variableName##_transformOffsetByAnimationPool[maxAnimationCnt] __attribute__((aligned(4))) animationAttribute; \
    TTRootBonePosition variableName##_rootBonePositionByPosePool[maxPoseCnt] transformAttribute; \
    TTBone variableName##_bonePool[maxMeshCnt] __attribute__((aligned(4))) transformAttribute; \
    TTAnimationSetSpec1 variableName##_animationSetSpecPool[maxAnimationSetCnt] __attribute__((aligned(4))) animationAttribute; \
    uint16_t variableName##_transformOffsetByAnimationSetPool[maxAnimationSetCnt] __attribute__((aligned(4))); animationAttribute \
    uint8_t variableName##_animationSpecOffsetByModelPool[maxAnimationSetCnt] __attribute__((aligned(4))) animationAttribute; \
    uint8_t variableName##_animatedInstanceCountByModelPool[maxAnimationSetCnt] __attribute__((aligned(4))) instanceAttribute; \
    uint8_t variableName##_animatedInstanceOffsetByModelPool[maxAnimationSetCnt] __attribute__((aligned(4))) instanceAttribute; \
    TTAnimationState variableName##_animationStatePool[maxInstanceCnt] __attribute__((aligned(4))) instanceAttribute; \
    TTInstancePosition variableName##_animatedInstanceCurrentPositionPool[maxInstanceCnt] __attribute__((aligned(4))) instanceAttribute; \
    TTBoneRotation* variableName##_animatedInstanceCurrentPosePool[maxInstanceCnt] __attribute__((aligned(4))) instanceAttribute; \
    TTRootBonePosition* variableName##_animatedInstanceCurrentRootBonePositionPool[maxInstanceCnt] __attribute__((aligned(4))) instanceAttribute; \
    TTAnimationPools1 variableName = { \
        .animatedInstanceCurrentPosePool = variableName##_animatedInstanceCurrentPosePool, \
        .animatedInstanceCurrentRootBonePositionPool = variableName##_animatedInstanceCurrentRootBonePositionPool, \
        .animatedInstanceCurrentPositionPool = variableName##_animatedInstanceCurrentPositionPool, \
            \
        .animationSpecOffsetByModelPool = variableName##_animationSpecOffsetByModelPool, \
        .nextFreeAnimationSpecOffsetByModel = variableName##_animationSpecOffsetByModelPool,  \
        .animatedInstanceCountByModelPool = variableName##_animatedInstanceCountByModelPool, \
        .animatedInstanceOffsetByModelPool = variableName##_animatedInstanceOffsetByModelPool, \
        .animationStatePool = variableName##_animationStatePool, \
            \
        .transformOffsetByAnimationPool = variableName##_transformOffsetByAnimationPool, \
        .nextFreeTransformOffsetByAnimation = variableName##_transformOffsetByAnimationPool, \
        .transformOffsetByAnimationSetPool = variableName##_transformOffsetByAnimationSetPool, \
        .nextFreeTransformOffsetByAnimationSet = variableName##_transformOffsetByAnimationSetPool, \
            \
        .transformPool = variableName##_transformPool, \
        .nextFreeTransform = variableName##_transformPool, \
        .rootBonePositionByPosePool = variableName##_rootBonePositionByPosePool, \
        .nextFreeRootBonePositionByPose = variableName##_rootBonePositionByPosePool, \
        .bonePool = variableName##_bonePool, \
        .nextFreeBone = variableName##_bonePool, \
        .animationSpecPool = variableName##_animationSpecPool, \
        .nextFreeAnimationSpec = variableName##_animationSpecPool, \
        .animationSetSpecPool = variableName##_animationSetSpecPool, \
        .nextFreeAnimationSetSpec = variableName##_animationSetSpecPool, \
        .maxTransformCount = maxTransformCnt, \
        .maxAnimationSetCount = maxAnimationSetCnt, \
        .maxAnimationCount = maxAnimationCnt, \
        .maxPoseCount = maxPoseCnt, \
        .maxAnimatedInstanceCount = maxInstanceCnt, \
        .maxMeshCount = maxMeshCnt \
    };

// #define TT_ALLOC_ANIMATION_POOLS_1(variableName, maxAnimationSetCnt, maxMeshCnt, maxPoseCnt, maxAnimationCnt, animationAttribute, maxTransformCnt, transformAttribute, maxInstanceCnt, instanceAttribute) \
//     TTAnimationSpec1 variableName##_animationSpecPool[maxAnimationCnt] __attribute__((aligned(4))) animationAttribute; \
//     uint16_t variableName##_transformOffsetByAnimationPool[maxAnimationCnt] __attribute__((aligned(4))) animationAttribute; \
//     TTRootBonePosition variableName##_rootBonePositionByPosePool[maxPoseCnt] animationAttribute; \
//     TTBone variableName##_bonePool[maxMeshCnt] __attribute__((aligned(4))) animationAttribute; \
//     TTAnimationSetSpec1 variableName##_animationSetSpecPool[maxAnimationSetCnt] __attribute__((aligned(4))) animationAttribute; \
//     uint16_t variableName##_transformOffsetByAnimationSetPool[maxAnimationSetCnt] __attribute__((aligned(4))); animationAttribute \
//     uint8_t variableName##_animationSpecOffsetByModelPool[maxAnimationSetCnt] __attribute__((aligned(4))) animationAttribute; \
//     uint8_t variableName##_animatedInstanceCountByModelPool[maxAnimationSetCnt] __attribute__((aligned(4))) instanceAttribute; \
//     uint8_t variableName##_animatedInstanceOffsetByModelPool[maxAnimationSetCnt] __attribute__((aligned(4))) instanceAttribute; \
//     TTAnimationState variableName##_animationStatePool[maxInstanceCnt] __attribute__((aligned(4))) instanceAttribute; \
//     TTInstancePosition variableName##_animatedInstanceCurrentPositionPool[maxInstanceCnt] __attribute__((aligned(4))) instanceAttribute; \
//     TTBoneRotation* variableName##_animatedInstanceCurrentPosePool[maxInstanceCnt] __attribute__((aligned(4))) instanceAttribute; \
//     TTRootBonePosition* variableName##_animatedInstanceCurrentRootBonePositionPool[maxInstanceCnt] __attribute__((aligned(4))) instanceAttribute; \
//     TTAnimationPools1 variableName = { \
//         .animatedInstanceCurrentPosePool = variableName##_animatedInstanceCurrentPosePool, \
//         .animatedInstanceCurrentRootBonePositionPool = variableName##_animatedInstanceCurrentRootBonePositionPool, \
//         .animatedInstanceCurrentPositionPool = variableName##_animatedInstanceCurrentPositionPool, \
//             \
//         .animationSpecOffsetByModelPool = variableName##_animationSpecOffsetByModelPool, \
//         .nextFreeAnimationSpecOffsetByModel = variableName##_animationSpecOffsetByModelPool,  \
//         .animatedInstanceCountByModelPool = variableName##_animatedInstanceCountByModelPool, \
//         .animatedInstanceOffsetByModelPool = variableName##_animatedInstanceOffsetByModelPool, \
//         .animationStatePool = variableName##_animationStatePool, \
//             \
//         .transformOffsetByAnimationPool = variableName##_transformOffsetByAnimationPool, \
//         .nextFreeTransformOffsetByAnimation = variableName##_transformOffsetByAnimationPool, \
//         .transformOffsetByAnimationSetPool = variableName##_transformOffsetByAnimationSetPool, \
//         .nextFreeTransformOffsetByAnimationSet = variableName##_transformOffsetByAnimationSetPool, \
//             \
//         .transformPool = (TTBoneRotation*) 0x00200000, \
//         .nextFreeTransform = (TTBoneRotation*) 0x00200000, \
//         .rootBonePositionByPosePool = variableName##_rootBonePositionByPosePool, \
//         .nextFreeRootBonePositionByPose = variableName##_rootBonePositionByPosePool, \
//         .bonePool = variableName##_bonePool, \
//         .nextFreeBone = variableName##_bonePool, \
//         .animationSpecPool = variableName##_animationSpecPool, \
//         .nextFreeAnimationSpec = variableName##_animationSpecPool, \
//         .animationSetSpecPool = variableName##_animationSetSpecPool, \
//         .nextFreeAnimationSetSpec = variableName##_animationSetSpecPool, \
//         .maxTransformCount = maxTransformCnt, \
//         .maxAnimationSetCount = maxAnimationSetCnt, \
//         .maxAnimationCount = maxAnimationCnt, \
//         .maxPoseCount = maxPoseCnt, \
//         .maxAnimatedInstanceCount = maxInstanceCnt, \
//         .maxMeshCount = maxMeshCnt \
//     };

#endif //TT_POOL_ANIMATION1_H