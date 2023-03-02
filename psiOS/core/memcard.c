#include "core.h"

// player data
extern CHAR_DATA char_data;
extern USER_DATA user_data;
extern MISC_DATA misc_data;
extern SPEC_DATA spec_data;

extern volatile int playtime;
extern volatile int money;
extern volatile int kills;
extern volatile int rooms;
extern volatile short pad_confirm;
extern volatile short pad_cancel;
extern volatile short pad_menu;
extern volatile short pad_jump;
extern volatile short pad_atk1;
extern volatile short pad_atk2;
extern volatile short pad_slide;
extern volatile short pad_map;

extern u32 save_icon1[];				// tim with save icon

// set up the memory card file headers
struct DIRENTRY dirEntries[15];	// for reading the cards
MC_FILE_HEADER fileHeader[15];	// header+icon
_CARD fastHeader[15];			// just header

// some variables for use with the memory cards
static long ev0,ev1,ev2,ev3,ev10,ev11,ev12,ev13;
long port;
u16 usage;

// async i/o variables
static int async_file;			// file handle
static u8* async_buffer;		// destination
static int async_read;			// list mode: current block header to read
static int async_read_cnt;		// list mode: count of blocks
static int async_channel;		// list mode: port to use
static struct DIRENTRY *async_d;// list mode: directory
static u8 async_name[20];		// list mode: filter
static int async_wait;			// list mode: freeze screen when it's 0
static int card_operation;		// operation to monitor

/************************************
 * GENERIC MEMORY CARD FUNCTIONS	*
 ************************************/
__inline void SetPort(long channel)
{
	port=channel;
}

int GetCardStatus(long channel)
{
	int ret;

	// as the memory cards read on a VSync port 1 then 2,1,2,1,2 etc
	VSync(3);
	ClearCardEventsSw();
	_card_info(channel);
	ret=GetCardEventSw();

	switch(ret)
	{
	case EVENT_IOE:
		return PRESENT_CARD_FORMATTED;		// change to PRESENT_CARD_FORMATTED = 0
	case EVENT_ERROR:
		return PRESENT_CARD_BAD;			// change to PRESENT_CARD_BAD = 1
	case EVENT_TIMEOUT:
		return PRESENT_CARD_NONE;			// change to PRESENT_CARD_NONE = 2
	}

	// if the card is newly inserted then use the code below
	ClearCardEventsHw();
	_card_clear (channel);
	ret=GetCardEventHw();

	ClearCardEventsSw();
	_card_load(channel);
	ret=GetCardEventSw();
	switch(ret) 
	{
	case EVENT_IOE:
		return NEW_CARD_FORMATTED;
	case EVENT_ERROR:
		return NEW_CARD_BAD;
	case EVENT_TIMEOUT:
		return NEW_CARD_NONE;
	case EVENT_NEWCARD:
		return NEW_CARD_UNFORMATTED;
	}
	return -1;
}

int FormatCard(long channel)
{
	int i;
	char drive[16];

	sprintf(drive,"bu%.2X:",channel);
	DEBUGPRINT(("Formatting card %s\n",drive));
	for(i=0; i<15; i++) fileHeader[i].blockEntry=0;
	return format(drive);
}

int InitializeCard(long channel)
{
	long i;
	char buffer[128] ;

	// set up a 128 byte buffer of absolutey nothing
	memset2(buffer,0xFF,128);
	// write NULL'd to memory card 1024 times
	for(i=0; i<1024; i++) 
	{
		// print this on screen to prove the code has not stopped
		DEBUGPRINT(("This will take about a minute - %d of 1024    \r",i));

		ClearCardEventsHw();
		_new_card();
		_card_write(channel,i,&buffer[0]);

		if(GetCardEventHw()!=0) return 0;
	}
	for(i=0; i<15; i++) fileHeader[i].blockEntry=0;

	return 1;
}

int QuickInitializeCard(long channel)
{
	int i;
	char buffer[128];

	buffer[0]=buffer[1]=0xff;
	ClearCardEventsHw();
	_new_card();
	_card_write(channel,0,&buffer[0]);

	if(GetCardEventHw()!=0) return 0;
	for(i=0; i<15; i++) fileHeader[i].blockEntry=0;

	return 1;
}

int DeleteFileFromCard(long channel, char *fileName) 
{
  char path[32];

  // set the file to be deleted
  sprintf(path,"bu%.2X:%s",channel, fileName);
  return erase(path);
}

