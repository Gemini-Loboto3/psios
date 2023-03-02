/* ---------------------------------------------------------------------------
 * - (C) No!Maki. All Rights Reserved.
 * -
 * - Project:	Red Moon engine
 * -
 * - Name:		main.h
 * -
 * - Author:	Gemini
 * -
 * - Date:		17 Oct 2011
 * ---------------------------------------------------------------------------*/
#include <sys/types.h>
#include <sys/file.h>
#include <stdio.h>
#include <kernel.h>
#include <string.h>
#include <malloc.h>
#include "common.h"
#include "cd.h"
#include "cdfile.h"
#include "general.h"

/* queued data: NULL=free */
CD_FILE *cd_queue[CD_QUEUE_MAX]={NULL};

/*======================================================*
 * Allocate an async request in the queue table.		*
 *------------------------------------------------------*
 * Return: the allocated id, -1 if the queue is full.	*
 *======================================================*/
static int allocate_queue(CD_FILE *file)
{
	int i;

	for(i=0; i<CD_QUEUE_MAX; i++)
	{
		if(!cd_queue[i]/*.active*/)
		{
			// free cell found, allocate
			//cd_queue[i].queue=file;
			//cd_queue[i].active=TRUE;
			cd_queue[i]=file;
			return i;
		}
	}

	// could not allocate
	return -1;
}

/*======================================================*
 * Dellocate an async request in the queue table.		*
 *------------------------------------------------------*
 * Return: the freed id, -1 if the entry was not found.	*
 *======================================================*/
//static int free_queue(CD_FILE *file)
//{
//	int i=0;
//
//	for(i=0; i<CD_QUEUE_MAX; i++)
//	{
//		if(cd_queue[i]/*.queue*/==file)
//		{
//			// found associated cell, free
//			//cd_queue[i].queue=NULL;
//			//cd_queue[i].active=FALSE;
//			cd_queue[i]=NULL;
//			return i;
//		}
//	}
//
//	// count not free
//	return -1;
//}

/*======================================================*
 * Set read state to complete							*
 *======================================================*/
static void _cb_release(int queue_id)
{
	CdControlF(CdlNop,NULL);
	cd_queue[queue_id]->last_read=0;
	cd_queue[queue_id]=NULL;
}

/*======================================================*
 * Callback function to be executed upon completion of	*
 * CdRead(), _read1() and _read_async().				*
 *------------------------------------------------------*
 * TODO: handle CdlDiskError status.					*
 *======================================================*/
static CdlCB _cd_read_callback(u_char status, u_char *result)
{
	int i;
	u32 temp;

	// handle read errors
	if(status==CdlDiskError)
	{
		DEBUGPRINT(("CdReadCallback(): CdlDiskError!\n"));
		// TODO: add here procedure to re-read data
	}

	// search for an active transfer in async mode
	for(i=0; i<CD_QUEUE_MAX; i++)
	{
		// if an active transfer is found
		if(cd_queue[i])
		{
			//DEBUGPRINT(("CdReadCallback() status: %d\n",cd_queue[i]->status));
			switch(cd_queue[i]->status)
			{
			case CDFS_NONE:			// no operation is set
			case CDFS_FILL_SEEK:	// finished filling buffer after non sector-precise seek
				_cb_release(i);
				break;
			case CDFS_AREAD:		// finished reading data to destination buffer
				// check if the last non-full sector is necessary
				if((cd_queue[i]->last_read%SECTOR)>0)
				{
					DEBUGPRINT(("Reading last sector %d.\n",cd_queue[i]->last_read%SECTOR));
					// set transfer if so
					cd_queue[i]->status=CDFS_FILLING_BUFFER;
					cd_queue[i]->bufseek=0;
				}
				// no remaining data, clear entry
				else
				{
					DEBUGPRINT(("Sector-precise read done.\n"));
					// clear this entry otherwise
					cd_queue[i]->status=CDFS_READY;
					_cb_release(i);
				}
				return;
			case CDFS_FILLED_BUFFER:	// buffer fill request is done, copy to destination now
				// transfer size
				temp=cd_queue[i]->last_read%SECTOR;
				DEBUGPRINT(("Filling %x at %x with %d bytes\n",cd_queue[i]->dest,&cd_queue[i]->dest[cd_queue[i]->last_read-temp],temp));
				// fill destination with temp buffer data
				zmemcpy(&cd_queue[i]->dest[cd_queue[i]->last_read-temp],cd_queue[i]->buffer,temp);
				// update buffer seek and set 'ready' status
				cd_queue[i]->bufseek=temp;
				cd_queue[i]->status=CDFS_READY;
				_cb_release(i);
				return;
			}
		}
	}
}

