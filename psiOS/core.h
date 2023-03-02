#ifndef __CORE_H
#define __CORE_H

#include "engine.h"

// common definitions and macros
#include "core\common.h"
#include "core\general.h"
#include "core\fixed.h"

// DEBUGPRINT assertion
#include "core\asserte.h"
#include "core\font.h"

//// character data
//#include "core\char.h"
//
//// compression modules
//#include "core\compress.h"

// graphics
//#include "core\gte.h"
#include "core\graph.h"

// cd-rom management
#include "core\cd.h"
#include "core\cdfile.h"
#include "core\cdthread.h"

//// sound and streaming
//#include "core\sound.h"
//#include "core\stream.h"

// pad management
#include "core\ctrller.h"
#include "core\control.h"
//
//// memory card management
//#include "core\memcard.h"

#include "core\readpng.h"

/* ---------------------------------------------------------------------------
 * - GLOBAL DEFINITIONS 
 * ---------------------------------------------------------------------------
 */
#define MAX_VOL			127
#define INPUT_DELAY		25

#ifndef NDS
extern unsigned long __heapbase;
extern unsigned long __heapsize;
extern unsigned long __bss;
extern unsigned long __bsslen;
extern unsigned long __data;
extern unsigned long __datalen;
extern unsigned long __text;
extern unsigned long __textlen;
extern unsigned long _ramsize;
extern unsigned long _stacksize;
#endif

//extern OBJECT player;
//extern OBJECT spell;
//extern OBJECT weapon[];
//extern OBJECT enemy[];

extern GsBOXF box;

#define STACK			(0x80000000+_ramsize-_stacksize)

#endif	// __CORE_H
