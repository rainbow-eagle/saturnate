#ifndef TT_CORE_ANIMATION0_H
#define TT_CORE_ANIMATION0_H

#include <jo/sgl_prototypes.h>
#include "animation_common.h"

/**
 * @defgroup MOT0
 * @brief MOT0 version of the animation libraries.
 * 
 * Supports rotating and moving parts of a model at every frame. Does not support scaling.
 * 
 * Allows the user to load animations from files and use the animation data in conjunction with
 * meshes to animate them. At present the idea is to comibine a .NYA file and a .MOT file to animate a model.
 * An animation is linked to a model. A model is made of one or more meshes.
 * A set of animations that corresponds to a same given model are called an "animation set".
 * An animation is a series of poses that are applied in order to the corresponding model to animate it.
 * An animation can change the model's pose at every frame, or change it every 2, 3 or N frames,
 * reducing the fuidity of the animation as N increases.
 * A pose is a series of transformations (position shifts and rotations) that are applied to the meshes of the model.
 * 
 * As a result of all of this, an animation set loaded from a file conceptually is a 3 dimensional array of Transformations
 * TTTransform animations[animationCount][poseCount][meshCount]
 * where meshCount is the same accross all the animations but poseCount depends on each animation.
 * animations[N] is the Nth animation of the set.
 * animation[N][M] is the Mth pose of the Nth animation of the set.
 * 
 * An animation set is described by a TTMOTFileHeader (static data).
 * An animation is described by a TTAnimationSpec0 (static data) and an array of TTTransform (static data).
 * An animation can be instantiated, that is, a model can be animated according to the specification of that animation.
 * An instance of an animation will be described by a TTAnimationState (dynamic data)
 * referencing its model animation (TTAnimationSpec0 and TTTransform[]).
 * 
 * @see ModelLoader
 */

/**
 * @addtogroup MOT0
 * @{
 */

/**
 * @brief Structure representing a coordinate transformation and matrix stack operation.
 * Used in series by the animation system to represent a hierarchical animation.
 * The coordinate transformation is applied to mesh before chosing to
 * stack or pop matrices depending on the position of the next mesh in the hierarchy.
 */
typedef struct {
    FIXED xOffset, yOffset, zOffset; ///< x, y and z offset to be applied during a position shift (slTranslate()).
    ANGLE xAngle, yAngle, zAngle; ///< x, y and z angle to be applied during rotations (calling in order slRotX, slRotY, slRotZ).
    Uint16 popCount; ///< Number of pops to be applied after applying the transformations. 0 to not pop anything.
} TTTransform;

/**
 * @brief Structure representing an animation through its specifications (static data).
 * Can be shared by several instances running the described animation.
 * 
 * @see TTAnimationState to represent an animation being ran (dynamic data).
 */
typedef struct {
    Sint8 totalFrameCountBetweenPoses; ///< Number of frames a pose remains in place before switching to the next pose in the animation.
    Sint8 totalPoseCount; ///< Total number of poses that compose this animation.
    Uint16 transformOffset; ///< Index (in the list of transforms of this animationSet, not globally) of the first TTTransform that describes this animation.
} TTAnimationSpec0;

/**
 * @brief Structure assembling all the required information for the engine to understand how to draw a pose for a model.
 */
typedef struct {
    TTTransform* currentPose; ///< [in] The current pose in the animation. Array of size meshCount.
    Uint32 meshCount; ///< [in] The number of meshes in the model (also is the number of transformations in the pose).
} TTPoseDescription0;

 /**
 * @brief Container describing where to load an animation set.
 * 
 * An animation set is described by metadata for the set, metadata for each animation, and the animation data itself.
 * Data is to be loaded to the pointers the user provides, respecting the containers' size thank to the provided "maxCount" parameters.
 */
typedef struct {
    TTMOTFileHeader* animationSetSpecPtr; ///< [out] Pointer to the memory space where a TTMOTFileHeader is loaded.
    TTAnimationSpec0* animationSpecs; ///< [out] Array where the animationSpecs are loaded.
    Uint32 maxAnimationSpecCount; ///< [in] Maximum amount of TTAnimationSpec0 that can be added to animationSpecs.
    TTTransform* animations; ///< [out] Array where the animations' TTTransform are loaded.
    Uint32 maxTransformCount; ///< [in] Maximum amount of TTTransform that can be added to animations.
} TTAnimationBuffer0;

/**
 * @brief Metadata describing an animation from a .MOT file.
 */
typedef struct {
    Uint16 totalPoseCount; ///< Total number of poses that compose this animation.
    Uint8 animationCount; ///< Total number of animations that compose this animation.
    Uint8 meshCount; ///< Total number of meshes or bones that compose the model to be animated by this animation.
} TTMOTFileMetadata0;

/**
 * @brief Loads the metadata of the animations described in a .MOT file into the given struct.
 * 
 * @note Requires GFS_Init() to be called prior to use.
 * @note This function is almost as slow as loading the file with ttLoadMOTAnimation0(). A workflow based on checking the size
 * and then opening the file (this function and then ttLoadMOTAnimation0()) should be avoided to optimize loading speed.
 * 
 * @param[in] filename The name of the file to be opened in the current directory.
 * @param[out] metadata The structure where the metadata are to be loaded.
 *
 * @see GFS_Init()
 */
