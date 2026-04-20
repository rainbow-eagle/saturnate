#include "jo/sgl_prototypes.h"
#include "animation.h"
#include "file.h"
#include "memory.h"

void ttLoadAnimation(char* fileName, TTMOTFileHeader* animationSetSpec, TTAnimationSpec* animationSpecs, TTTransform* animations) {
    TTFileStream stream;
    ttOpenFileStream(fileName, &stream);
    animationSetSpec = (TTMOTFileHeader*) TO_UNCACHED_ADDR(animationSetSpec);
    animationSpecs = (TTAnimationSpec*) TO_UNCACHED_ADDR(animationSpecs);

    ttReadBytes(&stream, sizeof(TTMOTFileHeader), (Uint8*) animationSetSpec);
    Uint32 animationCount = animationSetSpec->animationCount;
    Uint32 size = sizeof(TTAnimationSpec) * animationCount;
    ttReadBytes(&stream, size, (Uint8*) animationSpecs);
    size = 0;
    Uint32 i = animationCount;
    do
        size += animationSpecs++->totalPoseCount;
    while(--i);
    size *= animationSetSpec->meshCount * sizeof(TTTransform);
    ttReadBytes(&stream, size, (Uint8*) animations);

    ttCloseFileStream(&stream);
}

// void ttLoadAnimation(char* fileName, TTMOTFileHeader* animationSetSpec, TTAnimationSpec* animationSpecs, TTTransform* animations) {
//     ttLoadFileInPool(fileName);

//     TTMOTFileHeader* fileHeader = (TTMOTFileHeader*) TO_UNCACHED_ADDR(loadedPool);
//     *animationSetSpec = *fileHeader;
//     Uint32 meshCount = fileHeader->meshCount;
//     char* bufferPointer = TO_UNCACHED_ADDR(loadedPool) + sizeof(TTMOTFileHeader);
//     Uint32 size = sizeof(TTAnimationSpec) * fileHeader->animationCount;
//     //TTAnimationSpec is 32bits aligned so I can transfer with "Long" (i.e. using Sinc_Dinc_Long) to go faster
//     slDMAXCopy((void*) bufferPointer, animationSpecs, size, Sinc_Dinc_Long);
//     TTAnimationSpec* animationSpecPtr = (TTAnimationSpec*) bufferPointer;
//     bufferPointer += size;
//     size = 0;
//     for(int i = fileHeader->animationCount; i > 0 ; i--, animationSpecPtr++)
//         size += animationSpecPtr->totalPoseCount;
//     size *= meshCount * sizeof(TTTransform);
//     //TTTransform is 32bits aligned so I can transfer with "Long" (i.e. using Sinc_Dinc_Long) to go faster
//     slDMAXCopy((void*) bufferPointer, animations, size, Sinc_Dinc_Long);
// }

// void loadAnimationSC1(char* fileName, TTAnimationSpec* animationSpecs, Uint32 meshCount, TTTransformSC animationTransforms[][meshCount]) {
//     ttLoadFileInPool(fileName);

//     AnimationFileHeader* fileHeader = (AnimationFileHeader*) TO_UNCACHED_ADDR(loadedPool);
//     char* bufferPointer = TO_UNCACHED_ADDR(loadedPool) + sizeof(AnimationFileHeader);
//     Uint32 size = sizeof(TTAnimationSpec) * fileHeader->animationCount;
//     //TTAnimationSpec is 32bits aligned so I can transfer with "Long" (i.e. using Sinc_Dinc_Long) to go faster
//     slDMAXCopy((void*) bufferPointer, animationSpecs, size, Sinc_Dinc_Long);
//     bufferPointer += size;
//     size = 0;
// slDMAWait();
//     for(int i = 0; i < fileHeader->animationCount; i++) {// make it so you read from the buffer and don't need to slDMAWait above
//         size += animationSpecs[i].totalPoseCount;
//     }
//     size *= meshCount * sizeof(TTTransformSC);
//     //TTTransform is 32bits aligned so I can transfer with "Long" (i.e. using Sinc_Dinc_Long) to go faster
//     slDMAXCopy((void*) bufferPointer, animationTransforms, size, Sinc_Dinc_Long);
// }


