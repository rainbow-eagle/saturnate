#ifndef TT_POOL_ANIMATION0_H
#define TT_POOL_ANIMATION0_H

#include <sl_def.h>
#include "animation_common.h"
#include "../../core/format/animation0.h"
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
    uint16_t transformOffset; /** Pool index of the first TTTransform of this animation set. */
    uint8_t animationCount; /** Number of animations in this set. */
    uint8_t meshCount; /** Number of meshes composing the animated model */
} TTAnimationSetSpec0;

typedef struct {
    TTTransform** animatedInstanceCurrentPosePool;
    TTInstancePosition* animatedInstanceCurrentPositionPool;

    uint8_t* animationSpecOffsetByModelPool;
    uint8_t* nextFreeAnimationSpecOffsetByModel;
    uint8_t* animatedInstanceCountByModelPool;
    uint8_t* animatedInstanceOffsetByModelPool;
    TTAnimationState* animationStatePool;

    TTTransform* transformPool;
    TTTransform* nextFreeTransform;
    TTAnimationSpec0* animationSpecPool;
    TTAnimationSpec0* nextFreeAnimationSpec;
    TTAnimationSetSpec0* animationSetSpecPool;
    TTAnimationSetSpec0* nextFreeAnimationSetSpec;

    int32_t maxTransformCount;
    int32_t maxAnimationCount;
    int32_t maxAnimationSetCount;
    int32_t maxAnimatedInstanceCount;
} TTAnimationPools0;

// #define TT_ANIMATION_RAM_USAGE_0(animationSetCount, animationCount, transformCount) (animationSetCount * sizeof(TTAnimationSetSpec0) + sizeof(uint8_t)*3 + animationCount * sizeof(TTAnimationSpec0) + transformCount * sizeof(TTTransform))
#define TT_INSTANCE_RAM_USAGE_0() (sizeof(TTAnimationState) + sizeof(TTInstancePosition) + sizeof(TTTransform*))
#define TT_ANIMATION_RAM_USAGE_0(animationSetCount, animationCount, transformCount, instanceCount) \
    ((transformCount) * sizeof(TTTransform) + (animationCount) * sizeof(TTAnimationSpec0) \
    + (animationSetCount) * (sizeof(TTAnimationSetSpec0) + sizeof(uint8_t) * 3) \
    + (instanceCount) * (sizeof(TTAnimationState) + sizeof(TTInstancePosition) + sizeof(TTTransform)))
#define TT_ANIMATION_POOL_RESERVED_RAM_USAGE_0(animationPool) \
    (TT_ANIMATION_RAM_USAGE_0((animationPool)->maxAnimationSetCount, (animationPool)->maxAnimationCount, \
        (animationPool)->maxTransformCount, (animationPool)->maxAnimatedInstanceCount))


void ttInitAnimationPoolsX0(TTAnimationPools0* pools, uint8_t** nextFreeHRAMBytePtr, uint8_t** nextFreeLRAMBytePtr, uint32_t maxAnimationSetCount,
                            uint32_t maxMeshCount, uint32_t maxAnimationCount, TT_RAM_TYPE animationDataRAMType,
                            uint32_t maxTransformCount, TT_RAM_TYPE transformDataRAMType,
                            uint32_t maxInstanceCount, TT_RAM_TYPE instanceDataRAMType);
void ttInitAnimationPools0(TTAnimationPools0* pools, uint8_t** nextFreeHRAMBytePtr, uint8_t** nextFreeLRAMBytePtr, uint32_t maxAnimationSetCount,
                            uint32_t maxMeshCount, uint32_t maxPoseCnt, uint32_t maxAnimationCount, TT_RAM_TYPE animationDataRAMType,
                            uint32_t maxTransformCount, TT_RAM_TYPE transformDataRAMType,
                            uint32_t maxInstanceCount, TT_RAM_TYPE instanceDataRAMType);
void ttResetAnimationPools0(TTAnimationPools0* pools);
TTAnimationPoolLoadStatus ttLoadAnimationInPool0(TTModelPools* modelPools, TTAnimationPools0* animationPools, char* filename);
void ttGetAnimationRAMUsage0(TTAnimationPools0* pools, uint32_t* hRAMUsage, uint32_t* lRAMUsage,
                                uint32_t animationSetCount, uint32_t animationCount, uint32_t transformCount, uint32_t instanceCount);
void ttGetCurrentAnimationRAMUsage0(TTModelPools* modelPools, TTAnimationPools0* animationPools, uint32_t* hRAMUsage, uint32_t* lRAMUsage);
int32_t ttSetAnimatedInstanceCountInPool0(TTAnimationPools0* pools, uint8_t animatedModelCount, uint8_t instanceCounts[animatedModelCount]);
void ttStartAnimationInPool0(TTAnimationPools0* pools, uint8_t modelID, uint8_t instanceID, uint8_t animationSpecID);
void ttPauseAnimationInPool0(TTAnimationPools0* pools, uint8_t modelID, uint8_t instanceID);
void ttUnpauseAnimationInPool0(TTAnimationPools0* pools, uint8_t modelID, uint8_t instanceID);
bool ttIsAnimationPausedInPool0(TTAnimationPools0* pools, uint8_t modelID, uint8_t instanceID);
void ttDeactivateInstanceInPool0(TTAnimationPools0* pools, uint8_t modelID, uint8_t instanceID);
bool ttIsInstanceActiveInPool0(TTAnimationPools0* pools, uint8_t modelID, uint8_t instanceID);
void ttUpdateAnimationsInPool0(TTAnimationPools0* pools);
void ttDrawAnimatedModelsFromPool0(TTModelPools* modelPools, TTAnimationPools0* pools);

