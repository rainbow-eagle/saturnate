#ifndef TT_CORE_ANIMATION_H
#define TT_CORE_ANIMATION_H

#include <sl_def.h>

#ifndef TT_MOT
#define TT_MOT 1
#endif //TT_MOT

#define TT_GLUE(a, b) a ## b
#define TT_APPEND(a, suffix) TT_GLUE(a, suffix)
#define TT_APPEND_MOT_VERSION(a) TT_APPEND(a, TT_MOT)

#define TT_STRINGIFY(x) #x
#define TT_TO_STRING(x) TT_STRINGIFY(x)
#define TT_EVAL_PATH(path, v) TT_TO_STRING(TT_GLUE(path, v).h)
#define TT_INCLUDE_VERSION(path, version) TT_EVAL_PATH(path, version)

#include TT_INCLUDE_VERSION(format/animation, TT_MOT)

/**
 * @defgroup Animation
 * @brief Animation API abstracting version specific implementation.
 * 
 * Allows the user to load animations from files and use the animation data in conjunction with
 * meshes to animate them. The idea is to comibine a .NYA file and a .MOT file to animate a model.
 * 
 * Several animation engines are provided. Set the environment variable TT_MOT to a supported value (0 or 1)
 * to use the correspondig engine through the abstraction layer. This allows the change of engine by simply changing
 * the environment variable without having to modify the code.
 * 
 * The different engines can be used directly without the abstraction layer if prefered. In any case, one shouldn't mix the use
 * of both the abstraction layer and one or more engine in the same code.
 * 
 * @note As it is, the core/ part of the abstraction layer does not provide the necessary functions to exploit it correctly.
 * However the pool_engine/ part is complete and functional.
 * 
 * @see ModelLoader
 *
 * @addtogroup Animation
 * @{
 */

/**
 * @struct TTMOTFileMetadata
 * @brief (Abstracted type) Metadata describing an animation from a .MOT file.
 */
#define TTMOTFileMetadata TT_APPEND_MOT_VERSION(TTMOTFileMetadata)

/**
 * @struct TTAnimationSpec
 * @brief (Abstracted type) Structure representing an animation through its specifications (static data).
 * Can be shared by several instances running the described animation.
 */
#define TTAnimationSpec TT_APPEND_MOT_VERSION(TTAnimationSpec)

/**
 * @struct TTPoseDescription
 * @brief (Abstracted type) Structure assembling all the required information for the engine to understand how to draw a pose for a model.
 */
#define TTPoseDescription TT_APPEND_MOT_VERSION(TTPoseDescription)

 /**
 * @struct TTAnimationBuffer
 * @brief (Abstracted type) Container describing where to load an animation set.
 */
#define TTAnimationBuffer TT_APPEND_MOT_VERSION(TTAnimationBuffer)

/**
 * @struct TTInstancePosition
 * 
 * @struct TTMOTFileHeader
 * 
 * @struct TTAnimationState
 */

/**
 * @fn void ttLoadMOTMetadata(char* fileName, TTMOTFileMetadata* metadata)
 * @brief (Abstracted function) Loads the metadata of the animations described in a .MOT file into the given struct.
 * @note Requires GFS_Init() to be called prior to use.
 * @note This function is almost as slow as loading the file with ttLoadMOTMetaData0. A workflow based on checking the size
 * and then opening the file (this function and then ttLoadMOTMetaData0) should be avoided to optimize loading speed.
 * 
 * @param[in] filename char* The name of the file to be opened in the current directory.
 * @param[out] metadata TTMOTFileMetadata* The structure where the metadata are to be loaded.
 *
 * @see GFS_Init()
 */
#define ttLoadMOTMetadata(fileName, metadata) TT_APPEND_MOT_VERSION(ttLoadMOTMetadata)(fileName, metadata)

/**
 * @fn TTAnimationLoadStatus ttLoadMOTAnimation(char* fileName, TTAnimationBuffer* animationBuffer)
 * @brief (Abstracted function) Loads an animation set from the given file into the given user-allocated arrays.
 * @note Requires GFS_Init() to be called prior to use.
 * 
 * @param[in] filename The name of the file to be opened in the current directory.
 * @param[in,out] animationBuffer structure providing the pointers to where the data will be loaded.
 * 
 * @warning Output arrays must be pre-allocated with sufficient capacity to hold 
 * the entire file content to avoid returning an error code.
 * @see GFS_Init()
 */
#define ttLoadMOTAnimation(fileName, animationBuffer) TT_APPEND_MOT_VERSION(ttLoadMOTAnimation)(fileName, animationBuffer)

/**
 * @fn void ttStartAnimation(TTAnimationSpec* animationSpec, TTAnimationState* animationState)
 * @brief (Abstracted function) Starts an animation and stores the new animation instance state in the given animationState.
 * 
 * @param[in] animationSpec The animation to be started.
 * @param[out] animationState The state of the animation instance to be started.
 */
#define ttStartAnimation(animationSpec, animationState) TT_APPEND_MOT_VERSION(ttStartAnimation)(animationSpec, animationState)

/**
 * @fn void ttUpdateAnimation(TTAnimationState* animationState)
 * @brief (Abstracted function) Updates the state of an instance of an animation.
 * Must be called at every frame to maintain the state up to date.
 * 
 * @param[in] animationState The animation instance to be kept updated.
 */
#define ttUpdateAnimation(animationSpec, animationState) TT_APPEND_MOT_VERSION(ttUpdateAnimation)(animationState)

/**
 * @fn void ttDrawAnimatedModel(PDATA model[], TTPoseDescription* restrict poseDescription)
 * @brief (Abstracted function) Draws the given animated model according to the given pose.
 * 
 * @param[in] model The meshes composing the model to be animated.
 * @param[in] poseDescription The description of the current pose to be applied to the model.
 * @see ttDrawAnimatedModelAtPose1()
 */
#define ttDrawAnimatedModel(model, poseDescription) TT_APPEND_MOT_VERSION(ttDrawAnimatedModelAtPose)(model, poseDescription)

/**
 * @} //closes group Animation
 */

#endif //ANIMATION_H