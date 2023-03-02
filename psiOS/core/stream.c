#include "..\core.h"

static SpuStEnv *st_env;
//static CdlCB st_oldreadycallback=NULL;

static SpuStCallbackProc spustCB_preparation_finished(u_long voice_bit, long p_status);
static SpuStCallbackProc spustCB_transfer_finished(u_long voice_bit, long t_status);
static SpuStCallbackProc spustCB_stream_finished(u_long voice_bit, long s_status);
static void spust_prepare(void);
static long spust_start(u_long voice_bit);
static void spustCB_next(u_long voice_bit);

static volatile u_long st_stat=0;
static volatile u_long st_stop;
#define ST_STOP_INIT_VAL	0

static volatile u_long st_state;
static volatile long st_load_ave=0;
static volatile u_long played_size;		// size to finish
static volatile long track_size;		// total size
//static u_short st_pitch=4096;

static u8 st_buf[2][RAM_BUFSIZE];		// holds data for streaming
//static CdlFILE fmusic;					// current stream data
static CD_FILE *fmusic;					// handle to music data

int SsInitStreaming()
{
	int i;
	u_long buffer_addr[VOICE_LIMIT];
	SpuVoiceAttr s_attr;

	st_env=SpuStInit(0);
	// for finishing SPU streaming preparation
	SpuStSetPreparationFinishedCallback((SpuStCallbackProc)spustCB_preparation_finished);
	// for next transfer
	SpuStSetTransferFinishedCallback((SpuStCallbackProc)spustCB_transfer_finished);
	// for finising SPU streaming with some voices
	SpuStSetStreamFinishedCallback((SpuStCallbackProc)spustCB_stream_finished);

	// allocate left and right voices
	for(i=0; i<VOICE_LIMIT; i++)
	{
		if((buffer_addr[i]=SpuMalloc(SPU_BUFSIZE))==-1) return -1;
		st_env->voice[i].buf_addr=buffer_addr[i];
	}
	st_env->size=SPU_BUFSIZE;

	// set attributes for left channel (voice 0)
	s_attr.mask=(SPU_VOICE_VOLL | SPU_VOICE_VOLR | SPU_VOICE_PITCH | SPU_VOICE_WDSA | SPU_VOICE_ADSR_AMODE |
		SPU_VOICE_ADSR_SMODE | SPU_VOICE_ADSR_RMODE | SPU_VOICE_ADSR_AR | SPU_VOICE_ADSR_DR | SPU_VOICE_ADSR_SR |
		SPU_VOICE_ADSR_RR | SPU_VOICE_ADSR_SL);
	s_attr.volume.left=0x3FFF;
	s_attr.volume.right=0x0;
	s_attr.pitch=4096;	// 44100 Hz
	s_attr.a_mode=SPU_VOICE_LINEARIncN;
	s_attr.s_mode=SPU_VOICE_LINEARIncN;
	s_attr.r_mode=SPU_VOICE_LINEARDecN;
	s_attr.ar=s_attr.dr=s_attr.sr=0x0;
	s_attr.rr=0x3;
	s_attr.sl=0xf;
	s_attr.voice=SPU_VOICECH(0);
	s_attr.addr=buffer_addr[0];
	SpuSetVoiceAttr(&s_attr);
	// set attributes for right channel (voice 1)
	s_attr.volume.left=0x0;
	s_attr.volume.right=0x3FFF;
	s_attr.voice=SPU_VOICECH(1);
	s_attr.addr=buffer_addr[1];
	SpuSetVoiceAttr(&s_attr);

	DEBUGPRINT(("SPU Streaming initialized at %x (RAM)/%x-%x (SPURAM)\n",st_env,buffer_addr[0],buffer_addr[1]));
	return 1;
}

void SsStreamSetPitch(int value)
{
	SpuSetVoicePitch(0,value);
	SpuSetVoicePitch(1,value);
}

void SsStreamSetReverb(int value)
{
	SpuSetReverbModeType(value);
	SpuSetReverbModeDepth(0x3FFF,0x3FFF);

	SpuSetReverbVoice(TRUE,SPU_VOICECH(0));
	SpuSetReverbVoice(TRUE,SPU_VOICECH(1));
}

