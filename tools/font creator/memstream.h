#pragma once
#include "types.h"

#define MEMFILE_ALLOC		32*1024

typedef struct tagMemStream
{
	u8 *data;
	int size, alloc;
	int pos;
} MEM_STREAM;

/* release buffer and close stream */
void MemStreamClose(MEM_STREAM *str);
/* allocate buffer and create a new stream */
void MemStreamCreate(MEM_STREAM *str);
/* flush the contents of the buffer to a physical file */
void MemStreamFlush(MEM_STREAM *str, LPCTSTR name);
/* allocate buffer and creare a stream from an existing file */
u8* MemStreamOpen(MEM_STREAM *str, LPCTSTR name);
/* allocate buffer and create a stream from a memory buffer */
u8* MemStreamOpen(MEM_STREAM *str, u8* data, int size);

/* seek around the stream */
int MemStreamSeek(MEM_STREAM *str, int pos, int mode);

/* return a byte from the stream (EOF when the stream is terminated) */
int MemStreamReadByte(MEM_STREAM *str);
/* read a sequence of bytes */
int MemStreamRead(MEM_STREAM *str, void *read, int size);
/* write a byte to the stream and reallocate buffer if it's necessary */
void MemStreamWriteByte(MEM_STREAM *str, u8 val);
/* write a sequence of bytes, internally uses MemStreamWriteByte */
int MemStreamWrite(MEM_STREAM *str, void *write, int size);
