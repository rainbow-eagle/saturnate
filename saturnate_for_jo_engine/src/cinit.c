/* 

    Sample initialization routine required in C language (for SGL sample programs)
      1. Clear BSS (uninitialized area)
      2. Copy from ROM area to RAM area (not needed if it does not exist)
      3. Clear SGL system variable area
*/

#include "jo/sgl_prototypes.h"

#define		SystemWork		0x060ffc00		/* System Variable Address */
#define		SystemSize		(0x06100000-0x060ffc00)		/* System Variable Size */
/* Start and end symbols of the .bss section specified in sl.lnk */
extern Uint32 _bstart, _bend;
extern char _heap_end;
/* */
extern void ss_main( void );

int	main( void )
{
    Uint8	*dst;
    Uint32	i;

    /* 1. Zero Set .bss Section */
    for( dst = (Uint8 *)&_bstart; dst < (Uint8 *)&_bend; dst++ ) {
        *dst = 0;
    }
    /* 2. ROM has data at end of text; copy it. */

    /* 3. SGL System Variable Clear */
    for( dst = (Uint8 *)SystemWork, i = 0;i < SystemSize; i++) {
        *dst = 0;
    }

    /* Application Call */
    ss_main();
    return 0;
}
