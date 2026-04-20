#ifndef TT_POOL_ANIMATION_H
#define TT_POOL_ANIMATION_H

#include "jo/sgl_prototypes.h"
#include "../core/animation.h"

#define MAX_ANIMATED_MODEL_COUNT 20
#define MAX_ANIMATION_COUNT 100
#define MAX_TRANSFORM_COUNT 25000
#define MAX_ANIMATED_INSTANCE_COUNT 30

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
} TTAnimationSetSpec;

extern TTTransform transformPool[MAX_TRANSFORM_COUNT] __attribute__((aligned(4)));
extern TTTransform* nextFreeTransform;
extern TTAnimationSpec animationSpecPool[MAX_ANIMATION_COUNT] __attribute__((aligned(4)));
extern TTAnimationSpec* nextFreeAnimationSpec;
extern TTAnimationSetSpec animationSetSpecPool[MAX_ANIMATED_MODEL_COUNT] __attribute__((aligned(4)));
extern TTAnimationSetSpec* nextFreeAnimationSetSpec;
extern Uint8 animatedInstanceCountByModelPool[MAX_ANIMATED_MODEL_COUNT] __attribute__((aligned(4))); // same 20 than animationSetSpecPool and the next line
extern Uint8 animatedInstanceOffsetByModelPool[MAX_ANIMATED_MODEL_COUNT] __attribute__((aligned(4))); // offset[modelID] = offset[modelID-1] + count[modelID-1]

//il y a autant d'animationStates qu'il y a d'instances de tous les models réunis
//animationStatePool est en fait un genre de tableau à 2 dimensions : animationStatePool[animationModelCount][instanceCount (variable par model)]
extern TTAnimationState animationStatePool[MAX_ANIMATED_INSTANCE_COUNT] __attribute__((aligned(4)));
extern TTInstancePosition animatedInstanceCurrentPositionPool[MAX_ANIMATED_INSTANCE_COUNT] __attribute__((aligned(4)));

void ttResetAnimationPools();
void ttLoadAnimationInPool(char* filename);
void ttSetAnimatedInstancePositionInPool(Uint8 modelID, Uint8 instanceID, TTInstancePosition transform);
void ttSetAnimatedInstanceCountInPool(Uint8 animatedModelCount, Uint8 instanceCounts[animatedModelCount]);
void ttStartAnimationInPool(Uint8 modelID, Uint8 instanceID, Uint8 animationSpecID);
void ttUpdateAnimationsInPool();
void ttDrawAnimatedModelsFromPool();

#endif //TT_POOL_ANIMATION_H
