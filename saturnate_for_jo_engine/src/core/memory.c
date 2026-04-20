#include "jo/sgl_prototypes.h"
#include "memory.h"

void ttCopyBytes(Uint8* dest, Uint8* src, Uint32 byteCount) {
    // if(byteCount > 256) {
    //     slDMAXCopy((void*) src, dest, byteCount, Sinc_Dinc_Byte); 
    // }
    // else {
        Uint32 *dest32 = (Uint32*)dest;
        Uint32 *src32 = (Uint32*)src;
        while(byteCount >= 4) {
            *dest32++ = *src32++;
            byteCount -= 4;
        }
        dest = (Uint8*)dest32;
        src = (Uint8*)src32;
        while (byteCount--)
            *dest++ = *src++;
    // }
}