int LoadFileFromCardAsync(long channel, char *fileName, void* dest, int blocks)
{
	char path[32];

	sprintf(path,"bu%.2X:%s",channel,fileName);
	DEBUGPRINT(("Loading %s\n",path));

	async_file=open(path,O_RDONLY|O_NOWAIT);
	if(async_file<0)
	{
		DEBUGPRINT(("File %s could not be opened\n",path));
		return ERRLOAD_OPEN;	// file count not be opened
	}
	async_buffer=(u8*)dest;

	ClearCardEventsSw();
	read(async_file,dest,blocks*SINGLE_BLOCK);

	card_operation=CARDOP_READ;
	return ERR_NONE;
}

int LoadFileFromCard(long channel, char *fileName, void* dest, int blocks)
{
	int file;
	char path[32];
	int count;

	sprintf(path,"bu%.2X:%s",channel,fileName);
	DEBUGPRINT(("Reading %s\n",path));

	file=open(path,O_RDONLY);
	if(file<0)
	{
		DEBUGPRINT(("File %s could not be opened\n",path));
		return ERRLOAD_OPEN;	// file count not be opened
	}

	count=read(file,dest,blocks*SINGLE_BLOCK);
	if(count!=blocks*SINGLE_BLOCK)
	{
		DEBUGPRINT(("Error during read\n"));
		close(file);
		return ERRLOAD_READ;	// error during read
	}

	DEBUGPRINT(("Read %d bytes from %s\n",blocks*SINGLE_BLOCK,path));
	close(file);
	return ERR_NONE;		// success
}

int SaveFileToCardAsync(long channel, char *fileName, void* buffer, int blocks)
{
	char path[32];

	sprintf(path,"bu%.2X:%s",channel,fileName);

	if(FileExistsOnCard(path)!=1)
	{
		DEBUGPRINT(("Creating %s.\n", path));
		async_file=open(path,O_CREAT|(blocks<<16));
		if(async_file<0)
		{
			DEBUGPRINT(("Error: could not create file %s.\n",path));
			return ERRSAVE_CREATE;
		}
		close(async_file);
	}
	else DEBUGPRINT(("Overwriting %s.\n", path));

	async_file=open(path,O_WRONLY|O_NOWAIT);
	DEBUGPRINT(("Writing.\n"));
	if(async_file<0)
	{
		DEBUGPRINT(("Error: could not write file %s\n",path));
		return ERRSAVE_WRITE;
	}

	ClearCardEventsSw();
	write(async_file,buffer,blocks*SINGLE_BLOCK);

	card_operation=CARDOP_WRITE;
	return ERR_NONE;		// success
}
int SaveFileToCard(long channel, char *fileName, void* buffer, int blocks)
{
	int file;
	char path[32];

	sprintf(path,"bu%.2X:%s",channel,fileName);

	if(FileExistsOnCard(path)!=1)
	{
		DEBUGPRINT(("Creating %s.\n", path));
		file=open(path,O_CREAT|(blocks<<16));
		if(async_file<0)
		{
			DEBUGPRINT(("Error: could not create file %s.\n",path));
			return ERRSAVE_CREATE;
		}
		close(file);
	}
	else DEBUGPRINT(("Overwriting %s.\n", path));

	file=open(path,O_WRONLY);
	DEBUGPRINT(("Writing.\n"));
	if(async_file<0)
	{
		DEBUGPRINT(("Error: could not write file %s\n",path));
		return ERRSAVE_WRITE;
	}

	ClearCardEventsSw();
	write(file,buffer,blocks*SINGLE_BLOCK);

	return ERR_NONE;		// success
}

void InitializeCardAndEvents(int shared)
{
	EnterCriticalSection();
	InitCARD(shared);
	ExitCriticalSection();
	StartCARD();
	_bu_init();

	EnterCriticalSection();
	ev0=OpenEvent(SwCARD, EvSpIOE, EvMdNOINTR, NULL) ;
	ev1=OpenEvent(SwCARD, EvSpERROR, EvMdNOINTR, NULL) ;
	ev2=OpenEvent(SwCARD, EvSpTIMOUT, EvMdNOINTR, NULL) ;
	ev3=OpenEvent(SwCARD, EvSpNEW, EvMdNOINTR, NULL) ;
	ev10=OpenEvent(HwCARD, EvSpIOE, EvMdNOINTR, NULL) ;
	ev11=OpenEvent(HwCARD, EvSpERROR, EvMdNOINTR, NULL) ;
	ev12=OpenEvent(HwCARD, EvSpTIMOUT, EvMdNOINTR, NULL) ;
	ev13=OpenEvent(HwCARD, EvSpNEW, EvMdNOINTR, NULL) ;
	ExitCriticalSection();
				   
	EnableEvent(ev0);
	EnableEvent(ev1);
	EnableEvent(ev2);
	EnableEvent(ev3);
	EnableEvent(ev10);
	EnableEvent(ev11);
	EnableEvent(ev12);
	EnableEvent(ev13);

	port=0;
}

