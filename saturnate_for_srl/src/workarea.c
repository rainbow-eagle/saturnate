#pragma GCC optimize("O0")
#include	<sl_def.h>
#include    "workarea.h"

/*---- [5.Variable area ] -------------------------------------------------*/

// // struct WorkArea_ workArea __attribute__((aligned(0x10000), used));
// // struct WorkArea_ __attribute__((aligned(0x1000), used, section("WORK_AREA"))) workArea;

// const void* const   __attribute__((section("SLPROG"))) MasterStack   = (void*)(master_stack) ;
// const void* const   __attribute__((section("SLPROG"))) SlaveStack    = (void*)(slave_stack) ;
// const uint16_t  __attribute__((section("SLPROG"))) MaxVertices   = MAX_VERTICES ;
// const uint16_t  __attribute__((section("SLPROG"))) MaxPolygons   = MAX_POLYGONS ;
// const uint16_t  __attribute__((section("SLPROG"))) MaxEvents     = MAX_EVENTS ;
// const uint16_t  __attribute__((section("SLPROG"))) MaxWorks      = MAX_WORKS ;
// const uint16_t  __attribute__((section("SLPROG"))) EventSize     = sizeof(EVENT) ;
// const uint16_t  __attribute__((section("SLPROG"))) WorkSize      = sizeof(WORK) ;

// const EVENT* const RemainEvent[MAX_EVENTS] __attribute__((section("SLPROG"))) ;
// const WORK* const  RemainWork[MAX_WORKS] __attribute__((section("SLPROG"))) ;
// const EVENT  __attribute__((section("SLPROG"))) EventBuf[MAX_EVENTS] ;
// const WORK   __attribute__((section("SLPROG"))) WorkBuf[MAX_WORKS] ;

// const void* const   __attribute__((section("SLPROG"))) PCM_Work      = (void*)(pcmbuf) ;
// const uint32_t  __attribute__((section("SLPROG"))) PCM_WkSize    = PCM_SIZE ;

// // const void* const   __attribute__((section("SLPROG"))) SortList      = (void*)(workArea.SortList) ;
// // const uint32_t  __attribute__((section("SLPROG"))) SortListSize  = (MAX_POLYGONS + 6) * _LongWord_ * 3 ;
// // const void* const   __attribute__((section("SLPROG"))) TransList     = (void*)(trans_list) ;
// // const void* const   __attribute__((section("SLPROG"))) Zbuffer       = (void*)(workArea.Zbuffer) ;
// // const void* const   __attribute__((section("SLPROG"))) SpriteBuf     = (void*)(workArea.SpriteBuf) ;
// // const uint32_t  __attribute__((section("SLPROG"))) SpriteBufSize = _Sprite_ * (MAX_POLYGONS + 6) * 2 ;
// // const void* const   __attribute__((section("SLPROG"))) Pbuffer       = (void*)(workArea.Pbuffer) ;
// // const void* const   __attribute__((section("SLPROG"))) CommandBuf    = (void*)(workArea.CommandBuf) ;
// // const void* const   __attribute__((section("SLPROG"))) CLOfstBuf     = (void*)(workArea.CLOfstBuf) ;
// // const void* const nextEntry = (void*) (&workArea.NextEntry);

// const void* const   __attribute__((section("SLPROG"))) SortList      = (void*)(sort_list) ;
// const uint32_t  __attribute__((section("SLPROG"))) SortListSize  = (MAX_POLYGONS + 6) * _LongWord_ * 3 ;
// const void* const   __attribute__((section("SLPROG"))) TransList     = (void*)(trans_list) ;
// const void* const   __attribute__((section("SLPROG"))) Zbuffer       = (void*)(zbuffer) ;
// const void* const   __attribute__((section("SLPROG"))) SpriteBuf     = (void*)(spritebuf) ;
// const uint32_t const  __attribute__((section("SLPROG"))) SpriteBufSize = _Sprite_ * (MAX_POLYGONS + 6) * 2 ;
// const void* const   __attribute__((section("SLPROG"))) Pbuffer       = (void*)(pbuffer) ;
// const void* const   __attribute__((section("SLPROG"))) CommandBuf    = (void*)(commandbuf) ;
// const void* const   __attribute__((section("SLPROG"))) CLOfstBuf     = (void*)(clofstbuf) ;
// const void* const nextEntry = (void*) (NextEntry);

/*------------------------------------------------------------------------*/

/* New method using assembly, extremely close result to original sglarea.o */
#define     _Byte_          1
#define     _Word_          2
#define     _LongWord_      4
#define     _Sprite_        (2 * 18)

