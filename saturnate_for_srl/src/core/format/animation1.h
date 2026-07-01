#ifndef TT_CORE_ANIMATION1_H
#define TT_CORE_ANIMATION1_H

#include <sl_def.h>
#include "animation_common.h"

/**
 * @defgroup MOT1
 * @brief MOT1 version of the animation libraries.
 * 
 * Supports rotating parts of a model at every frame. Does not support scaling. Each bone has a fixed sized for a given model
 * with the exception of the root bone which can change at every frame. The root bone is not attached to an actual
 * mesh, it is an invisible bone connecting the origin of the model to the first mesh. Let's explain why with an example.
 * Say you have a human character animated with this engine, a fixed size root bone would imply that the root mesh,
 * i.e. typically the hip, cannot move and therefore the character cannot crouch. Changing the size of the root bone would allow
 * the character to crouch, slightly bob up and down as he walks etc.
 * 
 * Allows the user to load animations from files and use the animation data in conjunction with
 * meshes to animate them. At present the idea is to comibine a .NYA file and a .MOT file to animate a model.
 * An animation is linked to a model. A model is made of one or more meshes.
 * A set of animations that corresponds to a same given model are called an "animation set".
 * An animation is a series of poses that are applied in order to the corresponding model to animate it.
 * An animation can change the model's pose at every frame, or change it every 2, 3 or N frames,
 * reducing the fuidity of the animation as N increases.
 * A pose is a series of rotations that are applied to the meshes of the model.
 * The position of meshes relative to one another is described by its armature, that is, a set of bones (this matches
 * Blender's vocabulary), characterized by  their hierarchy and the position of their origin relative to the position of the origin
 * of its parent bone; that is, an armature cannot change for a given animation set, with one exception. Indeed the first mesh/bone
 * can change position relatively to the global origin of the model. For example, having a human character with its origin between
 * its feet and the first mesh being its hip, this allow the hip to move up and down (crouching, simpling "bobbing" when walking etc.).
 * 
 * As a result of all of this, an animation set loaded from a file conceptually is a 3 dimensional array of Transformations
 * TTTransform animations[animationCount][poseCount][meshCount]
 * where meshCount is the same accross all the animations but poseCount depends on each animation.
 * animations[N] is the Nth animation of the set.
 * animation[N][M] is the Mth pose of the Nth animation of the set.
 * 
 * An animation set is described by a TTMOTFileHeader (static data).
 * An animation is described by a TTAnimationSpec1 (static data), an array of: TTBone, TTRootBonePosition, TTBoneRotation (static data).
 * An animation can be instantiated, that is, a model can be animated according to the specification of that animation.
 * An instance of an animation will be described by a TTAnimationState (dynamic data)
 * referencing its model animation (TTAnimationSpec1, TTBone, TTRootBonePosition and TTBoneRotation).
 * 
 * @see ModelLoader
 */

/**
 * @addtogroup MOT1
 * @{
 */

/**
 * @brief Structure representing a bone, part of an armature, for an animation.
 * 
 * Bones do not change in size nor in position within the armature; they can only rotate around their origin.
 */
typedef struct {
    FIXED xOffset, yOffset, zOffset; ///< x, y and z offset to be applied during a position shift from one bone's origin to the next (slTranslate()).
    uint8_t popCount; ///< Number of matrix pops to be applied after applying the animation transformations for this bone. Used to characterize the hierarchy within the armature. 0 to not pop anything.
    uint8_t reservedSpace1;
    uint16_t reservedSpace2;
} TTBone;// I could replace this bone representation with 3 ANGLES, a bone length and popcount = 3 * 32bits instead of 4 * 32bits but would imply calculating more rotations

/**
 * @brief Structure representing the rotation to be applied to a bone.
 * 
 * An array of meshCount TTBoneRotation, combined with an array of meshCount TTBone describes the operations to be applied to
 * reach a pose in an animation. A 30 frames animation therefore requires 30*meshCount TTBoneRotation.
 */
typedef struct {
    ANGLE xAngle, yAngle, zAngle; ///< x, y and z angle to be applied during rotations (calling in order slRotX, slRotY, slRotZ).
    uint16_t reservedSpace;
} TTBoneRotation;

/**
 * @brief The change in position of the root bone of an armature relatively to its world origin.
 * 
 * Allows an armature to have its parent bone move during an animation. Had the parent bone an immuable position
 * (like the other bones), a character wouldn't be able to crouch for example.
 */
typedef struct {
    FIXED xOffset, yOffset, zOffset;  ///< x, y and z offset to be applied to place a root mesh relatively to its origin (slTranslate()).
} TTRootBonePosition;

/**
 * @brief Structure representing an animation through its specifications (static data).
 * Can be shared by several instances running the described animation.
 * 
 * @see TTAnimationState to represent an animation being ran (dynamic data).
 */
