/* ---------------------------------------------------------------------------
 * - (C) Computer o Tsukau Neko. All Rights Reserved.
 * -
 * - Project:	Lzs compressor & decompressor
 * -
 * - Name:		lzs.c
 * -
 * - Author:	Mat
 * -
 * - Description:
 * - ------------
 * - Manages compression and decompression using a lzs algorithm.
 * ---------------------------------------------------------------------------
 */
#include "compress.h"

int Decompress(u8* src, u8* dest)
{
	CMP_HEADER head;
	memcpy(&head,&src[0],sizeof(CMP_HEADER));
	src+=4;

	switch(head.type)
	{
	case PERSONA_LZSS:
		DecLzssP(dest,src,head.size);
		break;
	case TOSE_LZSS:
		DecLzssT(src,dest,head.size);
		break;
	case NINTENDO_LZ77:		// LZ77
		DecLz77N(src,head.size,dest);
		break;
	case NINTENDO_HUFFMAN:	// Huffman
		DecHuffN(src,head.size,dest,4<<head.huf);
		break;
	case LUFIA_LZSS:
		DecLzssL(src,head.size,dest);
		break;
	case SRTF_LZSS:
		DecLzssS(dest,src,head.size);
		break;
	}
}

//==================================================================================
// Expanding plain huffman data
//==================================================================================
struct CodeNode
{
	u8 value;
	u8 codeSize;
	u16 code;
};

void DecHuffW(u8 *in, u8 *out, int dataSize)
{
	u8 ch;
	u32 mask, maskSize=0;
	int x, y, offset, codesUsed;
	struct CodeNode codes[256];

	// decoding table size
	codesUsed=*((u16*)in);
	in+=2;

	// read and store decoding table
	for(x = 0; x < codesUsed; x++, in+=2)
	{
		y=*((u16*)in);
		codes[x].value=(u8)y;
		codes[x].codeSize=(u8)((y>>8)+1);
	}

	// set bit encoding data
	for(x = 0, offset=7; x < codesUsed; x++)
	{
		mask=0;
		for(y = codes[x].codeSize - 1; y >= 0; y--)
		{
			if(offset==7) ch=*in++;
			mask |= ((ch >> offset) & 1) << y;
			offset = (offset - 1) & 7;
		}
		codes[x].code=mask;
	}

	// decompress buffer
	for(x=0, y=0, offset=7, mask=0;;)
	{
		// read new byte if necessary
		if(offset == 7) ch=*in++;
		// update bit stream
		mask <<= 1;
		mask |= (ch >> offset) & 1;
		++maskSize;
		offset = (offset - 1) & 7;

		// skip unnecessary mask values
		while(codes[y].codeSize < maskSize) y++;
		// now search binary match
		while(codes[y].codeSize == maskSize)
		{
			// match found?
			if(codes[y].code == mask)
			{
				// out to buffer
				out[x++]=codes[y].value;
				// decompression is done
				if(x >= dataSize) return;
				// reset bit seek
				mask = 0;
				maskSize = 0;
				y = 0;
				break;
			}
			// search next
			y++;
		}
	}
}

