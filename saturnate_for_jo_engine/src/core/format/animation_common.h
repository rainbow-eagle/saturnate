#ifndef TT_CORE_ANIMATION_COMMON_H
#define TT_CORE_ANIMATION_COMMON_H

#include <jo/sgl_prototypes.h>

/** 
 * @brief Structure representing a coordinate transformations (position shift and then rotation).
 * Not used by the animation system per se. Used to place the instance in position in the 3D world before animating it.
 * TTTransform is the similar structure used by the animation system.
 * 
 * @see TTTransform
 * @ingroup MOT0 MOT1
 */
typedef struct {
    FIXED xOffset, yOffset, zOffset; ///< x, y and z offset to be applied during a position shift (slTranslate()).
    ANGLE xAngle, yAngle, zAngle; ///< x, y and z angle to be applied during rotations (calling in order slRotX, slRotY, slRotZ).
    Uint16 reservedSpace;
} TTInstancePosition;

/**
 * @brief Structure representing the header of a MOT file.
 * The data represents a set of animations for a model.
 * 
 * @warning Do not edit manually.
 * @ingroup MOT0 MOT1
 */
typedef struct {
    Uint8 reservedSpace1; ///< Should be a file format descriptor to make sure you're reading the right type of file
    Uint8 reservedSpace2; ///< Should become a version number to allow retrocompatibility in case of evolution
    Uint8 animationCount; ///< Number of animations in this set.
    Uint8 meshCount; ///< Number of meshes composing the animated model
} TTMOTFileHeader;

/**
 * @brief Structure representing the state of an instance of an animation being run.
 * Is specific to an instance, not shared.
 * (Well one could want to share it among several instances to animate, say,
 * Sonic's rings all rotating the same way at the same time).
 * @ingroup MOT0 MOT1
 */
typedef struct {
    Sint8 totalFrameCountBetweenPoses; ///< (Copy of corresponding animationSpec data) Number of frames a pose remains in place before switching to the next pose in the animation.
    Sint8 totalPoseCount; ///< (Copy of corresponding animationSpec data) Total number of poses that compose this animation.
    Sint8 currentPoseID; ///< Index of the pose currently assumed by the model (between animationSpec totalPoseCount-1 and zero).
    Sint8 frameCountBeforeNextPose; ///< Remaining number of frames to wait before switching to the next pose.
} TTAnimationState;

/**
 * @brief Return codes for animation loading operations.
 * SGL's OK value (zero) means success.
 * A negative value indicates a critical error preventing the use of the animation.
 */
typedef enum {
    TT_ANIMATION_OVERFLOW = -1,
    TT_BONE_OVERFLOW = -2,
    TT_POSE_OVERFLOW = -3,
    TT_TRANSFORM_OVERFLOW = -4
} TTAnimationLoadStatus;

/**
 * @brief Updates the state of an instance of an animation.
 * Must be called at every frame to maintain the state up to date.
 * 
 * @param[in] animationState The animation instance to be kept updated.
 */
static inline __attribute__((always_inline)) void ttUpdateAnimation(TTAnimationState* restrict animationState) {
    if(__builtin_expect(animationState->frameCountBeforeNextPose == -1, 0))// means inactive
        return;
    if(__builtin_expect(--animationState->frameCountBeforeNextPose < 0, 1)) {
        animationState->frameCountBeforeNextPose = animationState->totalFrameCountBetweenPoses;
        if(__builtin_expect(--animationState->currentPoseID < 0, 0))
            animationState->currentPoseID = animationState->totalPoseCount - 1;
    }
}

#endif //TT_CORE_ANIMATION_COMMON_H