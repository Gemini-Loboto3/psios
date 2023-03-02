#include "..\core.h"

//extern long _spu_getInTransfer();
//extern void _spu_setInTransfer(long in);

static int snd_mode=SND_STEREO;

static u_short _svm_rms_count=0;			// total of rms allocated
static u_long _svm_rms_start[RMS_MAX]={0};	// allocation in spu ram
static u_long _svm_rms_total[RMS_MAX]={0};	// size of vag segment
static u_long _svm_rms_vh[RMS_MAX]={0};		// position of headers in ram
static u_char _svm_rms_used[RMS_MAX]={0};	// 0=free, 1=???, 2=in use
static u_long _svm_rms_pg[RMS_MAX]={0};		// total of vags in a rms

//char seq_table[SS_SEQ_TABSIZ*4*5];		// seq data table
static char spu_malloc_rec[SPU_MALLOC_RECSIZ*(SPU_MALLOC_MAX+1)]; 

// rms allocation
static u_char vab_head[4*1024];
//short vab;

void SsInit()
{
	CdlATV	aud;
	u_char result[8];

	aud.val0=127;
	aud.val1=127;
	aud.val2=127;
	aud.val3=127;
	CdControl(CdlDemute,NULL,result);
	CdControlB(CdlSetfilter,NULL,result);
	CdMix(&aud);

	SpuInit();
	SpuInitMalloc(SPU_MALLOC_MAX,spu_malloc_rec);

	SpuSetCommonMasterVolume(0x3fff,0x3fff);
	SpuSetCommonCDMix(SPU_ON);
	SpuSetCommonCDVolume(0x3fff,0x3fff);

	SpuSetReverbModeDepth(0,0);
	SpuSetReverb(SPU_ON);
	SpuSetReverbModeType(SPU_REV_MODE_OFF | SPU_REV_MODE_CLEAR_WA);
	SpuSetReverbModeDepth(0x3FFF,0x3FFF);
	SsClearReverb();

	SpuSetKey(SPU_OFF, SPU_ALLCH);
	SsInitStreaming();
}

void SsQuit()
{
	// reset voices
	SpuSetKey(SPU_OFF, SPU_ALLCH);
	// clear reverb
	SpuSetReverb(SPU_OFF);
	SpuSetReverbModeDepth(0,0);
	SpuSetReverbModeType(SPU_REV_MODE_OFF);
	SsClearReverb();
	
	// close
	SpuStQuit();
	SpuQuit();
}

void SsClearReverb()
{
	// clear last 100K of SPU RAM (i.e. maximum reverb workarea)
	SpuWrite0(102400);
	while(SpuIsTransferCompleted(SPU_TRANSFER_WAIT)==0);
}

// ===========================================================
// misc
// ===========================================================
void SsSetMode(int mode)
{
	snd_mode=mode;
};

// ===========================================================
// allocation
// ===========================================================
int SsRmsLoad(char *filename)
{
	u8 *vh;
	short vab;

	RMS_HEADER *rh=(RMS_HEADER*)FS_Load(filename,ARCHIVE_SYSTEM,NULL);
	zmemcpy(vab_head,(void*)rh,rh->data);
 
	// VAB opening and transmission to sound buffer
	vab=SsRmsOpenHead((u_char*)vab_head,-1);
	if(vab==-1)
	{
		DEBUGPRINT(("RMS header open failed\n", vab));
		return -1;
	}

	DEBUGPRINT(("RMS %d is now open\n",vab));
	if(SsRmsTransferBody((u_char*)rh+rh->data,vab)==-1)
	{
		DEBUGPRINT(("RMS body open failed\n"));
		return -2;
	}
	while(SpuIsTransferCompleted(SPU_TRANSFER_WAIT)==0);

	free3(rh);
	// redefine header pointer
	rh=(RMS_HEADER*)vab_head;
	// reallocate correctly
	vh=(u8*)malloc3(rh->data);
	if(_svm_rms_vh[vab]) free3((void*)_svm_rms_vh[vab]);
	_svm_rms_vh[vab]=(u_long)vh;
	// copy header data
	zmemcpy(vh,(void*)vab_head,rh->data);

	DEBUGPRINT(("Allocated RMS %d at %x-%x (SPURAM)/%x-%x (RAM)\n",vab,
		_svm_rms_start[vab],_svm_rms_start[vab]+rh->size,
		_svm_rms_vh[vab],_svm_rms_vh[vab]+rh->data));

	return vab;
}

int SsRmsFree(int id)
{
	// just in case
	if(id>=0 && id<RMS_MAX)
	{
		if(_svm_rms_used[id])
		{
			DEBUGPRINT(("Releasing RMS %d at %x (SPURAM)/%x (RAM)\n",id,_svm_rms_start[id],_svm_rms_vh[id]));
			// free both ram and spuram
			SpuFree(_svm_rms_start[id]);
			free3((void*)_svm_rms_vh[id]);
			// reset rms entry
			_svm_rms_used[id]=0;
			_svm_rms_start[id]=0;
			_svm_rms_total[id]=0;
			_svm_rms_start[id]=0;
			_svm_rms_vh[id]=0;
			_svm_rms_pg[id]=0;
			// decrease count
			_svm_rms_count--;

			return 1;	// rms correctly released
		}
	}
	return 0;	// nothing released
}

