/************************************************************************
 GUI helpers: scrolling lists
	This is just a set of instructions for allowing easier management of
	scrolling-sensitive lists
*************************************************************************/
#include "list.h"

/* simple initalization to call before menu drawing */
void InitSList(SCROLL_LIST *list, int cols, int rows, int tot)
{
	zmemset(list,0,sizeof(SCROLL_LIST));
	list->rows=rows;
	list->cols=cols;
	list->smax=rows*cols;
	list->max=tot;
}

/* update internal values when switching to a different item list */
void UpdateSListData(SCROLL_LIST *list)
{
	list->curs_pos=0;
	list->pos=0;
	// redefine low boundary
//	if(list->smax > list->max) list->sblo=list->max-1;
//	else list->sblo=list->smax-1;
	// redefine high boundary
//	list->sbhi=((list->max+1)&(~1))-list->sblo;
//	if(list->sbhi<0) list->sbhi=list->max;
}

/* manage movement when pad==KEY_RIGHT */
int SListMoveForward(SCROLL_LIST *list)
{
	// see if we can step forward
	if(list->pos+1 < list->max)
	{
		// we can, increase position
		list->pos++;
		list->curs_pos++;

		// check if page scrolling is necessary
		if(list->curs_pos >= list->smax)
		{
			// scroll needed, go down once
			list->scroll+=list->cols;
			// fix cursor position to next row
			list->curs_pos=list->smax-list->cols;
		}
	}

	return list->curs_pos;
}

/* manage movement when pad==KEY_LEFT */
int SListMoveBackward(SCROLL_LIST *list)
{
	// see if we can step backward
	if(list->pos-1 >= 0)
	{
		// we can, decrease position
		list->pos--;
		list->curs_pos--;

		// check if page scrolling is necessary
		if(list->curs_pos < 0)
		{
			// scroll needed, go up once
			list->scroll-=list->cols;
			// fix cursor position
			list->curs_pos=list->cols-1;
		}
	}

	return list->curs_pos;
}

/* manage movement when pad==KEY_DOWN */
int SListMoveRForward(SCROLL_LIST *list)
{
	int i;

	for(i=0; i<list->cols; i++)
		SListMoveForward(list);
}

/* manage movement when pad==KEY_UP */
int SListMoveRBackward(SCROLL_LIST *list)
{
	int i;

	for(i=0; i<list->cols; i++)
		SListMoveBackward(list);
}

int SListMovePForward(SCROLL_LIST *list)
{
	int i, page;

	page=list->rows*list->cols;
	for(i=0; i<page; i++)
		SListMoveForward(list);
}

int SListMovePBackward(SCROLL_LIST *list)
{
	int i, page;

	page=list->rows*list->cols;
	for(i=0; i<page; i++)
		SListMoveBackward(list);
}
