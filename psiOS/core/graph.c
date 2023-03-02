#include <sys/types.h>
#include <sys/file.h>
#include <stdio.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <MALLOC.H>
#include "common.h"
#include "graph.h"
#include "general.h"
#include "compress.h"

u8 GfxBuffer[8192];	// 128x128 4bpp images go here

// GFX dynamic allocation
volatile int alloc_swap=0;
void *alloc_gfx, *alloc_base[2];

void InitGfxAlloc(int size)
{
	alloc_base[0]=(void*)malloc3(size);
	alloc_base[1]=(void*)malloc3(size);
}

void CloseGfxAlloc()
{
	free3(alloc_base[0]);
	free3(alloc_base[1]);
}

void* MallocGfx(int size)
{
	void* cur_ptr=alloc_gfx;
	alloc_gfx+=size;

	return cur_ptr;
}

void DeallocGfx()
{
	alloc_gfx=alloc_base[alloc_swap];
	alloc_swap^=1;
}

void BeginDraw()
{
	DeallocGfx();
	outputBufferIndex=GsGetActiveBuff();
	GsSetWorkBase((PACKET*)GpuOutputPacket[outputBufferIndex]);
	GsClearOt(0,0,&WorldOrderingTable[outputBufferIndex]);
}

void EndDraw(int r, int g, int b)
{
	DrawSync(0);
	VSync(0);
	GsSwapDispBuff();
	GsSortClear(r,g,b,&WorldOrderingTable[outputBufferIndex]);
	GsDrawOt(&WorldOrderingTable[outputBufferIndex]);
}

#define SPRITE_BRIGHT	1<<6	// ignore rgb values
#define SPRITE_VFLIP	1<<22	// unused
#define SPRITE_HFLIP	1<<23
#define SPRITE_DEPTH	1<<24	// 2 bits: 0=4bpp, 1=8bpp, 2=16bpp
#define SPRITE_ROTATION	1<<27	// unused
#define SPRITE_TRANSR	1<<28	// 2 bits, transparency rate
#define SPRITE_TRANSP	1<<30	// transparency enabled or disabled
#define SPRITE_HIDDEN	1<<31

// fix for the sdk function
void SetTexWindowX(DR_TWINX *p, RECT *tw)
{
	((P_TAG*)p)->len=(u_char)(1);
	((u_long *)(p))[1]=_get_tw(tw);
}

void InitPolyFT4(POLY_FT4* polygon, u_short x, u_short y, u_short w, u_short h, u_char u, u_char v, u_short clut, u_short tpage, u_char mode)
{
	SetPolyFT4(polygon);
	polygon->r0=polygon->g0=polygon->b0=128;
	polygon->x0=polygon->x2=x;
	polygon->y0=polygon->y1=y;
	polygon->x1=polygon->x3=x+w;
	polygon->y2=polygon->y3=y+h;
	polygon->u0=polygon->u2=u;
//	if(mode==FT4_MODE4BPP) w>>=1;
//	if(u+w>0xFF) w--;
	polygon->u1=polygon->u3=u+w;
	polygon->v0=polygon->v1=v;
	polygon->v2=polygon->v3=v+h;
	polygon->clut=clut;
	polygon->tpage=tpage;
}

void InitPolyF4(POLY_F4* polygon, u_short x, u_short y, u_short w, u_short h, u_char r, u_char g, u_char b)
{
	SetPolyF4(polygon);
	polygon->r0=r;
	polygon->g0=g;
	polygon->b0=b;
	polygon->x0=polygon->x2=x;
	polygon->y0=polygon->y1=y;
	polygon->x1=polygon->x3=x+w;
	polygon->y2=polygon->y3=y+h;
}

void SetMapSprt(int x, int y, SPRT *sprite, GFX_MAPPING *map)
{
	setSprt(sprite);
	sprite->x0=x+map->x;
	sprite->y0=y+map->y;
	sprite->w=map->w;
	sprite->h=map->h;
	sprite->u0=map->u;
	sprite->v0=map->v;
	sprite->clut=map->clut;
	sprite->r0=sprite->g0=sprite->b0=0x80;
}