//==================================================================================
// Expanding Lz77 encoded data
//==================================================================================
int DecLz77N(u8 *srcp, u32 size, u8 *dstp)
{
	u32     LZDstCount;                // Expanded data size in bytes
	u32     LZSrcCount;                // Size of data that is targeted for expansion--after processing (bytes)
	u8      compFlags;                 // Flag sequence indicating whether or not compressed
	u8      length;                    // Target data length (3 is added)
	u16     offset;                    // Match data offset -1 (Always 2 or more)
	u8     *startp;                    // Match data start address
	u32     i, j;

	LZSrcCount = 0;
	LZDstCount = 0;
	offset = 0;
	length = 0;

	while (LZDstCount < size)
	{
		compFlags = srcp[LZSrcCount++];
		for (i = 0; i < 8; i++)
		{
			if (compFlags & 0x80)      // Compressed
			{
				length = (srcp[LZSrcCount] >> 4) + 3;   // Upper 4 bits. 3 is added.  
				offset = (srcp[LZSrcCount] & 0x0f) << 8;        // Lower 4 bits. When offset, 11-8th bit
				LZSrcCount++;
				offset |= srcp[LZSrcCount];
				offset++;
				LZSrcCount++;
				compFlags <<= 1;

				// Extraction process
				startp = &dstp[LZDstCount - offset];
				for (j = 0; j < length; j++)
				{
					dstp[LZDstCount++] = startp[j];
				}
			}
			else                       // Not compressed
			{
				dstp[LZDstCount++] = srcp[LZSrcCount++];
				compFlags <<= 1;
			}
			// end when size reached
			if (LZDstCount >= size)
			{
				break;
			}
		}
	}

	// Align to 4-byte boundary
	//   Does not include Data0 used for alignment as data size
	i = 0;
	while ((LZDstCount + i) & 0x3)
	{
		dstp[LZDstCount + i] = 0;
		i++;
	}
	return LZDstCount;
}

//==================================================================================
// Expanding Persona 2 encoded data
//==================================================================================
void DecLzssP(u8* dest, u8* source, int dec_size)
{
	int cmp_pos=0;
	int dec_pos=0;

	u8 len=0;
	u8 flag=0;
	u32 lenght, jump, i;
	short tmp;
	while(dec_pos<dec_size-1)
	{
		if(len==0)
		{
			flag=source[cmp_pos];
			cmp_pos++;
			if((flag&0x80)!=0)	// jump and lenght method
			{
				lenght=flag-0x7D;
				tmp=~source[cmp_pos];
				cmp_pos++;
				jump=dec_pos+tmp;
				for(i=0; i<lenght; i++) dest[dec_pos++]=dest[jump++];
			}
			else
			{
				lenght=flag+1;
				zmemcpy(dest+dec_pos,source+cmp_pos,lenght);
				dec_pos+=lenght;
				cmp_pos+=lenght;
			}
		}
	}
}

//==================================================================================
// Expanding Huffman encoded data
//==================================================================================
int DecHuffN(u8 *srcp, int size, u8 *dstp, u8 huffBitSize)
{
    u16     treeSize;                  // HuffTree size * 2
    u32     HuffSrcCount;              // Size of data that is targeted for expansion--after processing (bytes)
    u32     HuffDstCount;              // Extracted data
    u32     currentBitStream;
    u8      currentBit;
    u16     i;
    u16     treeAddr;
    u8      treeData;
    u8      preTreeData;
    u8      isUpper4bits = 0;

    treeSize = ((*srcp) + 1) * 2;
    HuffSrcCount = treeSize;           // Get beginning of data
    HuffDstCount = 0;
    treeAddr = 1;
    preTreeData = srcp[1];

    //  Extraction process
    while (1)                          // until return
    {
        currentBitStream = srcp[HuffSrcCount++];
        currentBitStream |= srcp[HuffSrcCount++] << 8;
        currentBitStream |= srcp[HuffSrcCount++] << 16;
        currentBitStream |= srcp[HuffSrcCount++] << 24;

        for (i = 0; i < 32; i++)
        {
            currentBit = (u8)(currentBitStream >> 31);
            currentBitStream <<= 1;

            if (((currentBit == 0) && (preTreeData & 0x80)) ||
                ((currentBit == 1) && (preTreeData & 0x40)))
            {
                if (huffBitSize == 8)
                {
                    treeData = srcp[(treeAddr * 2) + currentBit];       // code data
                    dstp[HuffDstCount++] = treeData;
                }
                else if (isUpper4bits)
                {
                    treeData |= (srcp[(treeAddr * 2) + currentBit]) << 4;
                    dstp[HuffDstCount++] = treeData;
                    isUpper4bits = 0;
                }
                else
                {
                    treeData = srcp[(treeAddr * 2) + currentBit];
                    isUpper4bits = 1;
                }

                if (HuffDstCount >= size)
                {
                    return HuffDstCount;
                }

                treeAddr = 1;
                preTreeData = srcp[1];
            }
            else
            {
                preTreeData = srcp[(treeAddr * 2) + currentBit];        // offset data
                treeAddr += (preTreeData & 0x3f) + 1;
            }
        }
    }
	return 1;
}

