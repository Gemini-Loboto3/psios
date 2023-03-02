/* ---------------------------------------------------------------------------
 * - (C) Sony Computer Entertainment. All Rights Reserved.
 * -
 * - Project:	Movie Player V2.0	
 * -
 * - Name:		control.c
 * -
 * - Author:	Vince Diesi
 * -
 * - Date:		13th Feb 1996
 * -
 * - Description:
 * - ------------
 * - Controller functions. Only supports the standard controller. 
 * ---------------------------------------------------------------------------
 */
#include <sys/types.h>
#include <kernel.h>
#include <libpad.h>
#include <libapi.h>

#include "ctrller.h"
#include "control.h"
#include "common.h"

/* ---------------------------------------------------------------------------
 * - CONSTANTS 
 * ---------------------------------------------------------------------------
 */

// Define for timing and testing functions.
// #define TESTING


// Controllers connected.
#define NO_PADS			0
#define PAD_ONE			1
#define PAD_TWO			2
#define BOTH_PADS		3

#define	DS_PAD_1		0		//port id for pad 1
#define	DS_PAD_2		1		//port id for pad 2

/* ---------------------------------------------------------------------------
 * - GLOBAL DEFINITIONS 
 * ---------------------------------------------------------------------------
 */

volatile int pad_lock=0;

volatile short __connected = 0;			// No. controllers connected.
volatile short __currController = 0;		// Current active controller.

volatile ControllerPacket __buffers[2];

#define vibBuffLength	8
u_char transmissionBuffer[2][vibBuffLength];	//vibration buffers for pads

/* ---------------------------------------------------------------------------
 * - FUNCTION DEFINITIONS
 * ---------------------------------------------------------------------------
 */
void InitControllers(void)
{
/* - Type:	PUBLIC 
 * -
 * - Usage: Init controllers. 
 */
	InitPAD((char *) &__buffers[0], MAX_CONTROLLER_BYTES, (char *) &__buffers[1], MAX_CONTROLLER_BYTES);
	StartPAD();
	ChangeClearPAD(0);

	VSync(0);
	CheckControllers();
}

/* ------------------------------------------------------------------------ */
void StopControllers(void)
{
/* - Type:	PUBLIC 
 * -
 * - Usage: Stop controllers. 
 */
	StopPAD();
	__connected = 0;
	__currController = 0;
}

/* ------------------------------------------------------------------------ */
void CheckControllers(void)
{
/* - Type:	PUBLIC 
 * -
 * - Usage:	Check number of controllers connected (stored in connected).
 * - 		Also selects the active controller (stored in currController). 
 * - 		Should be called in the VSyncCallback() to constantly check the
 * -		controller status.
 */
	__connected=0;

	if(GoodData(&__buffers[0]) && (GetType(&__buffers[0])==STD_PAD || GetType(&__buffers[0])==ANALOG_PAD)) __connected|=PAD_ONE;
	if(GoodData(&__buffers[1]) && (GetType(&__buffers[1])==STD_PAD || GetType(&__buffers[1])==ANALOG_PAD)) __connected|=PAD_TWO;
	if(__connected==BOTH_PADS || __connected==PAD_ONE) __currController=0;
	else if (__connected==PAD_TWO) __currController=1;
}

short PressedLock(short button)
{
	if(Pressed(button) && !pad_lock)
	{
		pad_lock=PAD_LOCK;
		return 1;
	}
	return 0;
}

/* ------------------------------------------------------------------------ */
void GetStablePad(int port)
{
	//this is just a simple loop that keeps going till both pads are in a stable state
	while (PadGetState(port)!=PadStateStable);	//loop till both pads are in a stable state
}
