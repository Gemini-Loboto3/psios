#ifndef __GRAPH_H
#define __GRAPH_H

//#include "common.h"

//#ifndef WIN32
//#include <sys/types.h>
//#include <libgpu.h>
//#else
//#include "porting.h"
//#endif

typedef struct tagDoubleBuffer
{
	DRAWENV draw;			// drawing environment
	DISPENV disp;			// display environment
} DB;

typedef struct tagDrTWinX
{
	u_long	tag;
	u_long	code[1];
} DR_TWINX, DR_OFFSETX;					// Texture Window fix

typedef struct WINDOW
{
	POLY_G4 back;			// background with gradient
	GsBOXF border;
} WINDOW;

typedef struct WINCOLOR
{
	u_char r0, g0, b0;	// upper color
	u_char r1, g1, b1;	// lower color
} WINCOLOR;

#define WM_OPAQUE		0
#define WM_TRANSPARENT	1

// menu specific data
typedef struct tagGfx
{
	short x, y;
	u_char u, v;
	u_short w, h;
	u_short clut;
	int tpage;
} GFX_MAPPING;

typedef struct GRAD_RGB
{
	u_char r0, g0, b0;
	u_char r1, g1, b1;
	u_char flag;		// 1=transparent
} GRAD_RGB;

// 6 words, variable w/h
typedef struct tagFastSprt
{
	// DR_MODE
	u32 tag;		// mode tag, 0x05XXXXXX
	u32 mode;		// tpage and mode
	// SPRT
	u32 rgbc;		// rgb and sprite code, 0x65000000
	u32 xy;			// x|(y<<16)
	u32 uvclut;		// u|(v<<8)|(clut<<16)
	u32 wh;			// w|(h<<16)
} FAST_SPRT;

// 5 words
typedef struct tagFastSSprt
{
	// DR_MODE
	u32 tag;		// mode tag, 0x04XXXXXX
	u32 mode;		// tpage and mode
	// SPRT
	u32 rgbc;		// rgb and sprite code, 0x65000000
	u32 xy;			// x|(y<<16)
	u32 uvclut;		// u|(v<<8)|(clut<<16)
} FAST_SSPRT;

typedef struct tagFadeTile
{
	// DR_MODE
	u32 tag;		// mode tag, 0x04XXXXXX
	u32 mode;		// tpage and mode
	// TILE
	u32 rgbc;		// rgb and sprite code, 0x60000000
	u32 xy;			// x|(y<<16)
	u32 wh;			// w|(h<<16)
} FADE_TILE;

#define SetFSprt(p,tpage)			{((FAST_SPRT*)(p))->tag=0x05000000; ((FAST_SPRT*)(p))->mode=0xE1000000|(tpage);}
#define SetFSSprt(p,tpage)			{((FAST_SPRT*)(p))->tag=0x04000000; ((FAST_SPRT*)(p))->mode=0xE1000000|(tpage);}
#define SetFSprtRGB(p,norgb)		((FAST_SPRT*)(p))->rgbc=0x64000000|((norgb)<<24)
#define SetFSprtRGBS(p,r,g,b)		((FAST_SPRT*)(p))->rgbc=0x64000000|(r)|(g<<8)|(b<<16)
#define SetFSprtRGB8(p,norgb)		((FAST_SPRT*)(p))->rgbc=0x74000000|((norgb)<<24)
#define SetFSprtRGB8S(p,r,g,b)		((FAST_SPRT*)(p))->rgbc=0x74000000|(r)|(g<<8)|(b<<16)
#define SetFSprtRGB16(p,norgb)		((FAST_SPRT*)(p))->rgbc=0x7C000000|((norgb)<<24)
#define SetFSprtRGB16S(p,r,g,b)		((FAST_SPRT*)(p))->rgbc=0x7C000000|(r)|(g<<8)|(b<<16)
#define SetFSprtUVClut(p,u,v,clut)	((FAST_SPRT*)(p))->uvclut=(u)|((v)<<8)|((clut)<<16)
#define SetFSprtXY(p,x,y)			((FAST_SPRT*)(p))->xy=((x)&0xFFFF)|((y)<<16)
#define SetFSprtWH(p,w,h)			((FAST_SPRT*)(p))->wh=(w)|((h)<<16)