static inline __attribute__((always_inline)) void ttGetAnimationRAMUsageFromMetaData0(TTAnimationPools0* pools, uint32_t* hRAMUsage, uint32_t* lRAMUsage,
                                            TTMOTFileMetadata0* metaData, uint32_t instanceCount) {
    ttGetAnimationRAMUsage0(pools, hRAMUsage, lRAMUsage, 1, metaData->animationCount,
                            metaData->meshCount * metaData->totalPoseCount, instanceCount);
}

static inline __attribute__((always_inline)) void ttGetReservedAnimationRAMUsage0(TTAnimationPools0* pools, uint32_t* hRAMUsage, uint32_t* lRAMUsage) {
    ttGetAnimationRAMUsage0(pools, hRAMUsage, lRAMUsage, pools->maxAnimationSetCount,
                            pools->maxAnimationCount, pools->maxTransformCount, pools->maxAnimatedInstanceCount);
}

//sets the instance position. The way it works will probably change a lot.
static inline __attribute__((always_inline)) void ttSetAnimatedInstancePositionInPool0(TTAnimationPools0* pools, uint8_t modelID, uint8_t instanceID, TTInstancePosition transform) {
    pools->animatedInstanceCurrentPositionPool[pools->animatedInstanceOffsetByModelPool[modelID] + instanceID] = transform;
}

// #define TT_ALLOC_ANIMATION_POOLS_X_0(variableName, maxAnimationSetCnt, maxMeshCnt, maxAnimationCnt, animationAttribute, maxTransformCnt, transformAttribute, maxInstanceCnt, instanceAttribute) \
//     TTTransform variableName##_transformPool[maxTransformCnt] __attribute__((aligned(4))) transformAttribute; \
//     TTAnimationSpec0 variableName##_animationSpecPool[maxAnimationCnt] __attribute__((aligned(4))) animationAttribute; \
//     TTAnimationSetSpec0 variableName##_animationSetSpecPool[maxAnimationSetCnt] __attribute__((aligned(4))) animationAttribute; \
//     uint8_t variableName##_animationSpecOffsetByModelPool[maxAnimationSetCnt] __attribute__((aligned(4))) animationAttribute; \
//     uint8_t variableName##_animatedInstanceCountByModelPool[maxAnimationSetCnt] __attribute__((aligned(4))) instanceAttribute; \
//     uint8_t variableName##_animatedInstanceOffsetByModelPool[maxAnimationSetCnt] __attribute__((aligned(4))) instanceAttribute; \
//     TTAnimationState variableName##_animationStatePool[maxInstanceCnt] __attribute__((aligned(4))) instanceAttribute; \
//     TTInstancePosition variableName##_animatedInstanceCurrentPositionPool[maxInstanceCnt] __attribute__((aligned(4))) instanceAttribute; \
//     TTTransform* variableName##_animatedInstanceCurrentPosePool[maxInstanceCnt] __attribute__((aligned(4))) instanceAttribute; \
//     TTAnimationPools0 variableName = { \
//         .animatedInstanceCurrentPosePool     = variableName##_animatedInstanceCurrentPosePool, \
//         .animatedInstanceCurrentPositionPool = variableName##_animatedInstanceCurrentPositionPool, \
//         .animationSpecOffsetByModelPool      = variableName##_animationSpecOffsetByModelPool, \
//         .nextFreeAnimationSpecOffsetByModel  = variableName##_animationSpecOffsetByModelPool, \
//         .animatedInstanceCountByModelPool    = variableName##_animatedInstanceCountByModelPool, \
//         .animatedInstanceOffsetByModelPool   = variableName##_animatedInstanceOffsetByModelPool, \
//         .animationStatePool                  = variableName##_animationStatePool, \
//         .transformPool                       = variableName##_transformPool, \
//         .nextFreeTransform                   = variableName##_transformPool, \
//         .animationSpecPool                   = variableName##_animationSpecPool, \
//         .nextFreeAnimationSpec               = variableName##_animationSpecPool, \
//         .animationSetSpecPool                = variableName##_animationSetSpecPool, \
//         .nextFreeAnimationSetSpec            = variableName##_animationSetSpecPool, \
//         .maxTransformCount = maxTransformCnt, \
//         .maxAnimationCount = maxAnimationCnt, \
//         .maxAnimationSetCount = maxAnimationSetCnt, \
//         .maxAnimatedInstanceCount = maxInstanceCnt \
//     };

// #define TT_ALLOC_ANIMATION_POOLS_0(variableName, maxAnimationSetCnt, maxMeshCnt, maxPoseCnt, maxAnimationCnt, animationAttribute, maxTransformCnt, transformAttribute, maxInstanceCnt, instanceAttribute) \
//             TT_ALLOC_ANIMATION_POOLS_X_0(variableName, maxAnimationSetCnt, maxMeshCnt, maxAnimationCnt, animationAttribute, maxTransformCnt, transformAttribute, maxInstanceCnt, instanceAttribute)

#endif //TT_POOL_ANIMATION0_H