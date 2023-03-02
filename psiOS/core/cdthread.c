#include "..\core.h"
#include "cdthread.h"

static CD_THREAD_ENTRY thread_queue[MAX_CD_THREADS]={0};

int CdThreadAppend(CD_THREAD_ENTRY *entry)
{
	int i;

	for(i=0; i<MAX_CD_THREADS; i++)
	{
		if(thread_queue[i].dest!=NULL)
		{
			zmemcpy(&thread_queue[i],entry,sizeof(CD_THREAD_ENTRY));
			return i;
		}
	}

	return -1;
}
