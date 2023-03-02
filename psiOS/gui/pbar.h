#ifndef __PBAR_H
#define __PBAR_H

#include "..\core.h"

typedef struct tagProgressBar
{
	int w;				// size in pixels
	fixed step;			// how much 'pos' grows with each step
	fixed pos, max;		// current and max position
} PROGRESS_BAR;

//
void PBarInitGfx(const PNG_UPLOAD *pos);
// initialize a progress bar control
static __inline void PBarInit(PROGRESS_BAR *pbar) {zmemset(pbar,0,sizeof(*pbar));}
// set bar width in pixels
static __inline void PBarSetWidth(PROGRESS_BAR *pbar, const int w) {pbar->w=w;}
// set value for 'step'
static __inline void PBarSetStep(PROGRESS_BAR *pbar, const fixed step) {pbar->step=step;}
// set total of steps possible
static __inline void PBarSetMax(PROGRESS_BAR *pbar, const int max) {pbar->max=max;}
// progress by a step
fixed PBarStepIt(PROGRESS_BAR *pbar);
// draw bar to screen
void PBarDraw(PROGRESS_BAR *pbar, const int x, const int y, void *ot);

#endif
