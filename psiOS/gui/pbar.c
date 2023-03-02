#include "pbar.h"

enum LIST_MAPPING
{
	MAP_LEFT,
	MAP_MIDDLE,
	MAP_RIGHT,
	MAP_BAR
};

static GFX_MAPPING map[]=
{
//	x	y	u	v	w	h	clut				tage
	{0,	0,	8,	0,	8,	8,	getClut(0,0),		getTPage(0,0,0,0)},	// bar left
	{0,	0,	16,	0,	8,	8,	getClut(0,0),		getTPage(0,0,0,0)},	// bar middle
	{0,	0,	24,	0,	8,	8,	getClut(0,0),		getTPage(0,0,0,0)},	// bar right
	{0,	0,	0,	0,	8,	8,	getClut(0,0),		getTPage(0,0,0,0)}	// bar progress indicator
};

// initiliaze bar graphics
void PBarInitGfx(const PNG_UPLOAD *pos)
{
	int i;
	u32 tpage, clut, u, v;

	tpage=getTPage(0,0,pos->px&0x300,pos->py&0x100);
	clut=getClut(pos->cx,pos->cy);
	u=(pos->px&0x3F)*4;
	v=pos->py&0xFF;

	DEBUGPRINT(("Loading pbar tpage:%x, clut: %x, u-v: %d-%d\n",tpage,clut,u,v));

	for(i=0; i<DIMENSION_OF(map); i++)
	{
		map[i].clut=clut;
		map[i].tpage=tpage;
		map[i].u+=u;
		map[i].v+=v;
	}
}

// progress by a step
fixed PBarStepIt(PROGRESS_BAR *pbar)
{
	// step it!
	pbar->pos+=pbar->step;
	// wrap max value
	if(pbar->pos>pbar->max) pbar->pos=pbar->max;

	return pbar->pos;
}

void PBarDraw(PROGRESS_BAR *pbar, const int x, const int y, void *ot)
{
	DR_TWINX *win=(DR_TWINX*)MallocGfx(sizeof(DR_TWINX)*2);
	// bar size in pixels
	int size=pbar->pos*pbar->w/pbar->max;
}
