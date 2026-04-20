#include <sl_def.h>
#include "memory.h"

void ttCopyBytes(uint8_t* dest, uint8_t* src, uint32_t byteCount) {
    // if(byteCount > 256) {
    //     slDMAXCopy((void*) src, dest, byteCount, Sinc_Dinc_Byte); 
    // }
    // else {
        uint32_t *dest32 = (uint32_t*)dest;
        uint32_t *src32 = (uint32_t*)src;
        while(byteCount >= 4) {
            *dest32++ = *src32++;
            byteCount -= 4;
        }
        dest = (uint8_t*)dest32;
        src = (uint8_t*)src32;
        while (byteCount--)
            *dest++ = *src++;
    // }
}