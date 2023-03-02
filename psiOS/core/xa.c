#include <sys/types.h>
#include <sys/file.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <libcd.h>
#include "xa.h"

#define XA_TOTAL	5

char *xa_file[XA_TOTAL]=
{
	"INTRO.XA",
	"STAGE0.XA",
	"STAGE1.XA",
	"STAGE2.XA",
	"EXTRA.XA"
};

XA_TRACK xa_track[XA_TOTAL]=
{
	{0,1278},
	{0,1138}
};

// XA specific data
static int StartPos, EndPos;
static int CurPos, XAAddon, XAChan;

void cbready(u_char intr, u_char *result);
static cdplay(u_char com);

static int gPlaying=0;
CdlCB oldreadycallback=NULL;

void PrepareXA(void)
{
	int i;
	u_char param[4];
	CdlFILE fp;
	char tempstr[64];

	// build XA tables
	for(i=0; i<1/*XA_TOTAL*/; i++)
	{
		sprintf(tempstr,"\\XA\\%s;1",xa_file[i]);
		while(CdSearchFile(&fp, tempstr)==0);// DEBUGPRINT(("%s: not found\n", tempstr));
		xa_track[0].start=CdPosToInt(&fp.pos);
		xa_track[0].end=CdPosToInt(&fp.pos)+xa_track[0].end*8;
		xa_track[1].start=CdPosToInt(&fp.pos);
		xa_track[1].end=CdPosToInt(&fp.pos)+xa_track[1].end*8;
	}

	/* setup for XA playback... */
	param[0] = CdlModeSpeed|CdlModeRT|CdlModeSF;
	CdControlB(CdlSetmode, param, 0);
	CdControlF(CdlPause,0);
	oldreadycallback=CdReadyCallback(cbready);
}

int StartXA(int startp, int endp, int index, int addon)
{
	CdlFILTER filt;

	StartPos = startp;
	EndPos   = endp;
	CurPos   = StartPos;
	XAAddon  = addon;

	filt.file=1;
	filt.chan=XAChan=index;

	CdControlF(CdlSetfilter, (u_char *)&filt);

	cdplay(CdlReadN);
	gPlaying=1;

	return 0;
}

int PlayingXA(void)
{
	return gPlaying;
}

void UnprepareXA(int mode)
{
	u_char param[4];

	CdReadyCallback((void *)oldreadycallback);
	CdControlF(mode,0);

	/* clear mode... */
	param[0] = CdlModeSpeed;
	CdControlB(CdlSetmode, param, 0);
}

void PauseXA(void)
{
	UnprepareXA(CdlPause);
	gPlaying=0;
}

void StopXA(void)
{
	UnprepareXA(CdlStop);
	gPlaying=0;
}

void ResumeXA(void)
{
	// set cd mode for XA reading
	u_char param[4];
	param[0] = CdlModeSpeed|CdlModeRT|CdlModeSF;
	CdControlB(CdlSetmode, param, 0);
	CdControlF(CdlPause,0);
	oldreadycallback=CdReadyCallback(cbready);

	if(gPlaying==0) StartXA(CurPos,EndPos,XAChan,XAAddon);
}

void cbready(u_char intr, u_char *result)
{
	if (intr == CdlDataReady)
	{
		CurPos+=XAAddon;
		if (CurPos > EndPos || CurPos < StartPos)
		{
			// loop
			StartXA(StartPos,EndPos,XAChan,XAAddon);
			//CdControlF(CdlPause,0);
			//gPlaying=0;
		}
	}
}

static cdplay(u_char com)
{
	CdlLOC  loc;

	CdIntToPos(StartPos, &loc);
	CdControlF(com, (u_char *)&loc);
	CurPos = StartPos;
}

void PlayXA(int group, int index)
{
	StartXA(xa_track[group].start,xa_track[group].end,index,XA_SIZE);
}
