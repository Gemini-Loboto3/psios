#ifndef __DATA_H
#define __DATA_H

#include <sys/types.h>

// Structures
typedef struct
{
	char header[4];
	u_long file_num;
	BagEntry* entry;
} SbBagHeader;

typedef struct
{
	char name[31];
	u_char compression;
	u_int position;
	u_int lenght;
} SbBagEntry;

#endif // __DATA_H