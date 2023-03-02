#ifndef __GENERAL_H
#define __GENERAL_H

#include "common.h"

#ifdef DEBUG
#define DEBUGPRINT(x)	_tprintf x
#else
#define DEBUGPRINT(x)	if(0)
#endif

extern void* HiRom;		// declare secondary rom address

int GetStringWidth(u8 *str, int mode);

#define MAX(x,y)		if((x)>(y)) (x)=(y);
#define MIN(x,y)		if((x)<(y)) (x)=(y);
#define RESETMAX(x,y)	if(x>(y)) x=0;
#define RESETMIN(x,y)	if(x<0) x=(y);

#define bsizeof(argument)		(sizeof(argument)/sizeof(*(argument)))
#define DIMENSION_OF(argument)	bsizeof((argument))

void Decrypt(u32* Data, int Size, int Seed);

void _tprintf(char *txt, ...);
static __inline int align(int val, int align) {return (((val-1)/align)+1)*align;}

void zmemset(void *b, u32 c, size_t size);
void* zmemcpy(void *dst, const void *src, size_t len);

#endif // __GENERAL_H
