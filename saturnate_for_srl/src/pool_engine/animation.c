#include <sl_def.h>
#include "animation.h"
#include "../core/animation.h"
#include "model_loader.h"
#include "debug.h"

TTTransform transformPool[MAX_TRANSFORM_COUNT] __attribute__((aligned(4)));
TTTransform* nextFreeTransform;
TTAnimationSpec animationSpecPool[MAX_ANIMATION_COUNT] __attribute__((aligned(4)));
TTAnimationSpec* nextFreeAnimationSpec;
TTAnimationSetSpec animationSetSpecPool[MAX_ANIMATED_MODEL_COUNT] __attribute__((aligned(4)));
TTAnimationSetSpec* nextFreeAnimationSetSpec = animationSetSpecPool;
uint8_t animationSpecOffsetByModelPool[MAX_ANIMATED_MODEL_COUNT] __attribute__((aligned(4))); //offset[modelID] = ID du premier animationSpec pour ce modèle
uint8_t* nextFreeAnimationSpecOffsetByModel = animationSpecOffsetByModelPool;
//envisager la possibilité de placer l'offset ci-dessus directement dans le animationSpecSet
//et plutôt que faire la somme d'offset du AnimationSetSpec.transformOffset et AnimationSet.transformOffset peut-être stocker la somme dans AnimationSet.transformOffset
uint8_t animatedInstanceCountByModelPool[MAX_ANIMATED_MODEL_COUNT] __attribute__((aligned(4))); // same 20 than animationSetSpecPool, the above like and the next line
uint8_t animatedInstanceOffsetByModelPool[MAX_ANIMATED_MODEL_COUNT] __attribute__((aligned(4))); // offset[modelID] = offset[modelID-1] + count[modelID-1]. offset[modelID] = ID de la première instance pour ce modèle

//il y a autant d'animationStates qu'il y a d'instances de tous les models réunis
//animationStatePool est en fait un genre de tableau à 2 dimensions : animationStatePool[animationModelCount][instanceCount (variable par model)]
TTAnimationState animationStatePool[MAX_ANIMATED_INSTANCE_COUNT] __attribute__((aligned(4)));
TTInstancePosition animatedInstanceCurrentPositionPool[MAX_ANIMATED_INSTANCE_COUNT] __attribute__((aligned(4)));
TTTransform* animatedInstanceCurrentPosePool[MAX_ANIMATED_INSTANCE_COUNT] __attribute__((aligned(4)));

void ttResetAnimationPools() {
    nextFreeTransform = transformPool;
    nextFreeAnimationSpec = animationSpecPool;
    nextFreeAnimationSetSpec = animationSetSpecPool;
    nextFreeAnimationSpecOffsetByModel = animationSpecOffsetByModelPool;
}

// requires "ttInitFile"
// animationTransforms is a 3 dimensional array : animationTransforms[animationCount][PoseCount (variable)][meshCount (not variable)]
void ttLoadAnimationInPool(char* filename) {
    ttLoadAnimation(filename, (TTMOTFileHeader*) nextFreeAnimationSetSpec, nextFreeAnimationSpec, nextFreeTransform);
    *nextFreeAnimationSpecOffsetByModel = nextFreeAnimationSpec - animationSpecPool;
    nextFreeAnimationSpecOffsetByModel++;
    nextFreeAnimationSetSpec->transformOffset = nextFreeTransform - transformPool;
    TTAnimationSpec lastAnimationSpec = *(nextFreeAnimationSpec + nextFreeAnimationSetSpec->animationCount - 1);
    nextFreeTransform += lastAnimationSpec.totalPoseCount * nextFreeAnimationSetSpec->meshCount + lastAnimationSpec.transformOffset;
    nextFreeAnimationSpec += nextFreeAnimationSetSpec->animationCount;
    nextFreeAnimationSetSpec++;
}

//sets the instance position. The way it works will probably change a lot.
void ttSetAnimatedInstancePositionInPool(uint8_t modelID, uint8_t instanceID, TTInstancePosition transform) {
    animatedInstanceCurrentPositionPool[animatedInstanceOffsetByModelPool[modelID] + instanceID] = transform;
}