/*======================================================*
 * Callback function to be executed whenever a non		*
 * sector-precise read is necessary.					*
 *======================================================*/
void CdFile_VSyncCB()
{
	int i;

	// search for an active transfer in async mode
	for(i=0; i<CD_QUEUE_MAX; i++)
	{
		// if an active transfer is found
		if(cd_queue[i])
		{
			//DEBUGPRINT(("CdFile_VSyncCB status %d\n",cd_queue[i]->status));
			switch(cd_queue[i]->status)
			{
			case CDFS_FILLING_BUFFER:	// fill buffer request
				// update queue status
				cd_queue[i]->status=CDFS_FILLED_BUFFER;
				// seek to the sector we need to bufferize
				CdFileSeek(cd_queue[i],cd_queue[i]->cdseek*SECTOR,SEEK_SET);
				// set transfer to the buffer
				_read_async(SECTOR,cd_queue[i]->buffer,CdlModeSpeed);
				break;
			}
		}
	}
}

/*======================================================*
 * Open a file handle using the CD-ROM system.			*
 *------------------------------------------------------*
 * Return: handle pointer if succeeded, NULL if failed.	*
 *======================================================*/
CD_FILE* CdFileOpen(char *filename)
{
	int i;
	bool found;
	char name[256];
	CdlFILE fp;
	CD_FILE *file;

	sprintf(name,"\\%s;1",filename);
	for(i=0, found=FALSE; i<READ_TRY; i++)
	{
		if(CdSearchFile(&fp,name))
		{
			found=TRUE;
			break;
		}
	}
	// nothing was found, error
	if(!found)
	{
		DEBUGPRINT(("CdFileOpen error: can't find %s",filename));
		return NULL;
	}

	// allocate
	file=(CD_FILE*)malloc3(sizeof(CD_FILE));
	// seek at the beginning
	CdControl(CdlSetloc,(u8*)&(fp.pos),0);
	// set data
	file->pos=CdPosToInt(&fp.pos);
	file->size=fp.size;
	file->cdseek=0;
	file->oldseek=-1;
	file->bufseek=0;
	// set status
	file->status=CDFS_NONE;
	// seek at the beginning
	//CdFileSeek(file,0,SEEK_SET);

	return file;	// success
}

/*======================================================*
 * Open a file handle using the archived file system	*
 * tables.												*
 *------------------------------------------------------*
 * Return: handle pointer if succeeded, NULL if failed.	*
 *======================================================*/
