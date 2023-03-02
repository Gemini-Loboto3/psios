/* ---------------------------------------------------------------------------
 * - (C) Computer o Tsukau Neko. All Rights Reserved.
 * -
 * - Project:	Red Moon	
 * -
 * - Name:		char.h
 * -
 * - Author:		Gemini
 * -
 * - Date:		7 Jun 2005
 * ---------------------------------------------------------------------------
 */
#ifndef __CHAR_H
#define __CHAR_H

#include "general.h"

/* ---------------------------------------------------------------------------
 * - GLOBAL DECLARATIONS
 * ---------------------------------------------------------------------------
 */
#define HP_MAX					9999
#define MP_MAX					9999
#define HEART_MAX				9999
#define STATS_MAX				999
#define EXP_MAX					9999999
#define GOLD_MAX				9999999
#define AFFINITY_MAX			99999
#define MINUTE					(60)
#define HOUR					(MINUTE*MINUTE)
#define TIME_MAX				(99*HOUR+59*MINUTE+59)	// 99:59:59

#define CURSE_TIMER				20*vsync_sec
#define POISON_TIMER			30*vsync_sec
#define STONE_TIMER				8

// retrieve bit fields for a key item
#define GetKeyItemFlags(bits,key)		((bits>>((key)*2))&3)
// retrieve bit field for affinities
#define GetAffinityBit(bits,value)		(bits&(1<<(value)))
// retrieve bit field for an enemy in the bestiary
#define GetBestiaryBits(val,index)		((val.enemy[index/2]>>((index&1)*4))&0xF)

#define ENEMYBIT_ENABLE		0x1
#define ENEMYBIT_KILLED		0x2
#define ENEMYBIT_ITEM1		0x4
#define ENEMYBIT_ITEM2		0x8
#define ENEMYBIT_COMPLETE	0xF

// use with GetAffinityBit
enum ELEMENTAL_AFFINITY
{
	HIT,
	SLASH,
	FIRE,
	WATER,
	LIGHTNING,
	ICE,
	EARTH,
	WIND,
	HOLY,
	DARK,
	STONE,
	CURSE,
	POISON,
	DAMAGE
};

// use in combination with GetKeyItemFlags
enum KEY_ITEM_FLAG
{
	KEY_WEAPON_SWITCH,
	KEY_ASTRAL_BODY,
	KEY_DOUBLE_JUMP,
	KEY_WALL_JUMP,
	KEY_SUPER_JUMP,
	KEY_MATERIALIZE,
	KEY_ELEMENT_ORB,
	KEY_WATER_WALK,
	KEY_WEATHER,
	KEY_SHOW_NAMES,
	KEY_SHOW_DAMAGE
};

typedef struct tagCharData
{
	// stats
	s16 hp, maxhp, bonus_hp;
	s16 mp, maxmp, bonus_mp;
	s16 hrt, maxhrt, bonus_hrt;		// heart amount
	s16 atk, def;
	s16 str, con, inte, mnd, lck;
	// stats actually used
	s16 tatk, tdef;
	s16 tstr, tcon, tinte, tmnd, tlck;
	// affinity bonuses
	u32 affinity[10];		// check enum PLAYER_AFFINITIES
	// equip
	u8 hand[3];
	u8 head;
	u8 body;
	u8 acces1;
	u8 acces2;
	u8 switcher;			// to be used with hand[3]
	// timers for status
	s16 poison;
	s16 poison_frame;
	s16 curse;
	u8 stone;				// times you need to shake before petrify is gone
	// other
	u8 id;					// character ID
	u32 exp;
	u32 gold;
	u16 level;
	u8 equip[256];
	u32 relics;				// bit variable, 16 relics, 2 bits each
	u8 items[256/8];		// bit variable, 256 items, 1 bit each
	u8 enemy[256/4];		// bit variable, 256 enemies, 4 bits each
} CHAR_DATA;

typedef struct tagStatsCopy
{
	s16 atk, def;
	s16 str, con, inte, mnd, lck;
} STATS_COPY;

/* ------------------------------------------------------------------------ */

#endif // __CHARACTERS_H 