int SsRmsPreAllocate(int size, int mode, u_short id)
{
	long ret_size=SpuMalloc(size);
	if(ret_size==-1)
	{
		_svm_rms_used[id]=0;
		_svm_rms_count--;
		//_spu_setInTransfer(0);
		return -1;
	}
	return ret_size;
}

short _SsRmsOpenHeadWithMode(u_char *addr, short rms_id, int (*func)(int size, int mode, u_short id), int mode)
{
	RMS_HEADER *head;
	int i, found=RMS_MAX, alloc;

	//if(_spu_getInTransfer()==1) return -1;
	//_spu_setInTransfer(1);

	// create entry
	if(rms_id>=0 && rms_id<=RMS_MAX-1)
	{
		// write only if empty
		if(_svm_rms_used[rms_id]==SPU_OFF)
		{
			_svm_rms_used[rms_id]=SPU_ON;
			_svm_rms_count++;
			found=rms_id;
		}
	}
	// allocate automatically
	else if(rms_id==-1)
	{
		// search for a free entry
		for(i=0; i<RMS_MAX; i++)
		{
			// found
			if(_svm_rms_used[i]==SPU_OFF)
			{
				_svm_rms_used[i]=SPU_ON;
				_svm_rms_count++;
				found=i;
				break;
			}
		}
	}
	// check generated id
	if(found>RMS_MAX)
	{
		//_spu_setInTransfer(0);
		return -1;
	}

	//// save rms header address
	//_svm_rms_vh[found]=(u_long)addr;
	//_svm_rms_not_send_size=0;
	// check header integrity
	if(*((u32*)addr)!=RMS_MAGIC)
	{
		_svm_rms_used[found]=0;
		//_spu_setInTransfer(0);
		_svm_rms_count--;
		return -1;
	}
	// copy structure data
	head=(RMS_HEADER*)addr;
	alloc=func(head->size,mode,rms_id);
	if(alloc!=-1)
	{
		_svm_rms_pg[found]=head->count;
		_svm_rms_used[found]=SPU_CLEAR;
		_svm_rms_total[found]=head->size;
		_svm_rms_start[found]=alloc;
	}
	// return rms id
	return found;
}

short SsRmsOpenHead(u_char *addr, short rms_id)
{
	return _SsRmsOpenHeadWithMode(addr,rms_id,&SsRmsPreAllocate,0);
}

short SsRmsTransferBody(u_char *addr, short rms_id)
{
	if(rms_id>RMS_MAX || _svm_rms_used[rms_id]!=2)
	{
		//_spu_setInTransfer(0);
		return -1;
	}

	DEBUGPRINT(("Transferring from %x at %x\n",addr,_svm_rms_start[rms_id]));
	//SpuSetTransferMode(SPU_TRANSFER_BY_DMA);
	if(SpuSetTransferStartAddr(_svm_rms_start[rms_id])==0)
	{
		//_spu_setInTransfer(0);
		return -1;
	}
	SpuWrite(addr,_svm_rms_total[rms_id]);
	_svm_rms_used[rms_id]=1;

	return rms_id;
}

// ===========================================================
// playback
// ===========================================================
int SsRmsPlay(int rms_id, int sample, int voice, int voll, int volr)
{
	RMS_ENTRY* entry=(RMS_ENTRY*)((u_char*)_svm_rms_vh[rms_id]+sizeof(RMS_HEADER));
	if(sample>=_svm_rms_pg[rms_id]) return -1;
	return SsPlaySound(_svm_rms_start[rms_id]+entry[sample].pos,voice,entry[sample].freq,voll,volr);
}

int SsPlaySound(u_long addr, int voice, int pitch, int lvol, int rvol)
{
	if(voice==-1)
	{
		if((voice=SsFindOpenVoice())==-1)
		{
			//DEBUGPRINT(("Cannot allocate voice.\n"));
			return -1;
		}
	}
	else if(SpuGetKeyStatus(1<<voice)==SPU_ON)
	{
		//DEBUGPRINT(("Voice is already in use.\n"));
		return -1;
	}
	//SpuSetIRQ(SPU_ON);
	if(snd_mode==SND_MONO) lvol=rvol=(lvol+rvol)/2;
	//DEBUGPRINT(("Playing sample %x on voice %d, pitch %d\n",addr,voice,pitch));
	SpuSetVoicePitch(voice,pitch);
	SpuSetVoiceVolume(voice,lvol<<7,rvol<<7);
	SpuSetVoiceStartAddr(voice,addr);
	SpuSetReverbVoice(SPU_ON,1<<voice);
	SpuSetKey(SPU_ON,1<<voice);

	return voice;
}

int SsFindOpenVoice()
{
	int i;
	char voice_status[NUM_SFX_VOICES];
	int empty_voice=-1;

	// check status of voices
	SpuGetAllKeysStatus(voice_status);

	// search for unused voice
	for(i=VOICE_DYNAMIC; i<NUM_SFX_VOICES; i++)
	{
		if(voice_status[i]!=SPU_ON) 
		{
			empty_voice=i;
			break;
		}
	}
	return empty_voice;
}
