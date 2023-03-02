/* ---------------------------------------------------------------------------
 * - (C) Sony Computer Entertainment. All Rights Reserved.
 * -
 * - Project:	Movie Player V2.0	
 * -
 * - Name:		control.h
 * -
 * - Author:	Vince Diesi
 * -
 * - Date:		13th Feb 1997
 * ---------------------------------------------------------------------------
 */

#ifndef __CONTROL_H
#define __CONTROL_H

#include "common.h"
#include "ctrller.h"

/* ---------------------------------------------------------------------------
 * - MACRO DEFINITIONS
 * ---------------------------------------------------------------------------
 */

// Check controllers every second.
#define CHECK_CONTROLLERS		50


// New (more meaningful) controller button names.
#define R2_KEY			PAD_FRB
#define L2_KEY			PAD_FLB
#define R1_KEY			PAD_FRT
#define L1_KEY			PAD_FLT
#define TRIANGLE_KEY	PAD_RU
#define X_KEY			PAD_RD
#define SQUARE_KEY		PAD_RL
#define CIRCLE_KEY		PAD_RR
#define UP_KEY			PAD_LU
#define DOWN_KEY		PAD_LD
#define LEFT_KEY		PAD_LL
#define RIGHT_KEY		PAD_LR
#define SELECT_KEY		PAD_SEL
#define START_KEY		PAD_START

#define VALID_INPUT		(R2_KEY|L2_KEY|R1_KEY|L1_KEY|TRIANGLE_KEY|X_KEY|SQUARE_KEY|CIRCLE_KEY|UP_KEY|DOWN_KEY|LEFT_KEY| \
							RIGHT_KEY|SELECT_KEY|START_KEY)

extern volatile int pad_lock;
extern volatile short __connected;			// No. controllers connected.
extern volatile short __currController;		// Current active controller.

extern volatile ControllerPacket __buffers[2];

/* ---------------------------------------------------------------------------
 * - PUBLIC FUNCTION PROTOTYPES
 * ---------------------------------------------------------------------------
 */

void InitControllers(void);

/* ------------------------------------------------------------------------ */

void StopControllers(void);

/* ------------------------------------------------------------------------ */

void CheckControllers(void);

/* ------------------------------------------------------------------------ */

static __inline short Pressed(short button) {return (__connected && !(__buffers[__currController].data.pad & button));}
short PressedLock(short button);
static __inline unsigned short GetPadValue() {if(__connected) return ~__buffers[__currController].data.pad; return 0;}
static __inline u16 GetPadValueLocked() {u16 ret; if(pad_lock<=0) {if((ret=GetPadValue())&VALID_INPUT) pad_lock=PAD_LOCK;return ret;}return 0;}

/* ------------------------------------------------------------------------ */

#endif // __CONTROL_H 
