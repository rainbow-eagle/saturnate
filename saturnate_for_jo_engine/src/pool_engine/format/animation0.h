#ifndef TT_POOL_ANIMATION0_H
#define TT_POOL_ANIMATION0_H

#include <jo/sgl_prototypes.h>
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
    Uint16 transformOffset; /** Pool index of the first TTTransform of this animation set. */
    Uint8 animationCount; /** Number of animations in this set. */
    Uint8 meshCount; /** Number of meshes composing the animated model */
} TTAnimationSetSpec0;

typedef struct {
    TTTransform** animatedInstanceCurrentPosePool;
    TTInstancePosition* animatedInstanceCurrentPositionPool;

    Uint8* animationSpecOffsetByModelPool;
    Uint8* nextFreeAnimationSpecOffsetByModel;
    Uint8* animatedInstanceCountByModelPool;
    Uint8* animatedInstanceOffsetByModelPool;
    TTAnimationState* animationStatePool;

    TTTransform* transformPool;
    TTTransform* nextFreeTransform;
    TTAnimationSpec0* animationSpecPool;
    TTAnimationSpec0* nextFreeAnimationSpec;
    TTAnimationSetSpec0* animationSetSpecPool;
    TTAnimationSetSpec0* nextFreeAnimationSetSpec;

    Sint32 maxTransformCount;
    Sint32 maxAnimationCount;
    Sint32 maxAnimationSetCount;
    Sint32 maxAnimatedInstanceCount;
} TTAnimationPools0;

// #define TT_ANIMATION_RAM_USAGE_0(animationSetCount, animationCount, transformCount) (animationSetCount * sizeof(TTAnimationSetSpec0) + sizeof(Uint8)*3 + animationCount * sizeof(TTAnimationSpec0) + transformCount * sizeof(TTTransform))
#define TT_INSTANCE_RAM_USAGE_0() (sizeof(TTAnimationState) + sizeof(TTInstancePosition) + sizeof(TTTransform*))
#define TT_ANIMATION_RAM_USAGE_0(animationSetCount, animationCount, transformCount, instanceCount) \
    ((transformCount) * sizeof(TTTransform) + (animationCount) * sizeof(TTAnimationSpec0) \
    + (animationSetCount) * (sizeof(TTAnimationSetSpec0) + sizeof(Uint8) * 3) \
    + (instanceCount) * (sizeof(TTAnimationState) + sizeof(TTInstancePosition) + sizeof(TTTransform)))
#define TT_ANIMATION_POOL_RESERVED_RAM_USAGE_0(animationPool) \
    (TT_ANIMATION_RAM_USAGE_0((animationPool)->maxAnimationSetCount, (animationPool)->maxAnimationCount, \
        (animationPool)->maxTransformCount, (animationPool)->maxAnimatedInstanceCount))


void ttInitAnimationPoolsX0(TTAnimationPools0* pools, Uint8** nextFreeHRAMBytePtr, Uint8** nextFreeLRAMBytePtr, Uint32 maxAnimationSetCount,
                            Uint32 maxMeshCount, Uint32 maxAnimationCount, TT_RAM_TYPE animationDataRAMType,
                            Uint32 maxTransformCount, TT_RAM_TYPE transformDataRAMType,
                            Uint32 maxInstanceCount, TT_RAM_TYPE instanceDataRAMType);
void ttInitAnimationPools0(TTAnimationPools0* pools, Uint8** nextFreeHRAMBytePtr, Uint8** nextFreeLRAMBytePtr, Uint32 maxAnimationSetCount,
                            Uint32 maxMeshCount, Uint32 maxPoseCnt, Uint32 maxAnimationCount, TT_RAM_TYPE animationDataRAMType,
                            Uint32 maxTransformCount, TT_RAM_TYPE transformDataRAMType,
                            Uint32 maxInstanceCount, TT_RAM_TYPE instanceDataRAMType);