CD_FILE* CdFileFSOpen(char *filename, int type)
{
	int i;
	//CdlFILE fp;
	extern FS_ALLOC fs_alloc[];
	ARCHIVE_HEADER *head;
	ARCHIVE_ENTRY *entry;
	CD_FILE *file;

	head=(ARCHIVE_HEADER*)fs_alloc[type].data;
	entry=(ARCHIVE_ENTRY*)(fs_alloc[type].data+sizeof(ARCHIVE_HEADER));
	// search entry
	for(i=0; i<head->count; i++)
	{
		if(strncmp(entry[i].name,filename,sizeof(entry[0].name))==0)
		{
			file=(CD_FILE*)malloc3(sizeof(CD_FILE));
			// allocate
			file->pos=fs_alloc[type].pos+entry[i].pos;
			file->size=(entry[i+1].pos-entry[i].pos)*SECTOR-(entry[i].rest*4);
			file->cdseek=0;
			file->oldseek=-1;
			file->bufseek=0;
			//file->not_ready=0;
			file->status=CDFS_NONE;
			// seek at the beginning
			//CdFileSeek(file,0,SEEK_SET);
			//CdIntToPos(file->pos,&fp.pos);
			//CdControl(CdlSetloc,(u8*)&(fp.pos),0);

			DEBUGPRINT(("CdFileFSOpen: handle for %s open: %d bytes at %d!\n",filename,file->size,file->pos));
			return file;	// success
		}
	}
	DEBUGPRINT(("CdFileFSOpen error: could not find %s.\n",filename));
	return NULL;
}

/*======================================================*
 * Read from CD_FILE. Takes care of non	sector precise	*
 * reads. (blocking)									*
 *------------------------------------------------------*
 * NOTE: CD_FILE->status is always set to CDFS_READY	*
 * at the end so that completion can be checked just	*
 * like for async accesses. Just in case.				*
 *======================================================*/
void CdFileRead(void *ptr, int size, int count, CD_FILE *file)
{
	u32 read_size, bufseek;
	u8* ref;

	if(file->oldseek!=file->cdseek) CdFileSeek(file,file->cdseek*SECTOR,SEEK_SET);
	// get real size
	size*=count;
	// set byte references
	file->dest=ref=(u8*)ptr;
	// take into account seek
	file->cdseek+=((size+file->bufseek)/SECTOR);

	// copy remaining buffer data if there's any
	if((bufseek=file->bufseek)>0)
	{
		//DEBUGPRINT(("CdFileRead: copying leftovers %d\n",bufseek));
		// determine transfer size
		read_size=(size+bufseek>SECTOR ? SECTOR-bufseek : size);
		// copy from buffer to destination
		zmemcpy(ptr,&file->buffer[bufseek],read_size);
		// update buffer seek (or reset it)
		file->bufseek=(bufseek+read_size)%SECTOR;
		// move pointer to next read position
		ref+=read_size;
		// get cd transfer size
		size-=read_size;
	}

	// set transfer if new sector data is necessary
	if(size>0)
	{
		//DEBUGPRINT(("CdFileRead: reading %d\n",size));
		// store current read size
		file->last_read=size;
		// determine size and mode
		if(size%SECTOR>0)
		{
			file->status=CDFS_FILLED_BUFFER;
			size-=(size%SECTOR);
		}
		// read the necessary data
		_read1(size,ref,CdlModeSpeed);
		// read any non sector-precise leftovers
		if(file->status==CDFS_FILLED_BUFFER)
		{
			//DEBUGPRINT(("CdFileRead: reading remains %d\n",file->last_read%SECTOR));
			CdFileSeek(file,file->cdseek*SECTOR,SEEK_SET);
			_read1(SECTOR,file->buffer,CdlModeSpeed);
			zmemcpy(&ref[(file->last_read/SECTOR)*SECTOR],file->buffer,file->last_read%SECTOR);
		}
	}

	// set 'ready' flag
	file->status=CDFS_READY;
}

/*======================================================*
 * Read from CD_FILE. Takes care of non	sector precise	*
 * reads and queues requests when new data needs to be	*
 * read. (non-blocking)									*
 *------------------------------------------------------*
 * NOTE: in order to know if the transfer is complete,	*
 * check CD_FILE->status. If it's CDFS_READY then the	*
 * transfer is successfully done.						*
 *======================================================*/
