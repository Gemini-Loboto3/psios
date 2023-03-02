#ifndef __LZS_H
#define __LZS_H

#include "common.h"

/* ---------------------------------------------------------------------------
 * - GLOBAL DEFINITIONS 
 * ---------------------------------------------------------------------------
 */
enum COMPRESSIONS
{
	PERSONA_LZSS,
	TOSE_LZSS,
	NINTENDO_LZ77,
	NINTENDO_HUFFMAN,
	LUFIA_LZSS,
	SRTF_LZSS
};

typedef struct tagCmpHeader
{
	u32 size:28;	// decompression size
	u32 huf:1;		// special field
	u32 type:3;		// check COMPRESSIONS
} CMP_HEADER;

int Decompress(u8* src, u8* dest);

// Nintendo
int DecLz77N(u8 *srcp, u32 size, u8 *dstp);
int DecHuffN(u8 *srcp, int size, u8 *dstp, u8 huffBitSize);
// Persona 2
void DecLzssP(u8* dest, u8* source, int dec_size);
// TOSE
extern int DecLzssT(u8* source, u8* dest, int size);
// Lufia
void DecLzssL(u8* srcp, int size, u8 *dstp);
// Super Robot Taisen F
void DecLzssS(u8* dest, u8* source, int dec_size);

#endif // __LZS_H