#define SetFTile(p,tpage)			{((FADE_TILE*)(p))->tag=0x04000000; ((FADE_TILE*)(p))->mode=0xE1000000|(tpage);}
#define SetFTileRGB(p,r,g,b,c,t)	((FADE_TILE*)(p))->rgbc=0x60000000|((c)<<24)|((t)<<25)|(r)|((g)<<8)|((b)<<16)
#define SetFTileXY(p,x,y)			((FADE_TILE*)(p))->xy=((x)&0xFFFF)|((y)<<16)
#define SetFTileWH(p,w,h)			((FADE_TILE*)(p))->wh=(w)|((h)<<16)

#define SCRATCHPAD		0x1F800000	// cpu ram
#define SCRATCHPAD_SIZE	1024		// 1kb

#define MapTPage(x)		(x)&0x1FFF
#define MapVFlip(x)		(x)&0x8000
#define MapHFlip(x)		(x)&0x4000

enum
{
	FADE_IN,
	FADE_OUT
};

#define FADE_SPEED	64

extern GsOT WorldOrderingTable[2];
extern GsOT_TAG zSortTable[2][1<<OT_LENGTH];
extern PACKET GpuOutputPacket[2][PACKETMAX2];
extern int outputBufferIndex;

extern void *alloc_gfx;

static __inline GsOT_TAG* GetOTag() {return WorldOrderingTable[outputBufferIndex].org;}
static __inline int GetBufferIndex() {return outputBufferIndex;}
void SetTexWindowX(DR_TWINX *p, RECT *tw);

// Prototypes
#ifndef WIN32
void InitGfxAlloc(int size);
void CloseGfxAlloc();
void* MallocGfx(int size);
static __inline void* GetGfxPtr() {return alloc_gfx;}
static __inline void SetGfxPtr(void *address) {alloc_gfx=address;}
void DeallocGfx();
void BeginDraw();
void EndDraw(int r, int g, int b);

SPRT* SortMapSprt(int x, int y, GFX_MAPPING *map, GsOT_TAG *ot);
void SetMapSprt(int x, int y, SPRT *sprite, GFX_MAPPING *map);
void SortMapPoly(int x, int y, GFX_MAPPING *map, GsOT_TAG *ot, int flip);
void SetMapPoly(int x, int y, POLY_FT4 *poly, GFX_MAPPING *map, int flip);

/* Functions for screen fade in/out */
#define FADE_VISIBLE	255
#define FADE_INVISIBLE	0

enum FADE_PHASES
{
	FADE_IDLE,			// do nothing
	FADEIN_START,		// from black to invisible
	FADEIN_WORKING,		// cycle until done
	FADEOUT_START,		// from invisible to black
	FADEOUT_WORKING,	// cycle until done
	FADE_TRANSITION,	// change screen drawing here
	FADEOUTMAP_START,	//
	FADEOUTMAP_WORKING,	//
	FADEMAP_TRANSITION	//
};

void SetFade(int val);
void SetFadeSpeed(int val);
int GetFade();
void FadeSpeed(int val);
void FadeOutStart();
int FadeOut();
int FadeOutMap();
void FadeInStart();
int FadeIn();
void DrawFade();

void LoadTim(long *tim);
void LoadTim2(long *tim, u_short x, u_short y, u_short cx, u_short cy);

void InitWindow(int x, int y, int width, int height, WINCOLOR color, int mode, WINDOW *win);
void DrawWindow(WINDOW *win);
void InitPolyFT4(POLY_FT4* polygon, u_short x, u_short y, u_short w, u_short h, u_char u, u_char v, u_short clut, u_short tpage, u_char mode);
void InitPolyF4(POLY_F4* polygon, u_short x, u_short y, u_short w, u_short h, u_char r, u_char g, u_char b);
#endif

//void DecompressMlz(void *data);

#endif // __GRAPH_H
