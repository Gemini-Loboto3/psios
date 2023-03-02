#ifndef __MEMCARD_H
#define __MEMCARD_H

#define id_code				"BISLPS-99999DXRM"
#define save_title			"ÇcÇwÅFÅ@ÇqÇÖÇÑÅ@ÇlÇèÇèÇéÅ^"	// "DX: Red Moon/" encoded in sjis

#define SINGLE_BLOCK		8192
#define MCICON_SIZE			128
#define ASYNC_READ			8192

enum CARD_SCHEDULE
{
	DO_NOTHING,
	ERR_NONE,
	ERRSAVE_CREATE,					// can't create, memory card is full
	ERRSAVE_WRITE,					// write failed
	ERRLOAD_READ,					// read failed
	ERRLOAD_CORRUPT,				// checksum doesn't match
	ERRLOAD_OPEN,					// can't open
	LIST_GENERATION,				// generating list
	LIST_GENERATED,					// generating list
	LOAD_COMPLETE,					// load complete
	SAVE_COMPLETE					// save complete
};

enum CARD_OPERATIONS
{
	CARDOP_NONE,					// no memory card process active
	CARDOP_READ,					// reading file
	CARDOP_WRITE,					// writing file
	CARDOP_PRELIST,					// prepare for listing blocks
	CARDOP_LIST,					// creating block list for one card
	CARDOP_LISTS					// creating block list for both cards
};

enum CARD_SW_EVENTS
{
	EVENT_IOE,						// processing complete
	EVENT_ERROR,					// bad card
	EVENT_TIMEOUT,					// no card
	EVENT_NEWCARD					// new card
};

enum
{
	PRESENT_CARD_FORMATTED,
	PRESENT_CARD_BAD,
	PRESENT_CARD_NONE,
	PRESENT_CARD_UNFORMATTED,
	NEW_CARD_FORMATTED,
	NEW_CARD_BAD,
	NEW_CARD_NONE,
	NEW_CARD_UNFORMATTED
};

#define SHA_ROT(X,l,r)	(((X) << (l)) | ((X) >> (r)))
#define SHA_ROL(X,n)	SHA_ROT(X,n,32-(n))
#define SHA_ROR(X,n)	SHA_ROT(X,32-(n),n)

typedef struct tagSaveShort
{
	u32 unlocked;			// for special content
	u8 name[8];
	fixed rate;				// map completion
	int money;
	char playtime_sec;
	char playtime_min;
	char playtime_hour;
	char area;
	char player;
	char difficulty;
	u16 level;
} SAVE_SHORT;

typedef struct tagMcFileHeader
{
	char magic[2];			// always "SC"
	char type;				// 0x11 1, icon 0x12 2 icons, 0x13 3 icons
	char blockEntry;		// number of 8k slots used
	u16 title[32];			// title 32 chars SJIS
	SAVE_SHORT res;			// usually reserved
	u16 clut[16];			// 4bit clut
	u8 icon[128];			// 16*16*4bit icon bitmaps
} MC_FILE_HEADER;

typedef struct tagCard		// for fast reads
{
	char magic[2];			// always "SC"
	char type;				// 0x11 1, icon 0x12 2 icons, 0x13 3 icons
	char blockEntry;		// number of 8k slots used
	u16 title[32];			// title 32 chars SJIS
	SAVE_SHORT res;			// usually reserved
	u16 clut[16];			// 4bit clut
} _CARD, MC_FAST_HEADER;

typedef struct tagSaveData
{
	u8 name[8];				// custom name
	// player data
	CHAR_DATA char_data;
	USER_DATA user_data;
	int money;
	int kills;
	int rooms;
	// botton variables
	short pad_confirm;
	short pad_cancel;
	short pad_menu;
	short pad_jump;
	short pad_atk1;
	short pad_atk2;
	short pad_slide;
	short pad_map;
	// time stat
	int playtime;
} SAVE_DATA;

typedef struct tagSaveBlock
{
	MC_FILE_HEADER head;
	u32 checksum;
	SAVE_DATA data;
} SAVE_BLOCK;

// generic i/o
void InitializeCardAndEvents(int shared);
int GetCardStatus(long channel);
void SetPort(long channel);
int ReadMcDirectory(long channel, struct DIRENTRY *d, char *filter);
int ReadMcDirectoryAsync(long channel, struct DIRENTRY *d, char *filter);
int FastReadMcDirectory(struct DIRENTRY *d, char *filter);
int DeleteFileFromCard(long channel, char *fileName);
int LoadFileFromCard(long channel, char *fileName, void* dest, int blocks);
int LoadFileFromCardAsync(long channel, char *fileName, void* dest, int blocks);
int SaveFileToCard(long channel, char *fileName, void* buffer, int blocks);
int SaveFileToCardAsync(long channel, char *fileName, void* buffer, int blocks);
// special functions
void PrepareReadMcDirectory(long channel, struct DIRENTRY *d, char *filter);
int FileExistsOnCard(char *fileName);
int GetBlocksRead(void);
int GetBlockCount(void);

// dangerous operations
int FormatCard(long channel);
int InitializeCard(long channel);
int QuickInitializeCard(long channel);

// conversion
u16 AsciiToSjis(char ascii);

// red moon specific
int CardMessages();
int IsRmSaveData(struct DIRENTRY *entry);
u32 GetDataChecksum(SAVE_DATA* data);
int CreateSave(long channel, char *fileName);

#endif	// __MEMCARD_H