typedef struct {
    int8_t totalFrameCountBetweenPoses; ///< Number of frames a pose remains in place before switching to the next pose in the animation.
    int8_t totalPoseCount; ///< Total number of poses that compose this animation.
    uint16_t poseOffset; ///< Index (in the list of poses of this animationSet, not globally) of the first pose that describes this animation, i.e. sum of all the totalPoseCount that precede it it.
} TTAnimationSpec1;

/**
 * @brief Structure assembling all the required information for the engine to understand how to draw a pose for a model.
 */
typedef struct {
    TTBone* armature; ///< [in] The armature describing the bone hierarcy of the model. Array of size meshCount.
    TTBoneRotation* currentPose; ///< [in] The current pose to be applied. Array of size meshCount
    TTRootBonePosition* currentRootBonePosition; ///< [in] The position of the root bone relatively to the origin.
    uint32_t meshCount; ///< [in] The number of meshes in the model (which is also the number of bones in the armature and boneRotations in the pose)
} TTPoseDescription1;

 /**
 * @brief Container describing where to load an animation set.
 * 
 * An animation set is described by metadata for the set, metadata for each animation, and the animation data itself.
 * Data is to be loaded to the pointers the user provides, respecting the containers' size thank to the provided "maxCount" parameters.
 */
typedef struct {
    TTMOTFileHeader* animationSetSpecPtr; ///< [out] Pointer to the memory space where a TTMOTFileHeader is loaded.
    TTAnimationSpec1* animationSpecs; ///< [out] Array where the animationSpecs are loaded.
    TTBone* armature; ///< [out] Array where the bones describing the armature are loaded.
    TTRootBonePosition* rootBonePositionByPose; ///< [out] Array where the rootBonePositions are loaded.
    TTBoneRotation* animations; ///< [out] Array where the animations' TTBoneRotation are loaded.
    uint32_t maxAnimationSpecCount; ///< [in] Maximum amount of TTAnimationSpec1 that can be added to animationSpecs.
    uint32_t maxBoneCount; ///< [in] Maximum amount of TTBone that can be added to armature.
    uint32_t maxRootBonePositionByPoseCount; ///< [in] Maximum amount of TTRootBonePosition that can be added to rootBonePositionByPose.
    uint32_t maxTransformCount; ///< [in] Maximum amount of TTBoneRotation that can be added to animations.
} TTAnimationBuffer1;

/**
 * @brief Metadata describing an animation from a .MOT file.
 */
typedef struct {
    uint16_t totalPoseCount; ///< Total number of poses that compose this animation.
    uint8_t animationCount; ///< Total number of animations that compose this animation.
    uint8_t meshCount; ///< Total number of meshes or bones that compose the model to be animated by this animation.
} TTMOTFileMetadata1;

/**
 * @brief Loads the metadata of the animations described in a .MOT file into the given struct.
 * 
 * @note Requires GFS_Init() to be called prior to use.
 * @note This function is almost as slow as loading the file with ttLoadMOTMetadata1(). A workflow based on checking the size
 * and then opening the file (this function and then ttLoadMOTMetaData1()) should be avoided to optimize loading speed.
 * 
 * @param[in] filename The name of the file to be opened in the current directory.
 * @param[out] metadata The structure where the metadata are to be loaded.
 *
 * @see GFS_Init()
 */
void ttLoadMOTMetadata1(char* fileName, TTMOTFileMetadata1* metaData);

/**
 * @brief Loads an animation set from the given file into the given user-allocated arrays.
 *
 * @note Requires GFS_Init() to be called prior to use.
 * 
 * @param[in] filename The name of the file to be opened in the current directory.
 * @param[in,out] animationBuffer structure providing the pointers to where the data will be loaded.
 *
 * @warning Output arrays must be pre-allocated with sufficient capacity to hold 
 * the entire file content to avoid returning an error code.
 * @see GFS_Init()
 */
TTAnimationLoadStatus ttLoadMOTAnimation1(char* fileName, TTAnimationBuffer1* animationBuffer);

/**
 * @brief Starts an animation and stores the new animation instance state in the given animationState.
 * 
 * @param[in] animationSpec The animation to be started.
 * @param[out] animationState The state of the animation instance to be started.
 */
static inline __attribute__((always_inline)) void ttStartAnimation1(TTAnimationSpec1* animationSpec, TTAnimationState* animationState) {
    animationState->frameCountBeforeNextPose = animationState->totalFrameCountBetweenPoses = animationSpec->totalFrameCountBetweenPoses;
    animationState->totalPoseCount = animationSpec->totalPoseCount;
    animationState->currentPoseID = animationSpec->totalPoseCount - 1;
}

static inline __attribute__((always_inline)) void ttUpdateAnimation1(TTAnimationState* restrict animationState) {
    ttUpdateAnimation(animationState);
}

