#ifndef __FONT_H
#define __FONT_H

enum FONT_TYPE
{
	FT_REFERENCE,
	FT_INDEX
};

typedef struct tagFontHeader
{
	u32 magic;
	u16 version;
	u16 count;
} FONT_HEADER;

typedef struct tagFontPointer
{
	u16 min, max;
	u32 ptr:24;
	u32 type:8;
} FONT_POINTER;

typedef struct tagFontData
{
	u8 *data;
	FONT_POINTER *ptr;
} FONT_DATA;

typedef struct tagFontIndex
{
	u32 count;
	u32 ptr[1];
} FONT_INDEX;

typedef struct tagFontGlyph
{
	u8 w, h;
	u8 x, y;
	u16 data[1];
} FONT_GLYPH;

typedef struct tagFontBitread
{
	u8* data;
	int bit;
	u32 byte;
} FONT_BITREAD;

/* create a memory handle to the font */
void *FontOpen(u8* data, FONT_DATA *fnt);

/* retrieve glyph data */
FONT_GLYPH *FontGetGlyph(const FONT_DATA *fnt, const u32 index);

/* expand glyph data */
void FontExpandGlyph(FONT_GLYPH *g, u32 *dst);

#endif	// __FONT_H