#define     EVENT_SIZE_NUM  128   
#define     WORK_SIZE_NUM   64

#define MACRO_sort_list   WORK_AREA
#define MACRO_zbuffer     AdjWork(MACRO_sort_list , _LongWord_ * 3, MAX_POLYGONS + 6)
#define MACRO_spritebuf   AdjWork(MACRO_zbuffer   , _LongWord_, MAX_POLYGONS)
#define MACRO_pbuffer     AdjWork(MACRO_spritebuf , _Sprite_, (MAX_POLYGONS + 6) * 2)
#define MACRO_clofstbuf   AdjWork(MACRO_pbuffer   , _LongWord_ * 4, MAX_VERTICES)
#define MACRO_commandbuf  AdjWork(MACRO_clofstbuf , _Byte_ * 32*3, 32)
#define MACRO_nextentry   AdjWork(MACRO_commandbuf , _LongWord_ * 8, MAX_POLYGONS)

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define ASM_LONG(val)  ".long " STR(val) "\n\t"
#define ASM_WORD(val)  ".word " STR(val) "\n\t"
#define ASM_SKIP(size) ".skip " STR(size) "\n\t"

__asm__ (
    ".section SLPROG, \"ax\"\n\t"
    ".balign 16\n\t"

    /* [0x00] Pointeurs système */
    ".global _MasterStack\n\t"
    "_MasterStack:   " ASM_LONG(master_stack)
    ".global _SlaveStack\n\t"
    "_SlaveStack:    " ASM_LONG(slave_stack)

    /* [0x08] Constantes de base */
    ".global _MaxVertices\n\t"
    "_MaxVertices:   " ASM_WORD(MAX_VERTICES)
    ".global _MaxPolygons\n\t"
    "_MaxPolygons:   " ASM_WORD(MAX_POLYGONS)

    /* [0x0C] La Work Area (Sega la mettait ici !) */
    ".global _SortList\n\t"
    "_SortList:         " ASM_LONG(MACRO_sort_list)
    ".global _SortListSize\n\t"
    "_SortListSize:     " ASM_LONG((MAX_POLYGONS + 6) * _LongWord_ * 3)
    ".global _TransList\n\t"
    "_TransList:        " ASM_LONG(trans_list)
    ".global _Zbuffer\n\t"
    "_Zbuffer:          " ASM_LONG(MACRO_zbuffer)
    ".global _SpriteBuf\n\t"
    "_SpriteBuf:        " ASM_LONG(MACRO_spritebuf)
    ".global _SpriteBufSize\n\t"
    "_SpriteBufSize:    " ASM_LONG(_Sprite_ * (MAX_POLYGONS + 6) * 2)
    ".global _Pbuffer\n\t"
    "_Pbuffer:          " ASM_LONG(MACRO_pbuffer)
    ".global _CommandBuf\n\t"
    "_CommandBuf:       " ASM_LONG(MACRO_commandbuf)
    ".global _CLOfstBuf\n\t"
    "_CLOfstBuf:        " ASM_LONG(MACRO_clofstbuf)

    /* [0x30] Configuration PCM */
    ".global _PCM_Work\n\t"
    "_PCM_Work:      " ASM_LONG(pcmbuf)
    ".global _PCM_WkSize\n\t"
    "_PCM_WkSize:    " ASM_LONG(PCM_SIZE)

    /* [0x38] Constantes des structures (Bouches-trous d'alignement) */
    ".global _MaxEvents\n\t"
    "_MaxEvents:     " ASM_WORD(MAX_EVENTS)
    ".global _EventSize\n\t"
    "_EventSize:     " ASM_WORD(EVENT_SIZE_NUM)
    ".global _MaxWorks\n\t"
    "_MaxWorks:      " ASM_WORD(MAX_WORKS)
    ".global _WorkSize\n\t"
    "_WorkSize:      " ASM_WORD(WORK_SIZE_NUM)

    /* [0x40] Les gros buffers de réservation */
    ".global _RemainEvent\n\t"
    "_RemainEvent:\n\t"
    ASM_SKIP(4 * MAX_EVENTS)

    ".global _RemainWork\n\t"
    "_RemainWork:\n\t"
    ASM_SKIP(4 * MAX_WORKS)

    ".global _EventBuf\n\t"
    "_EventBuf:\n\t"
    ASM_SKIP(EVENT_SIZE_NUM * MAX_EVENTS)

    ".global _WorkBuf\n\t"
    "_WorkBuf:\n\t"
    ASM_SKIP(WORK_SIZE_NUM * MAX_WORKS)

    ".text\n\t"   
);