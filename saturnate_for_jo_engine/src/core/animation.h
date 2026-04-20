#ifndef TT_CORE_ANIMATION_H
#define TT_CORE_ANIMATION_H

#include "jo/sgl_prototypes.h"

/**
 * @defgroup Animation Animation
 * @brief Utilities to animate models.
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
 * An animation is described by a TTAnimationSpec (static data) and an array of TTTransform (static data).
 * An animation can be instantiated, that is, a model can be animated according to the specification of that animation.
 * An instance of an animation will be described by a TTAnimationState (dynamic data)
 * referencing its model animation (TTAnimationSpec and TTTransform[]).
 * 
 * @see ModelLoader
 */

/**
 * @addtogroup Animation
 * @{
 */

/** @brief Structure representing a coordinate transformations (position shift and then rotation).
 * Not used by the animation system per se. Used to place the instance in position in the 3D world before animating it.
 * TTTransform is the similar structure used by the animation system.
 * 
 * @see TTTransform
 */
typedef struct {
    FIXED xOffset, yOffset, zOffset; /** x, y and z offset to be applied during a position shift (slTranslate()). */
    ANGLE xAngle, yAngle, zAngle; /** x, y and z angle to be applied during rotations (calling in order slRotX, slRotY, slRotZ). */
    Uint16 reservedSpace;
} TTInstancePosition;

/**
 * @brief Structure representing a coordinate transformation and matrix stack operation.
 * Used in series by the animation system to represent a hierarchical animation.
 * The coordinate transformation is applied to mesh before chosing to
 * stack or pop matrices depending on the position of the next mesh in the hierarchy.
 */
typedef struct {
    FIXED xOffset, yOffset, zOffset; /** x, y and z offset to be applied during a position shift (slTranslate()). */
    ANGLE xAngle, yAngle, zAngle; /** x, y and z angle to be applied during rotations (calling in order slRotX, slRotY, slRotZ). */
    Uint16 popCount; /** Number of pops to be applied after applying the transformations. 0 to not pop anything. */
} TTTransform;

/**
 * @brief Same as @TTTransform but directly providing sin/cos values instead of angles.
 * Good for execution speed, bad for memory space.
 * 
 * @see TTTransform
 */
typedef struct {
    FIXED xOffset, yOffset, zOffset; /** x, y and z offset to be applied during a position shift (slTranslate()). */
    FIXED xSin, xCos; /** Angle to be applied during rotation along the X axis. */
    FIXED ySin, yCos; /** Angle to be applied during rotation along the Y axis. */
    FIXED zSin, zCos; /** Angle to be applied during rotation along the Z axis. */
    Uint16 popCount; /** Number of pops to be applied after applying the transformations. 0 to not pop anything. */
    Uint16 reservedSpace;
} TTTransformSC;

/**
 * @brief Structure representing the header of a MOT file.
 *
 * The data represents a set of animations for a model.
 * 
 * @warning Do not edit manually.
 */
typedef struct {
    Uint8 reservedSpace1; /** Should be a file format descriptor to make sure you're reading the right type of file */
    Uint8 reservedSpace2; /** Should become a version number to allow retrocompatibility in case of evolution */
    Uint8 animationCount; /** Number of animations in this set. */
    Uint8 meshCount; /** Number of meshes composing the animated model */
} TTMOTFileHeader;

/**
 * @brief Structure representing an animation through its specifications (static data).
 * Can be shared by several instances running the described animation.
 * 
 * @see TTAnimationState to represent an animation being ran (dynamic data).
 */
typedef struct {
    Sint8 totalFrameCountBetweenPoses; /** Number of frames a pose remains in place before switching to the next pose in the animation. */
    Sint8 totalPoseCount; /** Total number of poses that compose this animation. */
    Uint16 transformOffset; /** Index (in the list of transforms of this animationSet, not globally) of the first TTTransform that describes this animation. */
} TTAnimationSpec;

/**
 * @brief Structure representing the state of an instance of an animation being run.
 * Is specific to an instance, not shared.
 * (Well I guess you could want to share it among several instances to animate, say,
 * Sonic's rings all rotating the same way at the same time...).
 */
typedef struct {
    Sint8 totalFrameCountBetweenPoses; /** (Copy of corresponding animationSpec data) Number of frames a pose remains in place before switching to the next pose in the animation. */
    Sint8 totalPoseCount; /** (Copy of corresponding animationSpec data) Total number of poses that compose this animation. */
    Sint8 currentPoseID; /** Index of the pose currently assumed by the model (between animationSpec totalPoseCount-1 and zero). */
    Sint8 frameCountBeforeNextPose; /** Remaining number of frames to wait before switching to the next pose. */
} TTAnimationState;