// reads the file directory
int ReadMcDirectory(long channel, struct DIRENTRY *d, char *filter)
{
	int i,p;
	char path[32];
	extern struct DIRENTRY *firstfile(), *nextfile();

	//VSync(3);
	ClearCardEventsHw();
	_card_clear(channel);
	GetCardEventHw();

	ClearCardEventsSw();
	_new_card();
	_card_load(channel);
	GetCardEventSw();

	i=0;
	sprintf(path,"bu%.2X:%s",channel,filter);
	DEBUGPRINT(("Scanning %s.\n",path));
	// builds block list
	if(firstfile(path,d)==d)
	{
		do
		{
			i++;
			d++;
		}
		while(nextfile(d)==d);
 	}

	for(p=0; p<i; p++) GetFileHeaderFromCard(channel,dirEntries[p].name,&fileHeader[p],sizeof(MC_FILE_HEADER));
	return i;
}

// reads the file directory
int FastReadMcDirectory(struct DIRENTRY *d, char *filter)
{
	int i,p;
	char key[32];
	extern struct DIRENTRY *firstfile(), *nextfile();

	VSync(3);
	ClearCardEventsHw();
	_card_clear(port);
	GetCardEventHw();

	ClearCardEventsSw();
	_new_card();
	_card_load(port);
	GetCardEventSw();

	i=0;
	sprintf(key,"bu%.2X:%s",port,filter);
	DEBUGPRINT(("Scanning %s\n",key));
	// builds block list
	if(firstfile(key,d)==d)
	{
		do
		{
			i++;
			d++;
		}
		while(nextfile(d)==d);
 	}

	for(p=0; p<i; p++) GetFileHeaderFromCard(port,dirEntries[p].name,&fastHeader[p],sizeof(_CARD));
	return i;
}

int ReadMcDirectoryAsync(long channel, struct DIRENTRY *d, char *filter)
{
	int i;
	char path[32];
	extern struct DIRENTRY *firstfile(), *nextfile();

	VSync(3);
	ClearCardEventsHw();
	_card_clear(channel);
	GetCardEventHw();

	ClearCardEventsSw();
	_new_card();
	_card_load(channel);
	GetCardEventSw();

	i=0;
	sprintf(path,"bu%.2X:%s",channel,filter);
	DEBUGPRINT(("Starting scan of %s\n",path));

	memset2(d,0,sizeof(struct DIRENTRY)*15);
	// builds block list
	if(firstfile(path,d)==d)
	{
		do
		{
			i++;
			d++;
		}
		while(nextfile(d)==d);
 	}

	if(i==0)
	{
		card_operation=DO_NOTHING;
		return LIST_GENERATED;	// card emtpy
	}

	async_read=0;
	async_read_cnt=i;

	card_operation=CARDOP_LIST;
	return LIST_GENERATION;			// success
}

int GetFileHeaderFromCard(long channel, char *fileName, void *fileHeader, int size)
{
	int file;
	char path[64];
	int count;

	sprintf(path,"bu%.2X:%s",channel,fileName);
	DEBUGPRINT(("Reading %s\n",path));

	file=open(path,O_RDONLY);
	if(file<0)
	{
		DEBUGPRINT(("File %s could not be opened\n",path));
		return ERRLOAD_OPEN;	// file count not be opened
	}

	count=read(file,fileHeader,size);
	if(count!=size)
	{
		DEBUGPRINT(("Error during read\n"));
		close(file);
		return ERRLOAD_READ;	// error during read
	}

	close(file);
	return ERR_NONE;		// success
}

int GetFileHeaderFromCardAsync(long channel, char *fileName, void *fileHeader, int size)
{
	char path[64];

	sprintf(path,"bu%.2X:%s",channel,fileName);
	DEBUGPRINT(("Reading header %s (async).\n",path));

	async_file=open(path,O_RDONLY|O_NOWAIT);
	if(async_file<0)
	{
		DEBUGPRINT(("File %s could not be opened.\n",path));
		return ERRLOAD_OPEN;	// file count not be opened
	}

	read(async_file,fileHeader,size);

	return ERR_NONE;		// success
}

