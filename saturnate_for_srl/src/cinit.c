/* 

    Sample initialization routine required in C language (for SGL sample programs)
      1. Clear BSS (uninitialized area)
      2. Copy from ROM area to RAM area (not needed if it does not exist)
      3. Clear SGL system variable area
*/

#include <sgl.h>
// #include "workarea.h"

#define		SystemWork		0x060ffc00		/* System Variable Address */
#define		SystemSize		(0x06100000-SystemWork)		/* System Variable Size */
/* Start and end symbols of the .bss section specified in sl.lnk */
extern uint32_t _bstart, _bend;
extern uint32_t _ttlowram_start, _ttlowram_end;
/* */
extern void ss_main( void );

void	main( void )
{
    uint8_t	*dst;
    uint32_t	i;

    /* 1. Zero Set .bss Section */
    for( dst = (uint8_t *)&_bstart; dst < (uint8_t *)&_bend; dst++ ) {
        *dst = 0;
    }
    /* 2. ROM has data at end of text; copy it. */

    /* 3. SGL System Variable Clear */
    for( dst = (uint8_t *)SystemWork, i = 0;i < SystemSize; i++) {
        *dst = 0;
    }

    /* Application Call */
    ss_main();
}
