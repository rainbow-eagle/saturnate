/*-------------------------------------------------------------------------*/
/*      Workarea assignment customize file                                 */
/*          for SGL ver. 2.10 (default)                                    */
/*                                                                         */
/*-------------------------------------------------------------------------*/

#include <jo/sgl_prototypes.h>

/*---- [1.This part must not be modified] ---------------------------------*/
#define		SystemWork		0x060ffc00			/* System Variable         */

/*---- [2.This part must not be modified] ---------------------------------*/

#define		MAX_VERTICES	5000	/* number of vertices that can be used */
#define		MAX_POLYGONS	4000	/* number of vertices that can be used */

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
// #define		_Byte_			sizeof(Uint8)
// #define		_Word_			sizeof(Uint16)
// #define		_LongWord_		sizeof(Uint32)
// #define		_Sprite_		(sizeof(Uint16) * 18)

#define		AdjWork(pt,sz,ct)	(pt + (sz) * (ct))

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
