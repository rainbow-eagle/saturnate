/*-------------------------------------------------------------------------*/
/*      Workarea assignment customize file                                 */
/*          for SGL ver. 2.10 (default)                                    */
/*                                                                         */
/*-------------------------------------------------------------------------*/

#include <sl_def.h>

/*---- [1.This part must not be modified] ---------------------------------*/
#define		SystemWork		0x060ffc00			/* System Variable         */

/*---- [2.This part must not be modified] ---------------------------------*/

// #define		MAX_VERTICES	5500	/* number of vertices that can be used */
// #define		MAX_POLYGONS	4800	/* number of vertices that can be used */
// #define		MAX_VERTICES	2500	/* number of vertices that can be used */
// #define		MAX_POLYGONS	1700	/* number of vertices that can be used */
#define		MAX_VERTICES	5000	/* number of vertices that can be used */
#define		MAX_POLYGONS	4000	/* number of vertices that can be used */
// #define		MAX_VERTICES	5500	/* number of vertices that can be used */
// #define		MAX_POLYGONS	3750	/* number of vertices that can be used */
// #define		MAX_VERTICES	5300	/* number of vertices that can be used */
// #define		MAX_POLYGONS	3600	/* number of vertices that can be used */

#define		MAX_EVENTS		  64	/* number of events that can be used   */
#define		MAX_WORKS		 256	/* number of works that can be used    */

#define		WORK_AREA		0x06080000			/* SGL Work Area           */
// #define		WORK_AREA		0x060C0000			/* SGL Work Area           */

#define		trans_list		0x060fb800		/* DMA Transfer Table      */

#define		pcmbuf			SoundRAM+0x78000	/* PCM Stream Address      */
#define		PCM_SIZE		0x8000				/* PCM Stream Size         */

#define		master_stack	SystemWork			/* MasterSH2 StackPointer  */
#define		slave_stack		0x06001e00			/* SlaveSH2  StackPointer  */

/*---- [3.Macro] ----------------------------------------------------------*/
// #define		_Byte_			sizeof(uint8_t)
// #define		_Word_			sizeof(uint16_t)
// #define		_LongWord_		sizeof(uint32_t)
// #define		_Sprite_		(sizeof(uint16_t) * 18)

#define		AdjWork(pt,sz,ct)	(pt + (sz) * (ct))

/*---- [4.My custom Work Areain the .bss] ------------------------------------------------------*/
// For whatever reason, the SGL's sample approach consisting of directly assigning addresses to the variables (SortList, Zbuffer etc.)
// through the use of an enum (see commented code below) doesn't work. But using a struct works fine so here ya go.
// struct WorkArea_
// {
//     char SortList[(_LongWord_ * 3) * (MAX_POLYGONS + 6)];
//     char Zbuffer[_LongWord_ * MAX_POLYGONS];
//     char SpriteBuf[_Sprite_ * ((MAX_POLYGONS + 6) * 2)];
//     char Pbuffer[(_LongWord_ * 4) * MAX_VERTICES];
//     char CLOfstBuf[(_Byte_ * 32 * 3) * 32];
//     char CommandBuf[(_LongWord_ * 8) * MAX_POLYGONS];
//     char NextEntry;
// };

/*---- [4.My custom dynamic Work Area (doesn't work)] ------------------------------------------------------*/
// The idea is to dynamically decide the begining of SGL's WORK_AREA by going top to bottom.
// We start where the stack ends (that is, at the top of the WORK_AREA) and do exactly like SGL's original workarea.c,
// but in reverse (that is, we progressively remove the size of each section from the top address depending on the chosen MAX_VERTICES etc.).
// Also, instead of putting trans_list at a fixed address, we put it right after the work area, into the master stack.
// SGL's documentation gives it a fixed size with maximum number of transfer = MAX_TRANSFER, but this notion never appears in this file
// so instead we let it grow bottom to top in the stack, while the stack grows top to bottom. Idk whether there's an actual maximum or
// whether we're just bounded by whatever moment trans_list grows into the used stack or vice versa.
// I'm not paying attention to the 32bits alignement, maybe I should... Well it's functional asis anyway.

#define     STACK_SIZE      0x216C //that's the stack size described by SGL documentation

// #define		PreviousWork(pt,sz,ct)	(pt - (sz) * (ct))
#define     WORK_AREA_END   0x60FC718 //that's the end of work area that starts at 0x060C0000 with default length (2500, 1700, 64, 256) (stack size is implicitely 0x34E8)
// #define     WORK_AREA_END   ((master_stack - STACK_SIZE) - 0x2000) //that should be the starting point with a smaller stack (size documented in SGL documentation), leaving more free ram for the main code
// #define     WORK_AREA_END   trans_list //that's the logical limit to not write workarea over trans_list, but it works to write over it so ignore this line

// enum workarea{
//     NextEntry  = WORK_AREA_END,
//     commandbuf = PreviousWork(NextEntry, _LongWord_ * 8, MAX_POLYGONS) ,
//     clofstbuf  = PreviousWork(commandbuf , _Byte_ * 32*3, 32) ,
//     pbuffer    = PreviousWork(clofstbuf   , _LongWord_ * 4, MAX_VERTICES) ,
//     spritebuf  = PreviousWork(pbuffer , _Sprite_, (MAX_POLYGONS + 6) * 2) ,
//     zbuffer    = PreviousWork(spritebuf   , _LongWord_, MAX_POLYGONS) ,
//     sort_list  = PreviousWork(zbuffer , _LongWord_ * 3, MAX_POLYGONS + 6)
// } ;

/*---- [4.Work Area] ------------------------------------------------------*/

#define	workSize(sz,ct)	((sz) * (ct))
#define WORK_AREA_SIZE (workSize(_LongWord_ * 3, MAX_POLYGONS + 6) + workSize(_LongWord_, MAX_POLYGONS) \
                        + workSize(_Sprite_, (MAX_POLYGONS + 6) * 2) + workSize(_LongWord_ * 4, MAX_VERTICES) \
                        + workSize(_Byte_ * 32*3, 32) + workSize(_LongWord_ * 8, MAX_POLYGONS))
// #define WORK_AREA ((WORK_AREA_END - (WORK_AREA_SIZE)) & 0xFFFFF000)
// #define WORK_AREA ((WORK_AREA_END - WORK_AREA_SIZE) & 0xFFFF0000)

// enum workarea{
//     sort_list  = WORK_AREA ,
//     zbuffer    = AdjWork(sort_list , _LongWord_ * 3, MAX_POLYGONS + 6) ,
//     spritebuf  = AdjWork(zbuffer   , _LongWord_, MAX_POLYGONS) ,
//     pbuffer    = AdjWork(spritebuf , _Sprite_, (MAX_POLYGONS + 6) * 2) ,
//     clofstbuf  = AdjWork(pbuffer   , _LongWord_ * 4, MAX_VERTICES) ,
//     commandbuf = AdjWork(clofstbuf , _Byte_ * 32*3, 32) ,
//     NextEntry  = AdjWork(commandbuf, _LongWord_ * 8, MAX_POLYGONS)
// } ;
