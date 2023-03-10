/*+--------+----------+----------+----------+----------+----------+---------+*/
/*                                                                           */
/* (C) Sony Computer Entertainment. All Rights Reserved.                     */
/*                                                                           */
/* Name:   ctrller.h                                                         */                      
/* Author: Allan J. Murphy                                                   */                     
/* Created:   9th February 1995.                                                */                       
/*                                                                           */
/* Description:                                                              */            
/*                                                                           */
/* An attempt to build a tidy header for accessing the pads, after SCE       */
/* Japan reversed the bytes in the packet buffer; and the bit organisation   */
/* to 1 = not pressed, 0 = pressed (for pads at least).                      */
/* This header includes macros to make it easier to deal with the raw        */
/* packet data, which you have to if you are using anything other than a     */
/* plain pad.                                                                */
/*                                                                           */
/* Updated 20/4/95 to include multi-tap data - not complete or tested.       */
/* Updated 30/5/95 to include mouse data.                                    */
/* Updated 31/5/95 to include negcon data.                                   */
/* Updated 12/6/95 to include tested multi-tap data.                         */
/*
** 20-nov-1995 (pholman)
**	    Added Analog Joystick
**                                                                           **
**+--------+----------+----------+----------+----------+----------+---------+*/

#ifndef __CTRLLER_H_
#define __CTRLLER_H_

/*+--------+----------+----------+----------+----------+----------+---------+*/

/* #defines for recognising the contents of the controller packet.           */

#define BAD_TRANSMISSION         0xff       /* No pad or bad transmission. */
#define NOKEY                    0xffff		/* No digital key pressed      */


#define MAX_CONTROLLER_BYTES    34          /* Size of the biggest packet  */
                                            /* you can possibly have.      */

#define STD_PAD_DATA            1           /* Defines for the amount of   */
#define MOUSE_DATA              2           /* data controllers return.    */   
#define NEGCON_DATA             3
#define ANALOG_JOY_DATA			8
#define MULTI_TAP_DATA          16

#define MOUSE                   1           // SCPH-1030
#define NEGCON                  2           // SLPH-00001 (Namco Ltd)
#define GUN_PAD					3			// SLPH-00014 (Konami Ltd)
#define STD_PAD                 4			// SCPH-1080, 1150, 1200 (Standard Pad)
#define ANALOG_JOY			    5			// SCPH-1110 (Analog Joystick)
#define	GUNCON					6			// SLPH-00034 (Namco Ltd)
#define ANALOG_PAD				7			// SCPH-1150, 1200 (Dual Shock)
#define MULTI_TAP               8			// SCPH-1070

/*+--------+----------+----------+----------+----------+----------+---------+*/
/* Pad controller data structure. */

/* The data returned by a pad is just a short bit mask for each key.         */

typedef unsigned short PadData;

/* Pad Controller Macros and masks. */

/* #defines for the keys on the pad. The best way to use these is to do      */
/* for example,  if (!(buffer1.data.pad & PAD_SEL)) { <handle select key> }  */
/* Or use the PadKeyIsPressed(&buffer,PAD_SEL).                              */

/* Front or shoulder buttons - F for front, L/R for left/right, T/B for      */
/* top or bottom, U/D for up/down.                                           */

#define PAD_FRB                  0x0200
#define PAD_FLB                  0x0100
#define PAD_FRT                  0x0800
#define PAD_FLT                  0x0400

/* Right directional;  RU = right up, RD = right down, and so on.            */
     
#define PAD_RU                   0x1000
#define PAD_RD                   0x4000
#define PAD_RL                   0x8000
#define PAD_RR                   0x2000

/* Left directional;  LU = left up, LD = left down, and so on.               */

#define PAD_LU                   0x0010
#define PAD_LD                   0x0040
#define PAD_LL                   0x0080
#define PAD_LR                   0x0020

/* The other two keys.                                                       */

#define PAD_SEL                  0x0001
#define PAD_START                0x0008

/* Controller packet processing (pass in address of packet) for pads.        */

#define PadKeyIsPressed(x,y)      (!((x)->data.pad & (y)))

#define PadKeyPressed(x)          ((x)->data.pad != NOKEY)
#define NoPadKeyPressed(x)        ((x)->data.pad == NOKEY)

#define PAD_LOCK	10

/*+--------+----------+----------+----------+----------+----------+---------+*/
/* Mouse data packet returns. You can use the routines from mouse.obj to     */
/* work with the mouse packets if you like, but its not that tough to deal   */
/* with the raw data yourself by working with the x and y offsets.           */

typedef struct MouseData
  {
     unsigned short buttons;
     signed char xOffset;
     signed char yOffset;
  }
MouseData;

/* Mouse Controller macros and masks. */

#define MOUSE_LEFT               0x0800               /* Left Mouse button.  */
#define MOUSE_RIGHT              0x0400               /* Right Mouse button. */

#define MOUSE_NOKEY              0xfcff               /* No buttons pressed. */

/* Mouse packet handling macros (pass address of packet).                    */

#define MouseXOffset(x)          ((x)->data.mouse.xOffset)
#define MouseYOffset(x)          ((x)->data.mouse.yOffset)

#define MouseMoving(x)           ((x)->data.mouse.xOffset || \
                                  (x)->data.mouse.yOffset)

#define MouseKeyIsPressed(x,y)   (!((x)->data.mouse.buttons & (y)))

#define MouseKeyPressed(x)       ((x)->data.mouse.buttons != MOUSE_NOKEY)
#define NoMouseKeyPressed(x)     ((x)->data.mouse.buttons == MOUSE_NOKEY)