//L'idée est de décider du nombre max d'instances d'un model animé avant le lancement d'un nivau/section or whatever
void ttSetAnimatedInstanceCountInPool(uint8_t animatedModelCount, uint8_t instanceCounts[animatedModelCount]) {
    if(animatedModelCount == 0)
        return; //TBD Genre, affiche un warning is mode debug idk

    uint8_t* instanceCountPtr = animatedInstanceCountByModelPool;
    uint8_t* instanceOffsetPtr = animatedInstanceOffsetByModelPool;

    uint32_t accumulatedOffset = 0;
    uint32_t i = (uint32_t) animatedModelCount;
    do {
        uint32_t currentCount = (uint32_t) *instanceCounts++;
        *instanceOffsetPtr++ = (uint8_t) accumulatedOffset;
        *instanceCountPtr++ = (uint8_t) currentCount;
        accumulatedOffset += currentCount;
    }
    while(--i > 0);

    TTAnimationState* animationStatePtr = animationStatePool;
    TTTransform** currentPosePtr = animatedInstanceCurrentPosePool;
    for(i = MAX_ANIMATED_INSTANCE_COUNT; i > 0; i--, animationStatePtr++, currentPosePtr++) {
        animationStatePtr->currentPoseID = -1; // -1 means inactive, do not treat
        currentPosePtr = NULL;
    }
}

void ttStartAnimationInPool(uint8_t modelID, uint8_t instanceID, uint8_t animationSpecID) {
    uint32_t specOffset = animationSpecOffsetByModelPool[modelID];
    uint32_t instanceOffset = animatedInstanceOffsetByModelPool[modelID];
    TTAnimationSpec* animationSpec = animationSpecPool + specOffset + animationSpecID;
    TTAnimationState* animationState = animationStatePool + instanceOffset + instanceID;
    animationState->frameCountBeforeNextPose = animationState->totalFrameCountBetweenPoses = animationSpec->totalFrameCountBetweenPoses;
    animationState->totalPoseCount = animationSpec->totalPoseCount;
    animationState->currentPoseID = animationSpec->totalPoseCount - 1;

    TTAnimationSetSpec animationSetSpec = animationSetSpecPool[modelID];
    animatedInstanceCurrentPosePool[animatedInstanceOffsetByModelPool[modelID] + instanceID] = transformPool + animationSetSpec.transformOffset +  animationSpec->transformOffset + animationState->currentPoseID * animationSetSpec.meshCount;
}

//Code heavily based on core/animation.c/ttUpdateAnimation but rewritten here to optimize it better
void ttUpdateAnimationsInPool() {
    TTAnimationSetSpec* animationSetSpec = animationSetSpecPool;
    const uint8_t* restrict instanceCount = animatedInstanceCountByModelPool;
    TTAnimationState* animationStatePtr = animationStatePool;
    TTTransform** currentPosePtr = animatedInstanceCurrentPosePool;
    for(int i = nextFreeAnimationSetSpec - animationSetSpecPool; i > 0; i--, animationSetSpec++, instanceCount++) {
        const uint32_t meshCount = (uint32_t) animationSetSpec->meshCount;
        for(int j = *instanceCount; j > 0; j--, animationStatePtr++, currentPosePtr++) {
            if(animationStatePtr->currentPoseID != -1 && --(animationStatePtr->frameCountBeforeNextPose) < 0) {
                animationStatePtr->frameCountBeforeNextPose = animationStatePtr->totalFrameCountBetweenPoses;
                if(--(animationStatePtr->currentPoseID) < 0) {
                    animationStatePtr->currentPoseID = animationStatePtr->totalPoseCount - 1;
                    (*currentPosePtr) += (animationStatePtr->totalPoseCount-1)*meshCount;
                }
                else
                    (*currentPosePtr)-=meshCount;
            }
        }
    }
}

void ttDrawAnimatedModelsFromPool() {
    const TTModelSpec* restrict modelSpec = modelSpecPool;
    const TTAnimationSetSpec* restrict animationSetSpec = animationSetSpecPool;
    const uint8_t* restrict instanceCount = animatedInstanceCountByModelPool;
    TTInstancePosition* restrict transform = animatedInstanceCurrentPositionPool;
    TTTransform** restrict currentPose = animatedInstanceCurrentPosePool;
    // je suppose que le tableau de models et d'animationSet corespondent (même taille, même entity au même indice), forcer ça by design
    for(int i = nextFreeModelSpec - modelSpecPool; i > 0; i--, modelSpec++, animationSetSpec++, instanceCount++) {
        const uint32_t localMeshCount = (uint32_t) animationSetSpec->meshCount;
        for(int j = *instanceCount; j > 0; j--, transform++, currentPose++) {
            if(*currentPose != NULL) {
                slPushMatrix();
                slTranslate(transform->xOffset, transform->yOffset, transform->zOffset);
                slRotX(transform->xAngle);
                slRotY(transform->yAngle);
                slRotZ(transform->zAngle);
                ttDrawAnimatedModelAtPose(meshPool + modelSpec->meshID, *currentPose, localMeshCount);
                slPopMatrix();
            }
        }
    }
}