/**
 * @brief Loads an animation set from the given file into the given user-allocated arrays.
 *
 * @note Requires GFS_Init() to be called prior to use.
 * 
 * @param[in] filename The name of the file to be opened in the current directory.
 * @param[out] animationSetSpec Pointer to the memory space where an animationSetSpec is loaded.
 * @param[out] animationSpecs Array where the animationSpecs are loaded.
 * @param[out] animations Array where the animations are loaded.
 * 
 * @see GFS_Init()
 */
void ttLoadAnimation(char* fileName, TTMOTFileHeader* animationSetSpec, TTAnimationSpec* animationSpecs, TTTransform* animations);
// void loadAnimationSC1(char* fileName, TTAnimationSpec* animationSpecs, Uint32 meshCount, TTTransformSC animationTransforms[][meshCount]);

/**
 * @brief Starts an animation and stores the new animation instance state in the given animationState.
 * 
 * @param[in] animationSpec The animation to be started.
 * @param[out] animationState The state of the animation instance to be started.
 */
static inline __attribute__((always_inline)) void ttStartAnimation(TTAnimationSpec* animationSpec, TTAnimationState* animationState) {
    animationState->frameCountBeforeNextPose = animationState->totalFrameCountBetweenPoses = animationSpec->totalFrameCountBetweenPoses;
    animationState->totalPoseCount = animationSpec->totalPoseCount;
    animationState->currentPoseID = animationSpec->totalPoseCount - 1;
}

/** @brief Updates the state of an instance of an animation.
 * Must be called at every frame to maintain the state up to date.
 * 
 * @param[in] animationState The animation instance to be kept updated.
 */
static inline __attribute__((always_inline)) void ttUpdateAnimation(TTAnimationState* restrict animationState) {
    if(animationState->currentPoseID == -1)// means inactive
        return;
    if(--animationState->frameCountBeforeNextPose < 0) {
        animationState->frameCountBeforeNextPose = animationState->totalFrameCountBetweenPoses;
        if(--animationState->currentPoseID < 0)
            animationState->currentPoseID = animationState->totalPoseCount - 1;
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
 * @param[in] currentPose The current pose in the animation.
 * @param[in] meshCount The number of meshes in the model (also is the number of transformations in the pose).
 * @see ttDrawAnimatedModelAtState()
 */
static inline __attribute__((always_inline)) void ttDrawAnimatedModelAtPose(PDATA model[], TTTransform* currentPose, int meshCount) {
    for(int i = 0; i < meshCount; i++, currentPose++, model++) {
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
 * The model is drawn according in the pose determined by the given animation and animation state.
 * The hieararchy whithin the meshes composing the 3D model is simulated through each TTTransform's popCount.
 * 
 * @param[in] model The meshes composing the model to be animated.
 * @param[in] animation The animation (that is, the set of poses) this model is currently playing.
 * @param[in] meshCount The number of meshes in the model (also is the number of transformations in the pose).
 * @param[in] animationState The current state of the animation.
 */
static inline __attribute__((always_inline)) void ttDrawAnimatedModelAtState(PDATA model[], TTTransform* animation, Uint32 meshCount, TTAnimationState* animationState) {
    TTTransform* currentPose = animation + animationState->currentPoseID * meshCount;
    ttDrawAnimatedModelAtPose(model, currentPose, meshCount);
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
static inline __attribute__((always_inline)) void ttDrawAnimatedModelFromSpecs(PDATA model[], TTTransform* animations, Uint32 meshCount, TTAnimationSpec* animationSpec, TTAnimationState* animationState) {
    TTTransform* currentAnimation = animations + animationSpec->transformOffset;
    ttDrawAnimatedModelAtState(model, currentAnimation, meshCount, animationState);
}

/**
 * @brief Draws the given animated model with sin and cos instead of angles.
 * Like ttDrawAnimatedModelAtState() but using a faster but bulkier data structure.
 * 
 * @param[in] model The meshes composing the model to be animated.
 * @param[in] currentPose The current pose in the animation.
 * @param[in] meshCount The number of meshes in the model (also is the number of transformations in the pose).
 * @see ttDrawAnimatedModelAtState()
 */
static inline __attribute__((always_inline)) void ttDrawAnimatedModelSC(PDATA model[], TTTransformSC* currentPose, int meshCount) {
    for(int i = 0; i < meshCount; i++, currentPose++, model++) {
        slPushMatrix();
        slTranslate(currentPose->xOffset, currentPose->yOffset, currentPose->zOffset);
        slRotXSC(currentPose->xSin, currentPose->xCos);
        slRotYSC(currentPose->ySin, currentPose->yCos);
        slRotZSC(currentPose->zSin, currentPose->zCos);
        slPutPolygon(model);

        for(int p = currentPose->popCount; p > 0; p--)
            slPopMatrix();
    }
}

/** @} */

#endif // TT_CORE_ANIMATION_H