SPRT* SortMapSprt(int x, int y, GFX_MAPPING *map, GsOT_TAG *ot)
{
	// allocate primitives
	DR_TPAGE *mode=MallocGfx(sizeof(DR_TPAGE));
	SPRT *sprite=MallocGfx(sizeof(SPRT));

	SetMapSprt(x,y,sprite,map);
	setDrawTPage(mode,0,1,map->tpage);

	addPrim(ot,sprite);
	addPrim(ot,mode);

	return sprite;
}

void SetMapPoly(int x, int y, POLY_FT4 *poly, GFX_MAPPING *map, int flip)
{
	setPolyFT4(poly);
	poly->x0=poly->x2=x+map->x;
	poly->x1=poly->x3=x+map->x+map->w;

	poly->y0=poly->y1=y+map->y;
	poly->y2=poly->y3=y+map->y+map->h;

	switch(flip)
	{
	case 0:	// no flip
		poly->u0=poly->u2=map->u;
		if(map->u+map->w>255) poly->u1=poly->u3=255;
		else poly->u1=poly->u3=map->u+map->w;

		poly->v0=poly->v1=map->v;
		if(map->v+map->h>255) poly->v2=poly->v3=255;
		else poly->v2=poly->v3=map->v+map->h;
		break;
	case 1: // hor flip
		poly->x0=poly->x2=poly->x0;
		poly->x1=poly->x3=poly->x1;

		poly->u1=poly->u3=map->u-1;
		if(map->u+map->w-1>255) poly->u0=poly->u2=255;
		else poly->u0=poly->u2=map->u+map->w-1;

		poly->v0=poly->v1=map->v;
		if(map->v+map->h>255) poly->v2=poly->v3=255;
		else poly->v2=poly->v3=map->v+map->h;
		break;
	case 2: // ver flip
		poly->y0=poly->y1=poly->y0;
		poly->y2=poly->y3=poly->y2;

		poly->u0=poly->u2=map->u;
		if(map->u+map->w>255) poly->u1=poly->u3=255;
		else poly->u1=poly->u3=map->u+map->w;

		poly->v2=poly->v3=map->v-1;
		if(map->v+map->h-1>255) poly->v0=poly->v1=255;
		else poly->v0=poly->v1=map->v+map->h-1;
		break;
	case 3: // h+v flip
		poly->x0=poly->x2=poly->x0;
		poly->x1=poly->x3=poly->x1;
		poly->y0=poly->y1=poly->y0;
		poly->y2=poly->y3=poly->y2;

		poly->u1=poly->u3=map->u-1;
		if(map->u+map->w-1>255) poly->u0=poly->u2=255;
		else poly->u0=poly->u2=map->u+map->w-1;

		poly->v2=poly->v3=map->v-1;
		if(map->v+map->h-1>255) poly->v0=poly->v1=255;
		else poly->v0=poly->v1=map->v+map->h-1;
	}

	poly->clut=map->clut;
	poly->tpage=map->tpage;
	poly->r0=poly->g0=poly->b0=0x80;
}

