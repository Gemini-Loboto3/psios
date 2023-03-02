#include <sys/types.h>
#include <sys/file.h>
#include <stdio.h>
#include <kernel.h>
#include <string.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <MALLOC.H>
#include "common.h"
#include "cd.h"
#include "cdfile.h"
#include "general.h"

/*======================================================*/
FS_ALLOC fs_alloc[ARCHIVE_COUNT]=
{
	{"SYSTEM.ARC",	0,0},
	{"MAP.ARC",		0,0},
	{"SOUND.ARC",	0,0},
	{"DATA.ARC",	0,0}
};

/*======================================================*
 * Initialize the archive-based file system.			*
 *------------------------------------------------------*
 * Return: 1 if succeeded, 0 if failed.					*
 *======================================================*/
int FS_Init()
{
	int i, fsize;
	CD_FILE* fp;
	u8 sector[SECTOR];
	ARCHIVE_HEADER *head=(ARCHIVE_HEADER*) sector;

	// install CdRead callback
	InitCdCallback();

	// install archive data
	for(i=0; i<ARCHIVE_COUNT; i++)
	{
		// open current archive handle
		if(!(fp=CdFileOpen(fs_alloc[i].name)))
		{
			// return error on missing archives
			DEBUGPRINT(("FS_Init error: can't find %s\n",fs_alloc[i].name));
			return 0;
		}
		// bufferize first sector, which contains the header
		CdFileRead(sector,sizeof(sector),1,fp);
		// determine total header size size
		fsize=(head->count+1)*sizeof(ARCHIVE_ENTRY)+sizeof(ARCHIVE_HEADER);
		// allocate FS header dada
		fs_alloc[i].data=(u8*)malloc3(head->size*SECTOR);
		// copy header to allocated data
		zmemcpy(fs_alloc[i].data,sector,fsize>SECTOR ? SECTOR : fsize);
		// keep read if there's more header data
		if(head->size>1) CdFileRead(&fs_alloc[i].data[SECTOR],fsize-SECTOR,1,fp);
		// set reference sector
		fs_alloc[i].pos=fp->pos;
		DEBUGPRINT(("FS_Init: %s loaded at %x (table %x)\n",fs_alloc[i].name,fs_alloc[i].data,&fs_alloc[i]));
		// close handle
		CdFileClose(fp);
	}

	// success
	return 1;
}

/*======================================================*
 * Load a full file to ram using the file system		*
 * archives (blocking). Doesn't work correctly if the	*
 * file system tables haven't been initialized with		*
 * FS_Init().											*
 *------------------------------------------------------*
 * Note: when dest is NULL, it will be automatically	*
 * allocated.											*
 *------------------------------------------------------*
 * Return: destination buffer, NULL if failed.			*
 *======================================================*/
void* FS_Load(char *fname, int type, void* dest)
{
	CD_FILE *file;

	// out of range
	if(type>=ARCHIVE_COUNT) return NULL;

	// search file
	if(!(file=CdFileFSOpen(fname,type))) return NULL;	// file not found
	// dynamically allocate if necessary
	if(dest==NULL) dest=malloc3(file->size);
	// read the entire file
	CdFileRead(dest,file->size,1,file);
	// free handle
	CdFileClose(file);

	return dest;	// success
}

/*======================================================*
 * Load a full file to ram using the regular CD-ROM		*
 * system (blocking).									*
 *------------------------------------------------------*
 * Note: when dest is NULL, it will be automatically	*
 * allocated.											*
 *------------------------------------------------------*
 * Return: destination buffer, NULL if failed.			*
 *======================================================*/
void* CdLoadFile(char *fname, void *dest)
{
	CD_FILE *file;

	// try and open a handle
	if(!(file=CdFileOpen(fname))) return NULL;	// failure
	// allocate if dest is not defined
	if(!dest) dest=malloc3(file->size);
	// store to memory
	CdFileRead(dest,file->size,1,file);
	// release handle
	CdFileClose(file);
	// return buffer
	return dest;		// success
}

/*======================================================*
 * Load a segmented graphical package (format *.MTX) in	*
 * VRAM (blocking).										*
 *======================================================*/
void CdLoadGPack(u_char* dest, char* filename, int x, int y)
{
	int p;
	RECT rect;
	CD_FILE *file;

	// open handle from MAP.ARC
	file=CdFileFSOpen(filename,ARCHIVE_MAP);

	for(p=0; p<11; p++)
	{
		// read section
		CdFileRead(dest,8192,2,file);

		// upload palette chunk
		if(p==0)
		{
			rect.x=320;
			rect.y=240;
			rect.w=320;
			rect.h=16;
			LoadImage(&rect,(u_long*)dest);
		}
		// upload picture chunk
		else
		{
			rect.x=x;
			rect.y=y;
			rect.w=64;
			rect.h=128;
			LoadImage(&rect,(u_long*)dest);
			if((p-1)&1) {x+=64;y=0;}
			else y+=128;
		}
		// wait for loadimage sync
		DrawSync(0);
	}
	// close handle
	CdFileClose(file);
}

/*======================================================*
 * Read from disc in SECTOR-large blocks (blocking).	*
 *------------------------------------------------------*
 * Return: should always be 0 when reading occurred		*
 * correctly.											*
 *======================================================*/
int _read1(long byte, void *sectbuf, int mode)
{
	int nsector,cnt;
	nsector=(byte+2047)/2048;
		
	// read start
	CdRead(nsector, sectbuf, mode);

	while((cnt=CdReadSync(1,NULL))>0) VSync(0);
	return cnt;
}

/*======================================================*
 * Read from disc in SECTOR-large blocks (non-blocking).*
 *------------------------------------------------------*
 * Return: the number of sectors set for read.			*
 *======================================================*/
int _read_async(int size, void *dest, int mode)
{
	// pad to sector size
	int nsector=align(size,SECTOR)/SECTOR;
	// read start
	CdRead(nsector,(u_long*)dest,mode);
	return nsector;
}