int ListMcDirectory(void)
{
	int ret=EVENT_IOE;

	if(async_read>0) ret=GetCardFastEventSw();
	// error
	if(ret==EVENT_ERROR || ret==EVENT_TIMEOUT || ret==EVENT_NEWCARD)
	{
		close(async_file);
		return ERRLOAD_READ;
	}
	// read complete, do next
	if(ret==EVENT_IOE)
	{
		if(async_read>0) close(async_file);
		// read next file
		if(async_read<async_read_cnt)
		{
			DEBUGPRINT(("Reading entry %d\n",async_read));
			ClearCardEventsSw();
			GetFileHeaderFromCardAsync(async_channel,dirEntries[async_read].name,&fileHeader[async_read],sizeof(MC_FILE_HEADER));
			async_read++;
		}
		// nothing more to read
		else
		{
			card_operation=CARDOP_NONE;
			return LIST_GENERATED;
		}
	}
	return LIST_GENERATION;
}

void PrepareReadMcDirectory(long channel, struct DIRENTRY *d, char *filter)
{
	async_wait=2;
	async_channel=channel;
	async_d=d;
	strcpy(async_name,filter);

	memset2(fileHeader,0,sizeof(fileHeader));

	card_operation=CARDOP_PRELIST;
}

int GetCardEventSw(void)
{
	while(1)
	{
		if(TestEvent(ev0)==1) return EVENT_IOE;
		if(TestEvent(ev1)==1) return EVENT_ERROR;
		if(TestEvent(ev2)==1) return EVENT_TIMEOUT;
		if(TestEvent(ev3)==1) return EVENT_NEWCARD;
	}
	return -1;
}

int GetCardFastEventSw(void)
{
	if(TestEvent(ev0)==1) return EVENT_IOE;
	if(TestEvent(ev1)==1) return EVENT_ERROR;
	if(TestEvent(ev2)==1) return EVENT_TIMEOUT;
	if(TestEvent(ev3)==1) return EVENT_NEWCARD;

	return -1;
}

int GetCardEventHw(void)
{
	while(1) 
	{
		if(TestEvent(ev10)==1) return 0;
		if(TestEvent(ev11)==1) return 1;
		if(TestEvent(ev12)==1) return 2;
		if(TestEvent(ev13)==1) return 3;
	}
	return -1;
}

// clears all of the Sw events 
int ClearCardEventsSw(void)
{
	TestEvent(ev0);
	TestEvent(ev1);
	TestEvent(ev2);
	TestEvent(ev3);
}

// clears all of the Hw events
int ClearCardEventsHw()			
{
	TestEvent(ev10);
	TestEvent(ev11);
	TestEvent(ev12);
	TestEvent(ev13);
}

int FileExistsOnCard(char *fileName)
{
	int file;

	file=open(fileName,O_WRONLY);
	if(file>=0)
	{
		close(file);
		return 1;
	}
	close(file);
	return 0;
}

__inline int GetBlocksRead(void)
{
	return async_read;
}

__inline int GetBlockCount(void)
{
	return async_read_cnt;
}
/************************************
 * SJIS CONVERSION					*
 ************************************/

// ASCII code to Shift-JIS code transfer table
static unsigned short ascii_table[3][2] = {
	{0x824f, 0x30},	/* 0-9  */
	{0x8260, 0x41},	/* A-Z  */
	{0x8281, 0x61},	/* a-z  */
};

// ASCII code to Shift-JIS code transfer table (kigou)
static unsigned short ascii_k_table[] = {
	0x8140,		/*   */
	0x8149,		/* ! */
	0x8168,		/* " */
	0x8194,		/* # */
	0x8190,		/* $ */
	0x8193,		/* % */
	0x8195,		/* & */
	0x8166,		/* ' */
	0x8169,		/* ( */
	0x816a,		/* ) */
	0x8196,		/* * */
	0x817b,		/* + */
	0x8143,		/* , */
	0x817c,		/* - */
	0x8144,		/* . */
	0x815e,		/* / */
	0x8146,		/* : */
	0x8147,		/* ; */
	0x8171,		/* < */
	0x8181,		/* = */
	0x8172,		/* > */
	0x8148,		/* ? */
	0x8197,		/* @ */
	0x816d,		/* [ */
	0x818f,		/* \ */
	0x816e,		/* ] */
	0x814f,		/* ^ */
	0x8151,		/* _ */
	0x8165,		/* ` */
	0x816f,		/* { */
	0x8162,		/* | */
	0x8170,		/* } */
	0x8150,		/* ~ */
};

