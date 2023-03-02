#ifndef __CDFILE_H
#define __CDFILE_H

#include <MALLOC.H>
#include "cd.h"

// queue maximum size
#define CD_QUEUE_MAX	16
// how many times to retry for CdSearchFile
#define READ_TRY		10

typedef struct tagCdFile
{
	int status;			// check CDFILE_STATUS
	u32 pos;			// absolute LBA
	u32 cdseek;			// current seek LBA
	u32 oldseek;		// previous seek LBA, for bufferized reads
	u32 size;			// file size in bytes
	u32 bufseek;		// current buffer seek (0-0x7FF)
	u32 last_read;		// size of the last read
	u8 buffer[SECTOR];	// data buffer for fread-like function
	u8 *dest;			// actual destination in ram
} CD_FILE;

typedef struct CdQueue
{
	CD_FILE *queue;
	int active;
} CD_QUEUE;

enum READ_TYPE
{
	READ_SYNC,
	READ_ASYNC
};

enum CDFILE_STATUS
{
	CDFS_NONE,			// no operation performed (or not interesting)
	CDFS_READ,			// regular read
	CDFS_AREAD,			// regular async read
	CDFS_FILLING_BUFFER,// setup buffer filling
	CDFS_FILLED_BUFFER,	// buffer is filled, copy to destination
	CDFS_FILL_SEEK,		// filling buffer in seek
	CDFS_READY,			// read done
};

/* open file using the cd toc */
CD_FILE* CdFileOpen(char *filename);
/* open file using the virtual toc */
CD_FILE* CdFileFSOpen(char *filename, int type);
/* close handle to file */
static __inline void CdFileClose(CD_FILE *file) {if(file) free3(file);}

/* read */
void CdFileRead(void *ptr, int size, int count, CD_FILE *file);
void CdFileReadAsync(void *ptr, int size, int count, CD_FILE *file);

/* set file seek*/
int CdFileSeek(CD_FILE *file, int offset, int origin);

void InitCdCallback();
void CdFile_VSyncCB();

#endif