/**
 * @brief Draws the given animated model.
 * The model is drawn according to the given pose.
 * The arrays model and currentPoseID share the same size.
 * Transformations described in currentPose[i] are applied to model[i].
 * The hieararchy within the meshes composing the 3D model is simulated through each TTTransform's popCount.
 * 
 * @param[in] model The meshes composing the model to be animated.
 * @param[in] armature The bones describing the armature of the model to be animated. The array must be as long as the model has meshes.
 * @param[in] currentPose The current pose in the animation.
 * @param[in] currentRootBonePosition The current position in the animation of the root bone/mesh.
 * @param[in] meshCount The number of meshes in the model (also is the number of bones in the armature and the transformations in the pose).
 * @see ttDrawAnimatedModelAtState()
 */
static inline __attribute__((always_inline)) void ttDrawAnimatedModelAtPose1(PDATA model[], TTBone* armature, TTBoneRotation* currentPose, TTRootBonePosition* currentRootBonePosition, int meshCount) {
    //Techniquement, je pourrais gagner de l'espace en mémoire parce que j'enregistre armature[0].x,y,zoffset pour rien.
    //(En effet, la position du root du modèle est mobile pour qu'il puisse s'accroupir etc. i.e. ce n'est pas un os de taille fixe)
    //Mais l'augmentation de la complexité du code pour gérer ça différemment (armature commence à l'os 1 plutôt qu'à l'os 0
    //parce qu l'os 0 est de taille variable) prendrait plus d'espace en mémoire que simplement procéder
    //comme implémenté ici, en écrasant armature[0] (valeur fixe qui n'a pas de sens) avec la valeur variable.
    //En plus, le code d'animation risquerait de ne plus tenir dans le cache dans son entièreté et donc de sérieusement ralentir.

    *((TTRootBonePosition*)armature) = *currentRootBonePosition;
    for(int i = meshCount; i > 0; i--, armature++, currentPose++, model++) {
        slPushMatrix();
        slTranslate(armature->xOffset, armature->yOffset, armature->zOffset);
        slRotX(currentPose->xAngle);
        slRotY(currentPose->yAngle);
        slRotZ(currentPose->zAngle);
        slPutPolygon(model);

        for(int p = armature->popCount; p > 0; p--)
            slPopMatrix();
    }
}

/**
 * @brief Draws the given animated model.
 * The model is drawn according to the given pose.
 * The arrays model and poseDescritption.currentPoseID share the same size.
 * Transformations described in poseDescription->currentPose[i] are applied to model[i].
 * The hieararchy within the meshes composing the 3D model is simulated through each TTTransform's popCount.
 * 
 * @param[in] model The meshes composing the model to be animated.
 * @param[in] poseDescription The description of the current pose to be applied to the model.
 * @see ttDrawAnimatedModelAtPose1()
 */
static inline __attribute__((always_inline)) void ttDrawAnimatedModel1(PDATA model[], TTPoseDescription1* restrict poseDescription) {
    ttDrawAnimatedModelAtPose1(model, poseDescription->armature, poseDescription->currentPose, poseDescription->currentRootBonePosition, poseDescription->meshCount);
}

/**
 * @brief Draws the given animated model.
 * The model is drawn according in the pose determined by the given animation and animation state.
 * The hieararchy whithin the meshes composing the 3D model is simulated through each TTTransform's popCount.
 * 
 * @param[in] model The meshes composing the model to be animated.
 * @param[in] armature The bones describing the armature of the model to be animated.
 * @param[in] animation The animation (that is, the set of poses) this model is currently playing.
 * @param[in] animationRootBonePosition The positions of the root bone/mesh that this model is currently playing.
 * @param[in] meshCount The number of meshes in the model (also is the number of transformations in the pose).
 * @param[in] animationState The current state of the animation.
 */
static inline __attribute__((always_inline)) void ttDrawAnimatedModelAtState1(PDATA* model, TTBone* armature, TTBoneRotation* animation, TTRootBonePosition* animationRootBonePositions, uint32_t meshCount, TTAnimationState* restrict animationState) {
    TTBoneRotation* currentPose = animation + animationState->currentPoseID * meshCount;
    TTRootBonePosition* currentRootBonePosition = animationRootBonePositions + animationState->currentPoseID;
    ttDrawAnimatedModelAtPose1(model, armature, currentPose, currentRootBonePosition, meshCount);
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
static inline __attribute__((always_inline)) void ttDrawAnimatedModelFromSpecs1(PDATA model[], TTBone* armature, TTBoneRotation* animations, TTRootBonePosition* rootBonePositions, uint32_t meshCount, TTAnimationSpec1* restrict animationSpec, TTAnimationState* animationState) {
    TTRootBonePosition* currentAnimationRootBonePositions = rootBonePositions + animationSpec->poseOffset;
    TTBoneRotation* currentAnimation = animations + animationSpec->poseOffset * meshCount;
    ttDrawAnimatedModelAtState1(model, armature, currentAnimation, currentAnimationRootBonePositions, meshCount, animationState);
}

/**
 * @} //closes group MOT1
 */

#endif // TT_CORE_ANIMATION1_H