void SsStreamPause()
{
	//SpuGetVoicePitch(0,&st_pitch);
	//SsStreamSetPitch(0);

	//SpuStTransfer(SPU_ST_IDLE,SPU_VOICECH(0)|SPU_VOICECH(1));
	if(st_state==STREAM_READING)
	{
		DEBUGPRINT(("Still streaming data...\n"));
		CdReadSync(0,NULL);
	}
	st_state=STREAM_PAUSE;
}

void SsStreamResume()
{
	//SpuStTransfer(SPU_ST_PLAY,SPU_VOICECH(0)|SPU_VOICECH(1));
	//SsStreamSetPitch(st_pitch);
	st_state=STREAM_PLAY;
}

void SsStreamStop()
{
	u_char param[4];

	DEBUGPRINT(("Stopping stream\n"));

	if(st_state==STREAM_READING)
	{
		DEBUGPRINT(("Still streaming data...\n"));
		//CdReadSync(0,NULL);
	}

	CdControlF(CdlStop,0);
	/* clear mode... */
	param[0]=CdlModeSpeed;
	CdControlB(CdlSetmode,param,0);

	SpuStTransfer(SPU_ST_STOP,ST_VOICES);
	st_stop=ST_VOICES;
	st_state=STREAM_STOP;
}

int SsSetStream(int index)
{
	ARCHIVE_HEADER *head;
	ARCHIVE_ENTRY *entry;
	extern FS_ALLOC fs_alloc[];
	char name[sizeof(entry->name)+1];

	head=(ARCHIVE_HEADER*)fs_alloc[ARCHIVE_SOUND].data;
	entry=(ARCHIVE_ENTRY*)(fs_alloc[ARCHIVE_SOUND].data+sizeof(ARCHIVE_HEADER));

	// seek
	//CdIntToPos(fs_alloc[ARCHIVE_SOUND].pos+entry[index].pos,&fmusic.pos);
	//CdControl(CdlSetloc,(u8*)&(fmusic),0);
	CdFileClose(fmusic);
	zmemcpy(name,entry[index].name,sizeof(entry->name));
	fmusic=CdFileFSOpen(name,ARCHIVE_SOUND);
	// fill the sound buffer
	//_read1(RAM_BUFSIZE*2,st_buf,CdlModeSpeed);
	CdFileRead(st_buf,RAM_BUFSIZE,2,fmusic);
	// set size of the stream
	//track_size=(entry[index+1].pos-entry[index].pos)*SECTOR;
	track_size=fmusic->size;
	spust_start(ST_VOICES);
	//SpuSetReverbVoice(SPU_ON,ST_VOICES);
	st_env->voice[0].status=SPU_ST_PLAY;
	st_env->voice[1].status=SPU_ST_PLAY;

	DEBUGPRINT(("Streaming %s, size %d\n",entry[index].name,track_size));
	track_size-=RAM_BUFSIZE*2;
	return 1;
}

static void spust_prepare(void)
{
    register long i;

    for(i=0; i<VOICE_LIMIT; i++)
	{
		st_env->voice[i].data_addr=(u_long)st_buf[i];
		st_env->voice[i].status=SPU_ST_PLAY;
    }
    //played_size=0;
	played_size=SPU_BUFSIZEHALF;

    st_state=STREAM_START;
	st_stop=ST_STOP_INIT_VAL;
    st_stat=SPU_ST_NOT_AVAILABLE;
    st_load_ave=0;
}

static long spust_start(u_long voice_bit)
{
	if(st_stat==SPU_ST_NOT_AVAILABLE) spust_prepare();
	st_stat|=voice_bit;

	return SpuStTransfer(SPU_ST_PREPARE,voice_bit);
}