// ASCII code to Shift-JIS code transfer function
u16 AsciiToSjis(char ascii)
{
	int sjis_code;
	int ascii_code;
	char stmp;
	char stmp2;

	stmp2=0;
	ascii_code=ascii;

	if((ascii_code >= 0x20) && (ascii_code <= 0x2f)) stmp2 = 1;				//  !"#$%&'()*+,-./
	else if((ascii_code >= 0x30) && (ascii_code <= 0x39)) stmp = 0;			// 0-9
	else if((ascii_code >= 0x3a) && (ascii_code <= 0x40)) stmp2 = 11;		// :;<=>?@
	else if((ascii_code >= 0x41) && (ascii_code <= 0x5a)) stmp = 1;			// A-Z
	else if((ascii_code >= 0x5b) && (ascii_code <= 0x60)) stmp2 = 37;		// [\]^_`
	else if((ascii_code >= 0x61) && (ascii_code <= 0x7a)) stmp = 2;			// a-z
	else if((ascii_code >= 0x7b) && (ascii_code <= 0x7e)) stmp2 = 63;		// {|}~
	else return AsciiToSjis('?');											// can't find, return '?'

	if(stmp2) sjis_code = ascii_k_table[ascii_code - 0x20 - (stmp2 - 1)];
	else sjis_code = ascii_table[stmp][0] + ascii_code - ascii_table[stmp][1];

	return (u16)((sjis_code>>8)|(sjis_code<<8));
}

/************************************
 * RED MOON SPECIFIC FUNCTIONS		*
 ************************************/
static u8 dec_tbl[256]=
{
	0xFF,0x7F,0xBF,0x3F,0xDF,0x5F,0x9F,0x1F,0xEF,0x6F,0xAF,0x2F,0xCF,0x4F,0x8F,0x0F,
	0xF7,0x77,0xB7,0x37,0xD7,0x57,0x97,0x17,0xE7,0x67,0xA7,0x27,0xC7,0x47,0x87,0x07,
	0xFB,0x7B,0xBB,0x3B,0xDB,0x5B,0x9B,0x1B,0xEB,0x6B,0xAB,0x2B,0xCB,0x4B,0x8B,0x0B,
	0xF3,0x73,0xB3,0x33,0xD3,0x53,0x93,0x13,0xE3,0x63,0xA3,0x23,0xC3,0x43,0x83,0x03,
	0xFD,0x7D,0xBD,0x3D,0xDD,0x5D,0x9D,0x1D,0xED,0x6D,0xAD,0x2D,0xCD,0x4D,0x8D,0x0D,
	0xF5,0x75,0xB5,0x35,0xD5,0x55,0x95,0x15,0xE5,0x65,0xA5,0x25,0xC5,0x45,0x85,0x05,
	0xF9,0x79,0xB9,0x39,0xD9,0x59,0x99,0x19,0xE9,0x69,0xA9,0x29,0xC9,0x49,0x89,0x09,
	0xF1,0x71,0xB1,0x31,0xD1,0x51,0x91,0x11,0xE1,0x61,0xA1,0x21,0xC1,0x41,0x81,0x01,
	0xFE,0x7E,0xBE,0x3E,0xDE,0x5E,0x9E,0x1E,0xEE,0x6E,0xAE,0x2E,0xCE,0x4E,0x8E,0x0E,
	0xF6,0x76,0xB6,0x36,0xD6,0x56,0x96,0x16,0xE6,0x66,0xA6,0x26,0xC6,0x46,0x86,0x06,
	0xFA,0x7A,0xBA,0x3A,0xDA,0x5A,0x9A,0x1A,0xEA,0x6A,0xAA,0x2A,0xCA,0x4A,0x8A,0x0A,
	0xF2,0x72,0xB2,0x32,0xD2,0x52,0x92,0x12,0xE2,0x62,0xA2,0x22,0xC2,0x42,0x82,0x02,
	0xFC,0x7C,0xBC,0x3C,0xDC,0x5C,0x9C,0x1C,0xEC,0x6C,0xAC,0x2C,0xCC,0x4C,0x8C,0x0C,
	0xF4,0x74,0xB4,0x34,0xD4,0x54,0x94,0x14,0xE4,0x64,0xA4,0x24,0xC4,0x44,0x84,0x04,
	0xF8,0x78,0xB8,0x38,0xD8,0x58,0x98,0x18,0xE8,0x68,0xA8,0x28,0xC8,0x48,0x88,0x08,
	0xF0,0x70,0xB0,0x30,0xD0,0x50,0x90,0x10,0xE0,0x60,0xA0,0x20,0xC0,0x40,0x80,0x00
};