void SortMapPoly(int x, int y, GFX_MAPPING *map, GsOT_TAG *ot, int flip)
{
	// allocate primitive
	POLY_FT4 *poly=MallocGfx(sizeof(POLY_FT4));

	setPolyFT4(poly);
	poly->x0=poly->x2=x+map->x;
	poly->x1=poly->x3=x+map->x+map->w;
	poly->y0=poly->y1=y+map->y;
	poly->y2=poly->y3=y+map->y+map->h;
	poly->clut=map->clut;
	poly->tpage=map->tpage;
	poly->r0=poly->g0=poly->b0=0x80;
	switch(flip)
	{
	case 0:	// no flip
		poly->u0=poly->u2=map->u;
		if(map->u+map->w>255) poly->u1=poly->u3=255;
		else poly->u1=poly->u3=map->u+map->w;

		poly->v0=poly->v1=map->v;
		if(map->v+map->h>255) poly->v2=poly->v3=255;
		else poly->v2=poly->v3=map->v+map->h;
		break;
	case 1: // hor flip
		poly->x0=poly->x2=poly->x0;
		poly->x1=poly->x3=poly->x1;

		poly->u1=poly->u3=map->u-1;
		if(map->u+map->w-1>255) poly->u0=poly->u2=255;
		else poly->u0=poly->u2=map->u+map->w-1;

		poly->v0=poly->v1=map->v;
		if(map->v+map->h>255) poly->v2=poly->v3=255;
		else poly->v2=poly->v3=map->v+map->h;
		break;
	case 2: // ver flip
		poly->y0=poly->y1=poly->y0;
		poly->y2=poly->y3=poly->y2;

		poly->u0=poly->u2=map->u;
		if(map->u+map->w>255) poly->u1=poly->u3=255;
		else poly->u1=poly->u3=map->u+map->w;

		poly->v2=poly->v3=map->v-1;
		if(map->v+map->h-1>255) poly->v0=poly->v1=255;
		else poly->v0=poly->v1=map->v+map->h-1;
		break;
	case 3: // h+v flip
		poly->x0=poly->x2=poly->x0;
		poly->x1=poly->x3=poly->x1;
		poly->y0=poly->y1=poly->y0;
		poly->y2=poly->y3=poly->y2;

		poly->u1=poly->u3=map->u-1;
		if(map->u+map->w-1>255) poly->u0=poly->u2=255;
		else poly->u0=poly->u2=map->u+map->w-1;

		poly->v2=poly->v3=map->v-1;
		if(map->v+map->h-1>255) poly->v0=poly->v1=255;
		else poly->v0=poly->v1=map->v+map->h-1;
	}

	addPrim(ot,poly);
}

/********************************************
 * GENERIC SPECIAL EFFECTS					*
 ********************************************/

// fade transparency counter
static int fade_val=0;		// 255=black, 0=white
// fade speed
static int fade_step=3;
// tile data
static FADE_TILE fade_tile[2]=
{
//	tag			tpage							tile		xy	wh
	{0x04000000,0xE1000000|getTPage(0,2,0,0),	0x60000000,	0,	320|(240<<16)},
	{0x04000000,0xE1000000|getTPage(0,2,0,0),	0x60000000,	0,	320|(240<<16)},
};

// set current value for fade
void SetFade(int val)
{
	fade_val=val;
}

void SetFadeSpeed(int val)
{
	fade_step=val;
}

int GetFade()
{
	return fade_val;
}

// set fade speed
void FadeSpeed(int val)
{
	fade_step=val;
}

// commence fade out
void FadeOutStart()
{
	int i=0;
	fade_val=0;

	for(i=0; i<2; i++)
	{
		SetFTile(&fade_tile[i],getTPage(0,2,0,0));
		SetFTileRGB(&fade_tile[i],0,0,0,0,1);
	}
}

// call every VSync, if 1 is returned FadeOut is complete */
int FadeOut()
{
	fade_val+=fade_step;
	if(fade_val>255) fade_val=255;

	SetFTileRGB(&fade_tile[outputBufferIndex],fade_val,fade_val,fade_val,0,1);
	//SetFTileRGB(&fade_tile[1],fade_val,fade_val,fade_val,0,1);

	if(fade_val==255) return 1;	// fade complete
	
	return 0;
}

// call every VSync, if 1 is returned FadeOutMap is complete */
int FadeOutMap()
{
	fade_val+=fade_step;
	if(fade_val>128) fade_val=128;

	SetFTileRGB(&fade_tile[outputBufferIndex],fade_val,fade_val,fade_val,0,1);
	//SetFTileRGB(&fade_tile[1],fade_val,fade_val,fade_val,0,1);

	if(fade_val==128) return 1;	// fade complete
	
	return 0;
}

// commence FadeIn
void FadeInStart()
{
	int i;
	fade_val=255;

	for(i=0; i<2; i++)
	{
		SetFTile(&fade_tile[i],getTPage(0,2,0,0));
		SetFTileRGB(&fade_tile[i],255,255,255,0,1);
	}
}

