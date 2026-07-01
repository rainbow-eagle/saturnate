#ifndef TT_POOL_ANIMATION_COMMON_H
#define TT_POOL_ANIMATION_COMMON_H

#include <sl_def.h>
#include "../../core/memory.h"
#include "../../core/format/animation_common.h"

typedef enum {
    TT_ANIMATION_POOL_OVERFLOW = TT_ANIMATION_OVERFLOW,
    TT_BONE_POOL_OVERFLOW = TT_BONE_OVERFLOW,
    TT_POSE_POOL_OVERFLOW = TT_POSE_OVERFLOW,
    TT_TRANSFORM_POOL_OVERFLOW = TT_TRANSFORM_OVERFLOW,
    TT_ANIMATION_SET_POOL_OVERFLOW = -5
} TTAnimationPoolLoadStatus;

#endif //TT_POOL_ANIMATION_COMMON_H