static void spustCB_next(u_long voice_bit)
{
	register int i;
	u_char param[4];

	DEBUGPRINT(("[next] %d\n",track_size));
	if(/*track_size<=0*/track_size-RAM_BUFSIZE*2<0 && st_state!=STREAM_STOP)
	{
		//track_size=0;
		//st_state=STREAM_STOP;
		//st_stop=SPU_ST_FINAL;
		//CdControlF(CdlPause,NULL);
		///* clear mode... */
		//param[0]=CdlModeSpeed;
		//CdControlB(CdlSetmode,param,NULL);
		//return;
		CdFileSeek(fmusic,0,SEEK_SET);
		track_size=fmusic->size;
	}

	if(played_size<=RAM_BUFSIZE-SPU_BUFSIZEHALF)
	{
		for(i=0; i<VOICE_LIMIT; i++) st_env->voice[i].data_addr+=SPU_BUFSIZEHALF;
		played_size+=SPU_BUFSIZEHALF;
    }
	else
	{
		// return to TOP
		for(i=0; i<VOICE_LIMIT; i++) st_env->voice[i].data_addr=(u_long)st_buf[i];
		played_size=SPU_BUFSIZEHALF;

		//CdIntToPos(CdPosToInt(&fmusic.pos)+(RAM_BUFSIZE*2)/SECTOR,&fmusic.pos);
		//CdControl(CdlSetloc,(u_char*)&fmusic.pos,NULL);
		//_read_async(RAM_BUFSIZE*2,st_buf,CdlModeSpeed);
		CdFileReadAsync(st_buf,RAM_BUFSIZE,2,fmusic);
		st_state=STREAM_READING;
		track_size-=RAM_BUFSIZE*2;
    }

	//played_size=0;
	if(st_stop!=ST_STOP_INIT_VAL)
	{
		for(i=0; i<VOICE_LIMIT; i+=2)
		{
			if(st_stat & SPU_VOICECH(i))
			{
			/* L-ch */
			st_env->voice[i].status      = SPU_ST_STOP;
			st_env->voice[i].last_size   = SPU_BUFSIZEHALF;
			/* R-ch */
			st_env->voice[i+1].status    = SPU_ST_STOP;
			st_env->voice[i+1].last_size = SPU_BUFSIZEHALF;
			}
		}
		st_stat&=~st_stop;
		st_stop=ST_STOP_INIT_VAL;
    }
	////{
	//	// rewind spu buffer
	//	st_env->voice[0].data_addr=(u_long)st_buf[0];
	//	st_env->voice[1].data_addr=(u_long)st_buf[1];
	//	//played_size=0;
	//	//if(st_stop&SPU_VOICECH(0))
	//	//{
	//	//	// L-ch
	//		//st_env->voice[0].status=SPU_ST_STOP;
	//		st_env->voice[0].last_size=SPU_BUFSIZE;
	//	//}
	//	//if(st_stop&SPU_VOICECH(1))
	//	//{
	//	//	// R-ch
	//		//st_env->voice[1].status=SPU_ST_STOP;
	//		st_env->voice[1].last_size=SPU_BUFSIZE;
	//	//}
	//	//st_stat&=~st_stop;
	//	//st_stop=ST_STOP_INIT_VAL;
	////}
	//// read data from disk now
	////else
	////{
	////	DEBUGPRINT(("Reading... %d\n",played_size));
	////	_read_async(RAM_BUFSIZE*2,st_buf,CdlModeSpeed);
	////	st_stop=ST_STOP_INIT_VAL;
	////}
}

static SpuStCallbackProc spustCB_preparation_finished(u_long voice_bit, long p_status)
{
	DEBUGPRINT(("preparation: [%06x] [%s]\n",voice_bit,p_status==SPU_ST_PREPARE ? "PREPARE" : "PLAY"));

    if(p_status==SPU_ST_PREPARE) spustCB_next(voice_bit);
    if(st_stat!=SPU_ST_NOT_AVAILABLE) SpuStTransfer(SPU_ST_START,voice_bit);
}

static SpuStCallbackProc spustCB_transfer_finished(u_long voice_bit, long t_status)
{
    // t_status is always SPU_ST_PLAY
    spustCB_next(voice_bit);
}

static SpuStCallbackProc spustCB_stream_finished(u_long voice_bit, long s_status)
{
	DEBUGPRINT(("stop: [%06x] [%s]\n",voice_bit,s_status == SPU_ST_PLAY ? "PLAY" : "FINAL"));

	SpuSetKey(SPU_OFF,voice_bit);
}