//==================================================================================
// Expanding Lufia encoded data
//==================================================================================
void DecLzssL(u8* srcp, int size, u8 *dstp)
{
	u16		Bits;
	u8		LzByte;
	u8		Buf;
	s16		Offsetting;
	int		Length;
	s16		DestPos;
	int		uPos, cPos, i;

	cPos=0;
	uPos=0;
	Bits=0;
	Offsetting=0;

	while (uPos<size) 
	{
		if (Bits == 0)
		{
			Bits = 0x80;
			LzByte = srcp[cPos++];
		}
		Buf = srcp[cPos++];
		// wenn das byte kleiner als 0x80 ist, dann ist es unkomprimiert
		if ((Buf & 0x80) == 0) dstp[uPos++] = Buf; 
		else
		{
			// jetzt wird das lzbyte befragt, ob das byte eine kompression ist
			if ((LzByte & Bits) == 0) dstp[uPos++] = Buf;
			else
			{
				// die entfernung wird berechnet. sie ist ein negativ wert und wird zur position addiert
				Offsetting = Buf << 8;
				Buf = srcp[cPos++];
	
				Offsetting |= Buf;
				Offsetting = Offsetting >> 4;

				if ((Buf & 0xF) == 0) // wenn die länge null ist, dann kann weiter zurückgegriffen werden 
				{                     // und mehr daten kopiert werden, aber dafür hat man ein weiteres byte
					Buf = srcp[cPos++];
					Length = (Buf & 0x3F)+3;
					Offsetting = ((Offsetting << 2) | (Buf >> 6)) & 0xFFFF;
				} 
				else Length = (Buf &0xF) + 2;

				DestPos = uPos + Offsetting;
				for (i = 0; i < Length; i++) dstp[uPos+i] = dstp[DestPos+i];
				uPos+=Length;
			}
			Bits = Bits >> 1; // und auf das nächste bit setzen
		}
	}
}

//==================================================================================
// Expanding Super Robot Taisen F encoded data
//==================================================================================
static __inline int GetBit(u8* InputBuffer, int *FlagByte, int *InputPos, int *Bit)
{
	int value;
	if(*Bit==0)
	{
		*FlagByte=InputBuffer[*InputPos];
		InputPos[0]++;
		*Bit=0x80;
	}
	value=*FlagByte&*Bit;
	*Bit>>=1;
	return (value == 0 ? 0 : 1);
}

void DecLzssS(u8* dest, u8* source, int dec_size)
{
	int Bit = 0x0;
	int FlagByte = 0;
	int InputPos = 0;
	int OutputPos = 0;
	int i;
	unsigned int length;
	unsigned int distance;

	while(OutputPos<dec_size)
	{
		if(GetBit(source,&FlagByte,&InputPos,&Bit)!=0) dest[OutputPos++] = source[InputPos++];
		else
		{
			if(GetBit(source,&FlagByte,&InputPos,&Bit) != 0)
			{
				distance = (source[InputPos] << 8) |source[InputPos+1];
				distance = abs((signed int)distance >> 3|0xFFFFE000);
				length = (source[InputPos+1] & 0x7) + 2;
				InputPos += 2;
				if (length == 2) length = source[InputPos++] + 1;
			}
			else
			{
				length = GetBit(source,&FlagByte,&InputPos,&Bit) << 1;
				length = (length | GetBit(source,&FlagByte,&InputPos,&Bit)) + 2;
				distance = abs((signed int)source[InputPos++]|0xFFFFFF00);
			}
			for (i = 0; i < (int)length; i++) dest[OutputPos+i] = dest[OutputPos+i-distance];
			OutputPos += length;
		}
	}
}