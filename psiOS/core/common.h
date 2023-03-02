#ifndef __COMMON_H
#define __COMMON_H

#define NO_SOUND

/* --------------------------------------------------------------------------
 * - All variable types														-
 * -------------------------------------------------------------------------- */
typedef unsigned char BYTE;
typedef unsigned short USHORT;
typedef unsigned long ULONG;
typedef void* LPVOID;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;
typedef unsigned long long u64;

typedef signed char s8;
typedef signed short s16;
typedef signed long s32;
typedef signed long long s64;

typedef int BOOL;
typedef int bool;

#define FALSE			0
#define TRUE			1

void run__templ();

#define GetS16(ptr)		*((s16*)(ptr))
#define GetU16(ptr)		*((u16*)(ptr))
#define GetS32(ptr)		*((s32*)(ptr))
#define GetU32(ptr)		*((u32*)(ptr))

// evaluates to the offset (in bytes) of a given member within a struct or union type, an expression of type size_t
#define offsetof(st, m)	((size_t)((char *)&((st*)(0))->m - (char*)0))
// allows something like a Mixin type to find the structure that contains it
#define container_of(ptr, type, member) ({ \
	const typeof(((type*)0)->member) *__mptr=(ptr); \
	(type*)((char*)__mptr - offsetof(type,member));})

// load-address for child process
extern void* KernelAddress;		// only kernel
extern void* FieldAddress;		// only field
extern void* LoadAddress;		// menus and temp modules (boot, title)
//extern void* EnemyAddress;		// enemy ai scripts
extern void* AllocData;			// temp for compressed overlays

// vsync data
extern int vsync_sec;

//#define GraBuffer			(AllocData+128*1024)
//#define EntBuffer			(AllocData+256*1024)

enum OVERLAY_ID
{
	OVERLAY_BOOT,
	OVERLAY_KERNEL,
	OVERLAY_TITLE,
	OVERLAY_FIELD,
	OVERLAY_MENU
};

/* decompress and load overlays */
void LoadOverlay(int id);
void LoadMenu(int id);

#define Align(val,align)	if(val%align!=0) val+=align-(val%align);
#define Align16(val)		(((val)+1)&~1)
#define Align32(val)		(((val)+3)&~3)
#define GetAlign(val,align)	(((val)+(align-1))&~(align-1))

/* ---------------------------------------------------------------------------
 * - CONSTANTS
 * ---------------------------------------------------------------------------
 */
//enum VIDEO_MODE
//{
//	MODE_NTSC,
//	MODE_PAL
//};

// Screen position and dimensions. 
#define	FRAME_X			320
#define	FRAME_Y			240
#define SCREEN_X		0
#define SCREEN_Y		16
#define SCREEN_W		FRAME_X
#define SCREEN_H		208
#define LOCK_X			(SCREEN_W/2)
#define LOCK_Y			(SCREEN_H/2)
#define	MAP_W			((SCREEN_W/16)+1)
#define MAP_H			((SCREEN_H/16)+1)

/* ---------------------------------------------------------------------------
 * - DATA TYPE AND STRUCTURE DECLARATIONS
 * ---------------------------------------------------------------------------
 */
enum FT4_MODE
{
	FT4_MODE4BPP,
	FT4_MODE8BPP,
	FT4_MODE16BPP
};

#define ROOM_MAX			2040

#define FLAG_SOUND			0x01	// sound mode: 0=Mono, 1=Stereo
#define FLAG_TRANS			0x02	// window transparency: 0=opaque, 1=transparent
#define FLAG_VIBRA			0x04	// vibration
#define FLAG_VIDEO			0x08	// video mode: 0=PAL, 1=NTSC
#define FLAG_WIDESCREEN		0x10	// field aspect ratio: 0=4:3, 1=16:9

#define MAGIC(a,b,c,d)		((a)|((b)<<8)|((c)<<16)|((d)<<24))

enum LANGUAGES
{
	ENGLISH,
	ITALIAN,
	GERMAN,
	SPANISH,
	FRENCH,
	JAPANESE
};

typedef struct USER_DATA
{
	u8 message_speed;
	u8 volume_voice;		// 0~127
	u8 volume_music;		// 0~127
	u8 volume_effect;		// 0~127
	s8 screenx;
	s8 screeny;
	s8 language;			// see enum LANGUAGES
	u8 flags;				// option flags, see FLAG_XXXX
	//u16 cheats;				// bit flags
} USER_DATA;

typedef struct tagMiscData
{
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
	// time stats
	char playtime_sec;
	char playtime_min;
	char playtime_hour;
} MISC_DATA;

typedef struct tagSpecialData
{
	u8 icon[128];
	u16 clut[16];
} SPEC_DATA;

typedef struct ITEM_PROP
{
	s16 atk, def;
	s8 str, con, inte, mnd, lck;
	u8 padding;				// unused
	u16 element;
	u16 weak, absorb, null, res;
	u8 type;				// see ITEM_TYPE list, 0x80=consumable
	u8 special;				// special item code
} ITEM_PROP;

typedef struct WEAPON_DATA
{
	u16 pose;				// used when attacking
	short sfx;				// 0=no sound effect, -1=loaded VAB
	u8 max;					// limit for items on screen
	u8 delay;				// frames until you can use it again
} WEAPON_DATA;

typedef struct SHOP_DATA
{
	u32 price;
	u8 flag;
} SHOP_DATA;

enum MONSTER_TYPE
{
	MONTYPE_NONE,
	MONTYPE_UNDEAD,
	MONTYPE_WEREWOLF,
	MONTYPE_HUMAN,
	MONTYPE_SPIRIT,
	MONTYPE_DRAGON
};

typedef struct tagMonsterData
{
	// stats
	s16 hp:15;
	s16 nodamage:1;
	s16 atk, def;
	// attributes
	u16 element;
	u16 weak, absorb, null, res;
	// other
	u8 drop[2];
	u8 dropf[2];
	u16 exp;
	u8 type;				// check MONSTER_TYPE
	u8 padding;
} MONSTER_DATA;

/* ---------------------------------------------------------------------------
 * - GLOBAL DECLARATIONS
 * ---------------------------------------------------------------------------
 */
	// GPU packet space
#define PACKETMAX		(2048)
#define PACKETMAX2		(PACKETMAX*8)

	// size of ordering table: 2 << OT_LENGTH
	// i.e.	16384 levels of z resolution
#define OT_LENGTH		(12)

#endif
