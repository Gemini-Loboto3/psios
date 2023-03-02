#ifndef __CD_THREAD_H
#define __CD_THREAD_H

#define MAX_CD_THREADS	16	

typedef struct tagCdThreadEntry
{
	u32 *dest;			// destination in memory
	u32 size;			// total of sectors
	u32 read;			// counter for read sectors
} CD_THREAD_ENTRY;

#endif