void CdFileReadAsync(void *ptr, int size, int count, CD_FILE *file)
{
	u32 read_size, bufseek;
	u8* ref;

	if(file->oldseek!=file->cdseek) CdFileSeek(file,file->cdseek*SECTOR,SEEK_SET);
	// get real size
	size*=count;
	// set byte reference
	ref=(u8*)ptr;
	// take into account seek
	file->cdseek+=((size+file->bufseek)/SECTOR);

	// copy remaining buffer data if there's any
	if((bufseek=file->bufseek)>0)
	{
		//DEBUGPRINT(("CdFileReadAsync: Copying leftovers %d\n",bufseek));
		// determine transfer size
		read_size=(size+bufseek>SECTOR ? SECTOR-bufseek : size);
		// copy from buffer to destination
		zmemcpy(ptr,&file->buffer[bufseek],read_size);
		// update buffer seek (or reset it)
		file->bufseek=(bufseek+read_size)%SECTOR;
		// move pointer to next read position
		ref+=read_size;
		// get cd transfer size
		size-=read_size;
		// set 'ready' if we're done reading from the buffer
		file->status=CDFS_READY;
	}

	// set updated reference
	file->dest=ref;
	// set transfer if new sector data is necessary
	if(size>0)
	{
		//DEBUGPRINT(("CdFileReadAsync: Reading %d\n",size));
		// store current read size
		file->last_read=size;
		// determine size and mode
		if(size>SECTOR)
		{
			// fix non sector-precise reads
			if(size%SECTOR>0) size-=size%SECTOR;
			file->status=CDFS_AREAD;
		}
		else file->status=CDFS_FILLED_BUFFER;
		// queue and read the necessary data
		allocate_queue(file);
		_read_async(size,ref,CdlModeSpeed);
	}
}

/*======================================================*
 * Seek on the CD_FILE entry and fills the internal		*
 * if it's necessary. Works best in async mode.			*
 *------------------------------------------------------*
 * Return: 1 if the seek is valid, otherwise 0.			*
 *======================================================*/
int CdFileSeek(CD_FILE *file, int offset, int origin)
{
	CdlLOC pos;

	// check if any previous operation is still active
	if((file->status!=CDFS_READY || file->status!=CDFS_NONE) && CdReadSync(0,NULL)>0)
	{
		DEBUGPRINT(("CdFileSeek: interrupting read (%d-%d).\n",file->status,CdReadSync(0,NULL)));
		CdControlF(CdlNop,NULL);
	}

	switch(origin)
	{
	case SEEK_SET:
		file->cdseek=offset/SECTOR;
		file->bufseek=offset%SECTOR;
		break;
	case SEEK_CUR:
		CdFileSeek(file,file->cdseek*SECTOR+offset,SEEK_SET);
		return 1;	// success
	case SEEK_END:
		CdFileSeek(file,file->size-offset,SEEK_SET);
	default:
		return 0;	// ignore
	}
	//DEBUGPRINT(("CdFileSeek(): Seek to %d, base sector %d\n",file->pos+file->cdseek,file->pos));
	// seek to sector if there's a new sector to read
	if(file->oldseek!=file->cdseek)
	{
		file->oldseek=file->cdseek;
		//DEBUGPRINT(("Seeking for real.\n"));
		CdIntToPos(file->pos+file->cdseek,&pos);
		CdControl(CdlSetloc,(u8*)&pos,0);
		// bufferize sector if necessary
		if(file->bufseek>0)
		{
			file->status=CDFS_FILL_SEEK;
			file->last_read=SECTOR;
			allocate_queue(file);
			_read_async(SECTOR,file->buffer,CdlModeSpeed);
		}
	}
	return 1;		// success
}

/*======================================================*
 * Set queued cd transfer, always async.				*
 *======================================================*/
int CdFileLoad(CD_FILE *file, void *dest)
{
	int ret;

	if((ret=allocate_queue(file))>0)
	{
		CdFileReadAsync(dest,file->size,1,file);
		return TRUE;
	}

	return FALSE;
}

/*======================================================*
 * Install CdRead() callback for checking the cd queue.	*
 *======================================================*/
void InitCdCallback()
{
	CdReadCallback((CdlCB)_cd_read_callback);
	DEBUGPRINT(("Installed CDFS callback.\n"));
}