int CardMessages()
{
	SAVE_BLOCK* block_sav;

	switch(card_operation)
	{
	case CARDOP_READ:
		switch(GetCardFastEventSw())
		{
		case EVENT_IOE:
			close(async_file);
			card_operation=CARDOP_NONE;
			// control checksum
			block_sav=(SAVE_BLOCK*)async_buffer;
			if(block_sav->checksum==GetDataChecksum(&block_sav->data)) return LOAD_COMPLETE;
			else return ERRLOAD_CORRUPT;
		case EVENT_ERROR:
			close(async_file);
			return ERRLOAD_READ;
		}
		break;
	case CARDOP_WRITE:
		switch(GetCardFastEventSw())
		{
		case EVENT_IOE:
			close(async_file);
			card_operation=CARDOP_NONE;
			return SAVE_COMPLETE;
		case EVENT_ERROR:
			close(async_file);
			return ERRSAVE_WRITE;
		}
		break;
	case CARDOP_PRELIST:
		if(async_wait<=0) return ReadMcDirectoryAsync(async_channel,async_d,async_name);
		async_wait--;
		break;
	case CARDOP_LIST:
		return ListMcDirectory();
	}
	return DO_NOTHING;
}

int IsRmSaveData(struct DIRENTRY *entry)
{
	if(memcmp(id_code,entry->name,sizeof(id_code)-1)==0) return 1;	// is RM data
	return 0;
}

u32 GetDataChecksum(SAVE_DATA* data)
{
	u32 i, checksum=0;
	u8* buffer=(u8*)data;

	for(i=0; i<sizeof(SAVE_DATA); i++) checksum^=SHA_ROL(dec_tbl[*buffer++],i);
	return checksum;
}

int CreateSave(long channel, char *fileName)
{
	SAVE_BLOCK *block;
	MC_FILE_HEADER *head;
	SAVE_DATA *save;
	SAVE_SHORT *shrt;
	u16 sjis[2];
	int i;
	char dest[SINGLE_BLOCK]={0};

	block=(SAVE_BLOCK*)dest;
	head=&block->head;
	save=&block->data;
	shrt=&head->res;

	// set mc block header
	head->magic[0]='S';
	head->magic[1]='C';
	head->type=0x11;
	head->blockEntry=1;
	memcpy2(head->clut,spec_data.clut,sizeof(spec_data.clut));
	memcpy2(head->icon,spec_data.icon,MCICON_SIZE);
	// reference data
//#ifdef DEBUG
	shrt->area=0;
	zmemcpy(shrt->name,"TEST",sizeof("TEST"));
	shrt->player=0;
	shrt->level=47;
	shrt->playtime_hour=10;
	shrt->playtime_min=23;
	shrt->playtime_sec=17;
	shrt->difficulty=1;
	shrt->money=99999;
	shrt->rate=(200<<16)|4;
//#else
//#endif
	// title
	zmemcpy(head->title,save_title,sizeof(save_title));
	sjis[1]='\0';
	for(i=0; i<8; i++)
	{
		if(shrt->name[i]==0) break;
		sjis[0]=AsciiToSjis(shrt->name[i]);
		strcat(head->title,(char*)sjis);
	}

	// set real data
	zmemcpy(&save->user_data,&user_data,sizeof(user_data));
	zmemcpy(&save->char_data,&char_data,sizeof(char_data));
	save->playtime=playtime;
	save->money=money;
	save->kills=kills;
	save->rooms=rooms;
	save->pad_confirm=pad_confirm;
	save->pad_cancel=pad_cancel;
	save->pad_menu=pad_menu;
	save->pad_jump=pad_jump;
	save->pad_atk1=pad_atk1;
	save->pad_atk2=pad_atk2;
	save->pad_slide=pad_slide;
	save->pad_map=pad_map;

	// set checksum
	block->checksum=GetDataChecksum(save);

	// write to memory card
	return SaveFileToCardAsync(channel,fileName,dest,1);
}
