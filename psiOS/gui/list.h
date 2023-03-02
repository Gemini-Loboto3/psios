#ifndef __LIST_H
#define __LIST_H

#include "..\core.h"

typedef struct tagScrollList
{
	short pos;			// actual position
	short curs_pos;		// page position
	short scroll;		// scrolling position
	short cols, rows;
	short max;			// element total for the list
	short smax;			// cursor range max on the list
//	short sblo, sbhi;	// high/low boudaries for scrolling
} SCROLL_LIST;

void InitSList(SCROLL_LIST *list, int rows, int cols, int tot);
void UpdateSListData(SCROLL_LIST *list);
// simple +/-1 movement
int SListMoveForward(SCROLL_LIST *list);
int SListMoveBackward(SCROLL_LIST *list);
// skip an entire row
int SListMoveRForward(SCROLL_LIST *list);
int SListMoveRBackward(SCROLL_LIST *list);
// skip an entire page
int SListMovePForward(SCROLL_LIST *list);
int SListMovePBackward(SCROLL_LIST *list);

#endif