void ttLoadMOTMetadata0(char* fileName, TTMOTFileMetadata0* metadata);

/**
 * @brief Loads an animation set from the given file into the given user-allocated arrays.
 *
 * @note Requires GFS_Init() to be called prior to use.
 * 
 * @param[in] filename The name of the file to be opened in the current directory.
 * @param[in,out] animationBuffer Structure providing the pointers to where the data will be loaded.
 *
 * @warning Output arrays must be pre-allocated with sufficient capacity to hold 
 * the entire file content to avoid returning an error code.
 * @see GFS_Init()
 */
TTAnimationLoadStatus ttLoadMOTAnimation0(char* fileName, TTAnimationBuffer0* animationBuffer);

/**
 * @brief Starts an animation and stores the new animation instance state in the given animationState.
 * 
 * @param[in] animationSpec The animation to be started.
 * @param[out] animationState The state of the animation instance to be started.
 */
static inline __attribute__((always_inline)) void ttStartAnimation0(TTAnimationSpec0* animationSpec, TTAnimationState* animationState) {
    animationState->frameCountBeforeNextPose = animationState->totalFrameCountBetweenPoses = animationSpec->totalFrameCountBetweenPoses;
    animationState->totalPoseCount = animationSpec->totalPoseCount;
    animationState->currentPoseID = animationSpec->totalPoseCount - 1;
}

static inline __attribute__((always_inline)) void ttUpdateAnimation0(TTAnimationState* restrict animationState) {
    ttUpdateAnimation(animationState);
}

/**
 * @brief Draws the given animated model.
 * The model is drawn according to the given pose.
 * The arrays model and currentPoseID share the same size.
 * Transformations described in currentPose[i] are applied to model[i].
 * The hieararchy whithin the meshes composing the 3D model is simulated through each TTTransform's popCount.
 * 
 * @param[in] model The meshes composing the model to be animated.
 * @param[in] currentPose The current pose in the animation.
 * @param[in] meshCount The number of meshes in the model (also is the number of transformations in the pose).
 * @see ttDrawAnimatedModelAtState()
 */
static inline __attribute__((always_inline)) void ttDrawAnimatedModelAtPose0(PDATA* model, TTTransform* currentPose, int meshCount) {
    for(int i = meshCount; i > 0; i--, currentPose++, model++) {
        slPushMatrix();
        slTranslate(currentPose->xOffset, currentPose->yOffset, currentPose->zOffset);
        slRotX(currentPose->xAngle);
        slRotY(currentPose->yAngle);
        slRotZ(currentPose->zAngle);
        slPutPolygon(model);

        for(int p = currentPose->popCount; p > 0; p--)
            slPopMatrix();
    }
}

/**
 * @brief Draws the given animated model.
 * The model is drawn according to the given pose.
 * The arrays model and currentPoseID share the same size.
 * Transformations described in currentPose[i] are applied to model[i].
 * The hieararchy whithin the meshes composing the 3D model is simulated through each TTTransform's popCount.
 * 
 * @param[in] model The meshes composing the model to be animated.
 * @param[in] poseDescription The description of the current pose to be applied to the model.
 * @see ttDrawAnimatedModelAtPose0()
 */
static inline __attribute__((always_inline)) void ttDrawAnimatedModel0(PDATA model[], TTPoseDescription0* restrict poseDescription) {
    ttDrawAnimatedModelAtPose0(model, poseDescription->currentPose, poseDescription->meshCount);
}

/**
 * @brief Draws the given animated model.
 * The model is drawn according in the pose determined by the given animation and animation state.
 * The hieararchy whithin the meshes composing the 3D model is simulated through each TTTransform's popCount.
 * 
 * @param[in] model The meshes composing the model to be animated.
 * @param[in] animation The animation (that is, the set of poses) this model is currently playing.
 * @param[in] meshCount The number of meshes in the model (also is the number of transformations in the pose).
 * @param[in] animationState The current state of the animation.
 */
static inline __attribute__((always_inline)) void ttDrawAnimatedModelAtState0(PDATA model[], TTTransform* animation, Uint32 meshCount, TTAnimationState* animationState) {
    TTTransform* currentPose = animation + animationState->currentPoseID * meshCount;
    ttDrawAnimatedModelAtPose0(model, currentPose, meshCount);
}

/**
 * @brief Draws the given animated model.
 * The animation is determined by the given set of animations and animation specifications
 * and the pose whithin that animation is determined by the given animation state.
 * The hieararchy whithin the meshes composing the 3D model is simulated through each TTTransform's popCount.
 * 
 * @param[in] model The meshes composing the model to be animated.
 * @param[in] animation The set of animations of this model.
 * @param[in] meshCount The number of meshes in the model (also is the number of transformations in the pose).
 * @param[in] animationSpec The specifications of the animation the model is currently playing.
 * @param[in] animationState The current state of the animation.
 */
static inline __attribute__((always_inline)) void ttDrawAnimatedModelFromSpecs0(PDATA model[], TTTransform* animations, Uint32 meshCount, TTAnimationSpec0* animationSpec, TTAnimationState* animationState) {
    TTTransform* currentAnimation = animations + animationSpec->transformOffset;
    ttDrawAnimatedModelAtState0(model, currentAnimation, meshCount, animationState);
}

/**
 * @} //closes group MOT0
 */

#endif // TT_CORE_ANIMATION0_H