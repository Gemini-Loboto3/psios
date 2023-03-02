#ifndef __CD_H
#define __CD_H

#include <libcd.h>

/* on-read size of a data sector */
#define SECTOR			2048
/* actual size of a MODE2 FORM1 sector */
#define SECTOR_FULL		2352

/* ID for each file system archive */
enum ARCHIVES
{
	ARCHIVE_SYSTEM,		// SYSTEM.ARC: kernel data for the most (overlays, messages, gfx packs, etc.)
	ARCHIVE_MAP,		// MAP.ARC: data used by FIELD (*.MTX, *.MAP)
	ARCHIVE_SOUND,		// SOUND.ARC: music SPU streams (*.SPU)
	ARCHIVE_DATA,		// DATA.ARC: misc data such as entities
	// working substitute of sizeof(ARCHIVES)
	ARCHIVE_COUNT
};

/* keeps track of each main archive */
typedef struct tagFsAlloc
{
	char name[12];
	u8 *data;			// pointer to allocated structure
	u32 pos;			// absolute lba on disc
} FS_ALLOC;

/* archive header data, at least 1 sector */
typedef struct tagArchiveHeader
{
	char name[12];		// internal name
	int count;			// number of files archived
	int size;			// in sectors
	int reserve[3];		// pad, for future expansions
} ARCHIVE_HEADER;

/* file entry in archives */
typedef struct tagArchiveEntry
{
	char name[12];		// empty for last entry
	u32 pos:23;			// relative lba
	u32 rest:9;			// dummy area of the last sector to be ignored
							// real_size=(lba[1]-lba[0])*2048-(rest*4)
} ARCHIVE_ENTRY;

/* archived file system functions */
/* initialize file system */
int FS_Init();
/* load a file in memory (blocking) */
void* FS_Load(char *fname, int type, void* dest);

/* regular file system functions */
/* load a file in memory (blocking) */
void* CdLoadFile(char *fname, void *dest);
/* caches MTX textures as necessary (blocking) */
void CdLoadGPack(u_char* dest, char* filename, int x, int y);

/* internal data-reading functions, never use directly */
/* read from disc (blocking) */
int _read1(long byte, void *sectbuf, int mode);
/* read from disc (non-blocking) */
int _read_async(int size, void *dest, int mode);

#endif