/*+--------+----------+----------+----------+----------+----------+---------+*/
/* Negcon controller packet structure. The Negcon has several different      */
/* buttons / analogue inputs; there is the 4 way directional pad on the left */
/* of the controller, the start button on the left, the B and A buttons on   */
/* the right, two analogue buttons (I and II), the central twisting analogue */
/* input, and two top buttons - the top left is analogue, the top right is   */
/* digital (strangely enough). Bit masks and locations are given for these.  */

typedef struct NegconData
{
	unsigned short digitalButtons;               /* Bit mask of plain keys. */
	char centralTwist;                           /* Analogue twisting thing.*/
	char buttonI;                                /* The I analogue button.  */
	char buttonII;                               /* The II analogue button. */
	char topLeft;                                /* The analogue tl button. */
}
NegconData;

/* Negcon handling macros and masks. */
                                    
#define NEGCON_UP                0x0010
#define NEGCON_RIGHT             0x0020
#define NEGCON_DOWN              0x0040
#define NEGCON_LEFT              0x0080

#define NEGCON_START             0x0008
#define NEGCON_A                 0x2000                                    
#define NEGCON_B                 0x1000                                    
#define NEGCON_TR                0x0800

#define TZP_UPPER                128
#define TZP_LOWER                125
                                    
#define NegconKeyIsPressed(x,y)   (!((x)->data.negcon.digitalButtons & (y)))

#define NegconKeyPressed(x)       ((x)->data.negcon.digitalButtons != NOKEY)
#define NoNegconKeyPressed(x)     ((x)->data.negcon.digitalButtons == NOKEY)

#define NegconTopLeft(x)          ((x)->data.negcon.topLeft)
#define NegconI(x)                ((x)->data.negcon.buttonI)
#define NegconII(x)               ((x)->data.negcon.buttonII)

#define NegconTwisting(x)         ((x)->data.negcon.centralTwist < TZP_LOWER || \
                                   (x)->data.negcon.centralTwist > TZP_UPPER)

#define NegconTwist(x)            ((x)->data.negcon.centralTwist)


/*
** Analog Joystick
*/
typedef struct AnalogjoyData
{
     unsigned short digitalButtons;               /* Bit mask of plain keys. */
     unsigned char  right_x;
     unsigned char  right_y;
     unsigned char  left_x;
     unsigned char  left_y;
} AnalogjoyData;


#define LEFT_MAIN_FIRE		0x0100
#define RIGHT_MAIN_FIRE		0x8000



#define JoystickRightX(x)           ( (x)->data.joystick.right_x)
#define JoystickRightY(x)           ( (x)->data.joystick.right_y)

#define JoystickLeftX(x)            ( (x)->data.joystick.left_x)
#define JoystickLeftY(x)            ( (x)->data.joystick.left_y)


#define JoystickKeyIsPressed(x,y)   (!((x)->data.joystick.digitalButtons & (y)))
#define JoystickKeyPressed(x)       ((x)->data.joystick.digitalButtons != NOKEY)
#define NojoystickKeyPressed(x)     ((x)->data.joystick.digitalButtons == NOKEY)


 /*+--------+----------+----------+----------+----------+----------+---------+*/
/* The data returned by the multi-tap is a set of controller returns.        */
/* Each of the controllers connected to the multi-tap can return up to 6     */
/* bytes of information; each of the controllers returns its own packet.     */
/* This means you can have 4 pads or NegCons connected, but not 4 mice, and  */
/* not 4 multi-taps (surprisingly). The packet for each of the multi-tap     */
/* controllers is just the same as the packet for a plain controller apart   */
/* from this, so you can handle the controller packet from the multi-tap     */
/* with the same macros as for the normal controller packets.                */

typedef struct TapCtrllerData
  {
     unsigned char transStatus;  /* 0xff = no pad, bad pad, bad transmission */
     unsigned char dataFormat;            /* Top 4 bits = type of controller */

     union                                         /* Controller data union. */                   /* Controller data union. */
      {
         PadData    pad;                                 /* Plain pad.       */
         NegconData negcon;                              /* Namco controller.*/
      }
     data;
  }
TapCtrllerData;

/* Now the multi-tap return structure, which is an array of the above.       */

typedef struct MultiTapData
  {
     TapCtrllerData ctrllers[4];               /* Just 4 controller packets. */
  }
MultiTapData;


#define GetTapData(x,y)           (&((x)->data.tap.ctrllers[(y)]))

/*+--------+----------+----------+----------+----------+----------+---------+*/
/* Controller packet structure. */

typedef struct ControllerPacket
{
	unsigned char transStatus;  /* 0xff = no pad, bad pad, bad transmission */
	unsigned char dataFormat;            /* Top 4 bits = type of controller */
                                       /* Bottom 4 == shorts of data written */
	union                                         /* Controller data union. */
	{
         PadData       pad;                     /* Plain pad.       */
         MouseData     mouse;                   /* Mouse.           */
         NegconData    negcon;                  /* Namco controller.*/
		 AnalogjoyData joystick;				/* Anlog Joystick   */
         MultiTapData  tap;                     /* 4-way multi-tap. */
	}
	data;
} ControllerPacket;

/* Packet macros. */

/* Macros for handling the controller packets; pass in the packet address.   */


#define GoodData(x) ((x)->transStatus != 0xff)

#define GetType(x) (((x)->dataFormat & 0xf0) >> 4)

#define DataShortsReturned(x) ((x)->dataFormat & 0xf)

#define DataBytesReturned(x)  (((x)->dataFormat & 0xf) << 1 )

#define ControllerDataAddress(x) (&((x)->dataFormat)+1)

/*+--------+----------+----------+----------+----------+----------+---------+*/

#endif  

/*+--------+----------+----------+----------+----------+----------+---------+*/
/* End ctrller.h                                                             */
/*+--------+----------+----------+----------+----------+----------+---------+*/