// call every VSync, if 1 is returned FadeIn is complete */
int FadeIn()
{
	fade_val-=fade_step;
	if(fade_val<0) fade_val=0;
	
	SetFTileRGB(&fade_tile[outputBufferIndex],fade_val,fade_val,fade_val,0,1);
	//SetFTileRGB(&fade_tile[1],fade_val,fade_val,fade_val,0,1);

	if(fade_val==0) return 1;
	return 0;
}

void DrawFade()
{
	if(fade_val!=FADE_INVISIBLE) AddPrim(GetOTag(),&fade_tile[outputBufferIndex]);
}

// ******* IMAGES ******
void LoadTim(long *tim)
{
	GsIMAGE image;

	GsGetTimInfo((u_long *)tim+1, &image);
	LoadImage((RECT*)&image.px, image.pixel);
	if(image.pmode&8) LoadImage((RECT*)&image.cx, image.clut);

	DrawSync(0);
}

void LoadTim2(long *tim, u_short x, u_short y, u_short cx, u_short cy)
{
	RECT rect;
	GsIMAGE image;

	GsGetTimInfo((u_long*)tim+1,&image);
	setRECT(&rect,x,y,image.pw,image.ph);
	LoadImage(&rect, image.pixel);
	if(image.pmode&8)
	{
		setRECT(&rect,cx,cy,image.cw,image.ch);
		LoadImage(&rect,image.clut);
	}

	DrawSync(0);
}

//char *cmp_type[]=
//{
//	"PERSONA_LZSS",
//	"TOSE_LZSS",
//	"NINTENDO_LZ77",
//	"NINTENDO_HUFFMAN",
//	"LUFIA_LZSS",
//	"SRTF_LZSS"
//};

// --------------------------------------------------------------------------
void InitWindow(int x, int y, int width, int height, WINCOLOR color, int mode, WINDOW *win)
{
	// angles init
	win->border.x=x;
	win->border.y=y;
	win->border.w=width;
	win->border.h=height;
	win->border.r=win->border.g=win->border.b=255;
	win->border.attribute=0;

	x+=1;
	y+=1;
	height-=2;
	width-=2;
	// background init
	win->back.r0=win->back.r1=color.r0;
	win->back.g0=win->back.g1=color.g0;
	win->back.b0=win->back.b1=color.b0;
	win->back.x0=win->back.x2=x;
	win->back.y0=win->back.y1=y;
	win->back.x1=win->back.x3=x+width;
	win->back.y2=win->back.y3=y+height;
	win->back.r2=win->back.r3=color.r1;
	win->back.g2=win->back.g3=color.g1;
	win->back.b2=win->back.b3=color.b1;

	SetPolyG4(&win->back);
	SetSemiTrans(&win->back,mode);
}

void DrawWindow(WINDOW *win)
{
	POLY_G4 *poly=MallocGfx(sizeof(POLY_G4));
	TILE *t=MallocGfx(sizeof(TILE));

	memcpy(poly,&win->back,sizeof(POLY_G4));
	SetTile(t);
	t->x0=win->border.x;
	t->y0=win->border.y;
	t->w=win->border.w;
	t->h=win->border.h;
	t->r0=win->border.r;
	t->g0=win->border.g;
	t->b0=win->border.b;
	//GsSortPoly(&win->back,&WorldOrderingTable[outputBufferIndex],0);
	//GsSortBoxFill(&win->border,&WorldOrderingTable[outputBufferIndex],0);
	AddPrim(GetOTag(),poly);
	AddPrim(GetOTag(),t);
}

//void DecompressMlz(void *data)
//{
//	int total, i;
//	void* temp;
//	CMP_HEADER *head;
//	u_long* ptr=(u_long*)data;
//
//	total=ptr[0]/4;
//	for(i=0; i<total; i++)
//	{
//		head=(CMP_HEADER*)(data+ptr[i]);
//		temp=(void*)malloc3(head->size);
//		//DEBUGPRINT(("Allocating gfx at %x (%d bytes), method %s\n",temp,head->size,cmp_type[head->type]));
//		Decompress(data+ptr[i],temp);
//		LoadTim((long*)temp);
//		free3(temp);
//	}
//}
