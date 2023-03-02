/* ---------------------------------------------------------------------------
 * - (C) No!Maki. All Rights Reserved.
 * -
 * - Project:	PSiOS
 * -
 * - Name:		main.h
 * -
 * - Author:	Gemini
 * -
 * - Date:		9 Feb 2012
 * ---------------------------------------------------------------------------*/
#include "core.h"

/* ---------------------------------------------------------------------------
 * - PRIVATE FUNCTION PROTOTYPES
 * ---------------------------------------------------------------------------
 */
void InitSys();
void SndInit(void);
void SndShutDown(void);
void CloseSys(void);

void VSyncCB(void);
void InitKernel(void);

void ClearFBuffer(short x, short y, short w, short h);

#include "readpng.h"
//GFX_MAPPING bg_pict[2]=
//{
////	x		y	u	v	w	h	clut			tage
//	{0,		0,	0,	0,	256,240,getClut(320,0),	getTPage(2,0,320,0)},
//	{256,	0,	0,	0,	64,	240,getClut(320,0),	getTPage(2,0,320+256,0)}
//};
//
//GFX_MAPPING ovr_pict[]=
//{
////	x	y	u	v	w	h	clut				tage
//	0,	0,	0,	1,	128,128,getClut(320,256),	getTPage(0,0,320,256),
//	0,	0,	0,	1,	64,	64,	getClut(640,0),		getTPage(0,0,640,0),
//	0,	0,	0,	0,	128,128,0,					getTPage(2,0,704,0)
//};

extern void init_test();
extern void draw_test();

/* ---------------------------------------------------------------------------
 * - MAIN FUNCTION
 * ---------------------------------------------------------------------------
 */
int main()
{
	extern void *HiRom;
	u32 heap_base[4];
	//u8* png_data;
	//char str[256];
	//GsPngImage img;
	//RECT rect;

	DEBUGPRINT(("--------------------\n"));
	DEBUGPRINT(("Heap position %x, size %d KB\n",(u32)heap_base,(STACK-(long)HiRom)/1024));
	InitHeap3((u32*)heap_base,STACK-(u32)heap_base);
	InitGfxAlloc(128*1024);
	InitSys();

	init_test();
	draw_test();

	//png_data=(u8*)CdLoadFile("CELL.PNG",NULL);
	//load_png(png_data,0,&img,MASK_DEFAULT);
	//free3(png_data);

	////setRECT(&rect,320,0,256,1);
	////LoadImage(&rect,(u_long*)img.clut);
	//setRECT(&rect,320,0,img.w,img.h);
	//LoadImage(&rect,(u_long*)img.pixel);

	//png_data=(u8*)CdLoadFile("4BIT.PNG",NULL);
	//load_png(png_data,3661,&img,MASK_DEFAULT);
	//free3(png_data);

	//setRECT(&rect,320,256,16,1);
	//LoadImage(&rect,(u_long*)img.clut);
	//setRECT(&rect,320,257,32,128);
	//LoadImage(&rect,(u_long*)img.pixel);

	//png_data=(u8*)CdLoadFile("2BIT.PNG",NULL);
	//load_png(png_data,3589,&img,MASK_BLACK_OPAQUE);
	//free3(png_data);

	//setRECT(&rect,640,0,2,1);
	//LoadImage(&rect,(u_long*)img.clut);
	//setRECT(&rect,640,1,img.w,img.h);
	//LoadImage(&rect,(u_long*)img.pixel);

	//png_data=(u8*)CdLoadFile("24BIT.PNG",NULL);
	//load_png(png_data,89701,&img,MASK_COLOR_OPAQUE);
	//free3(png_data);

	//setRECT(&rect,704,0,320,img.h);
	//LoadImage(&rect,(u_long*)img.pixel);

	while(1)
	{
		BeginDraw();

		//SortMapSprt(128,128,&ovr_pict[2],GetOTag());
		//SortMapSprt(0,0,&ovr_pict[1],GetOTag());
		//SortMapSprt((FRAME_X/2)-64,(FRAME_Y/2)-64,&ovr_pict[0],GetOTag());
		//SortMapSprt(0,0,&bg_pict[0],GetOTag());
		//SortMapSprt(0,0,&bg_pict[1],GetOTag());

		EndDraw(0,128,128);
	}
}

/* ------------------------------------------------------------------------ */
// common pad variables
volatile short pad_confirm=X_KEY;				// confirm
volatile short pad_cancel=CIRCLE_KEY;			// cancel

char message[2048];

// globals for display envoirment
//volatile GsOT WorldOrderingTable[2];
//GsOT_TAG zSortTable[2][1<<OT_LENGTH];
//volatile PACKET GpuOutputPacket[2][PACKETMAX2];
//volatile int outputBufferIndex;

/* ---------------------------------------------------------------------------
 * - FUNCTION DEFINITIONS
 * ---------------------------------------------------------------------------
 */
void ClearFBuffer(short x, short y, short w, short h)
{
	RECT rect;
	rect.x=x;
	rect.y=y;
	rect.w=w;
	rect.h=h;
	ClearImage(&rect,0,0,0);
	DrawSync(0);
}

// ------------------------------------------------------------------------
void InitSys()
{
	extern _GsPOSITION POSITION;
	extern DRAWENV GsDRAWENV;
	extern DISPENV GsDISPENV;

	ResetCallback();
	//InitializeCardAndEvents(0);
	//InitThreads();
	CdInit();
	//FS_Init();

	ResetGraph(0);
	SetGraphDebug(0);

	InitControllers();

	// sound shit
	//SsInit();
	//menu_vab=SsRmsLoad("MENU.RMS");

	SetVideoMode(MODE_NTSC);
	GsInitGraph(FRAME_X,FRAME_Y,GsOFSGPU|GsNONINTER, 1, 0);
	GsDefDispBuff(0,0,0,FRAME_Y);

	WorldOrderingTable[0].length=OT_LENGTH;
	WorldOrderingTable[0].org=zSortTable[0];
	WorldOrderingTable[1].length=OT_LENGTH;
	WorldOrderingTable[1].org=zSortTable[1];

	//Init3D();

	ClearFBuffer(0,0,1024,768);	// clear all the bios textures
	VSyncCallback((void (*)()) VSyncCB);
}

// ------------------------------------------------------------------------
void CloseSys(void)
{
	VSyncCallback(NULL);
	//SsQuit();
	StopCallback();
	//StopControllers();
	ResetGraph(3);
}

// ------------------------------------------------------------------------
void VSyncCB(void)
{
	extern volatile int pad_lock;

	pad_lock--;
	if(pad_lock<0) pad_lock=0;
}