void ttResetAnimationPools0(TTAnimationPools0* pools);
TTAnimationPoolLoadStatus ttLoadAnimationInPool0(TTModelPools* modelPools, TTAnimationPools0* animationPools, char* filename);
void ttGetAnimationRAMUsage0(TTAnimationPools0* pools, Uint32* hRAMUsage, Uint32* lRAMUsage,
                                Uint32 animationSetCount, Uint32 animationCount, Uint32 transformCount, Uint32 instanceCount);
void ttGetCurrentAnimationRAMUsage0(TTModelPools* modelPools, TTAnimationPools0* animationPools, Uint32* hRAMUsage, Uint32* lRAMUsage);
Sint32 ttSetAnimatedInstanceCountInPool0(TTAnimationPools0* pools, Uint8 animatedModelCount, Uint8 instanceCounts[animatedModelCount]);
void ttStartAnimationInPool0(TTAnimationPools0* pools, Uint8 modelID, Uint8 instanceID, Uint8 animationSpecID);
void ttPauseAnimationInPool0(TTAnimationPools0* pools, Uint8 modelID, Uint8 instanceID);
void ttUnpauseAnimationInPool0(TTAnimationPools0* pools, Uint8 modelID, Uint8 instanceID);
Bool ttIsAnimationPausedInPool0(TTAnimationPools0* pools, Uint8 modelID, Uint8 instanceID);
void ttDeactivateInstanceInPool0(TTAnimationPools0* pools, Uint8 modelID, Uint8 instanceID);
Bool ttIsInstanceActiveInPool0(TTAnimationPools0* pools, Uint8 modelID, Uint8 instanceID);
void ttUpdateAnimationsInPool0(TTAnimationPools0* pools);
void ttDrawAnimatedModelsFromPool0(TTModelPools* modelPools, TTAnimationPools0* pools);

static inline __attribute__((always_inline)) void ttGetAnimationRAMUsageFromMetaData0(TTAnimationPools0* pools, Uint32* hRAMUsage, Uint32* lRAMUsage,
                                            TTMOTFileMetadata0* metaData, Uint32 instanceCount) {
    ttGetAnimationRAMUsage0(pools, hRAMUsage, lRAMUsage, 1, metaData->animationCount,
                            metaData->meshCount * metaData->totalPoseCount, instanceCount);
}

static inline __attribute__((always_inline)) void ttGetReservedAnimationRAMUsage0(TTAnimationPools0* pools, Uint32* hRAMUsage, Uint32* lRAMUsage) {
    ttGetAnimationRAMUsage0(pools, hRAMUsage, lRAMUsage, pools->maxAnimationSetCount,
                            pools->maxAnimationCount, pools->maxTransformCount, pools->maxAnimatedInstanceCount);
}

//sets the instance position. The way it works will probably change a lot.
static inline __attribute__((always_inline)) void ttSetAnimatedInstancePositionInPool0(TTAnimationPools0* pools, Uint8 modelID, Uint8 instanceID, TTInstancePosition transform) {
    pools->animatedInstanceCurrentPositionPool[pools->animatedInstanceOffsetByModelPool[modelID] + instanceID] = transform;
}

// #define TT_ALLOC_ANIMATION_POOLS_X_0(variableName, maxAnimationSetCnt, maxMeshCnt, maxAnimationCnt, animationAttribute, maxTransformCnt, transformAttribute, maxInstanceCnt, instanceAttribute) \
//     TTTransform variableName##_transformPool[maxTransformCnt] __attribute__((aligned(4))) transformAttribute; \
//     TTAnimationSpec0 variableName##_animationSpecPool[maxAnimationCnt] __attribute__((aligned(4))) animationAttribute; \
//     TTAnimationSetSpec0 variableName##_animationSetSpecPool[maxAnimationSetCnt] __attribute__((aligned(4))) animationAttribute; \
//     Uint8 variableName##_animationSpecOffsetByModelPool[maxAnimationSetCnt] __attribute__((aligned(4))) animationAttribute; \
//     Uint8 variableName##_animatedInstanceCountByModelPool[maxAnimationSetCnt] __attribute__((aligned(4))) instanceAttribute; \
//     Uint8 variableName##_animatedInstanceOffsetByModelPool[maxAnimationSetCnt] __attribute__((aligned(4))) instanceAttribute; \
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