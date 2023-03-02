#pragma once
#include <vector>

using namespace std;

enum FONTSET_TYPE
{
	FT_REFERENCE,
	FT_INDEX
};

typedef struct tagFontGlyph
{
	wchar_t ucs;
	u8 w, h;
	u8 x, y;
	int size;
	u32 data[16];	// should hold all the 16x16 glyph
} FONT_GLYPH;

typedef struct tagGlyphRef
{
	u8 w, h;
	u8 x, y;
} GLYPH_REF;

typedef struct tagFontHeader
{
	u32 magic;		// always "GFNT"
	u16 version;	// should be 1
	u16 count;		// char set size
} FONT_HEADER;

typedef struct tagFontSetRange
{
	wchar_t min, max;
	u16 type;
	u16 count;
	u32 ptr;
	vector<wchar_t> idx;
} FONT_SET_RANGE;

typedef struct tagFontSetData
{
	u16 min, max;
	u32 ptr:24;
	u32 type:8;
} FONT_SET_DATA;

class CFont
{
public:
	CFont();
	~CFont();

	void AddGlyph(FONT_GLYPH* glyph);
	void Save(LPCTSTR filename);

	vector<FONT_GLYPH> g;
};
