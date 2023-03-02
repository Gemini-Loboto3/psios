#include "encoding.h"

extern UNICODE_TABLE cp932[];
//extern UNICODE_TABLE cp1253[];

static const UNICODE_TABLE ascii=
{
	ENCTYPE_DIRECT,	// type
	0,0xFF,			// min-max
	256,			// count
	0				// data
};

static const ENCODE_TABLE enc_tables[]=
{
	{1,&ascii},	// CP1252 (ASCII)
	{46,cp932},	// CP932 (Shift-JIS)
	//{1,cp1253}	// CP1253 (Greek [Windows])
};

static u16 search_range(const UNICODE_TABLE *table, const u16 ch)
{
	return table->data[ch-table->min];
}

static u16 search_match(const UNICODE_TABLE *table, const u16 ch)
{
	register u32 i, j;
	register UNICODE_CONV *data=(UNICODE_CONV*)table->data;

	// search match in the lookup table
	for(i=0, j=table->count; i<j; i++)
		if(data[i].chr==ch)
			return data[i].ucs;
	// nothing was found
	return -1;
}

// get the unicode symbol from a locale encoding (only cp932 is supported so far)
u16 char_to_ucs(const u16 ch, const int encoding)
{
	int i, j;
	u32 c;
	const UNICODE_TABLE *t;
	const ENCODE_TABLE *e=&enc_tables[encoding];

	for(i=0, j=e->count; i<j; i++)
	{
		// fast reference to table
		t=&e->tbl[i];
		// skip unnecessary ranges
		if(!(ch>=t->min && ch<=t->max)) continue;

		// select lookup method
		switch(t->type)
		{
		case ENCTYPE_DIRECT:
			return (ch-t->min)+(u32)(t->data);
		case ENCTYPE_RANGE:
			return search_range(t,ch);
		case ENCTYPE_MATCH:
			if((c=search_match(t,ch))==(u16)-1) return '?';
			return c;
		}
	}

	// return question mark if no match could be found
	return '?';
}

int utf8_to_ucs(const u8 *str, u32 *ucs)
{
	//asserte(ucs);

	// 1-byte code
	if(*str<0x80)
	{
		*ucs=*str;
		return 1;
	}
	// invalid
	else if(*str<0xC0) *ucs='?';
	// 2-byte code
	else if(*str<0xE0)
	{
		*ucs=((str[0]&0x1F)<<6)
			|((str[1]&0x3F));
		return 2;
	}
	// 3-byte code
	else if(*str<0xF0)
	{
		*ucs=((str[0]&0x0F)<<12)
			|((str[1]&0x3F)<<6)
			|((str[2]&0x3F));
		return 3;
	}
	// 4-byte code
	else if(*str<0xF8)
	{
		*ucs=((str[0]&0x07)<<18)
			|((str[1]&0x3F)<<12)
			|((str[2]&0x3F)<<6)
			|((str[3]&0x3F));
		return 4;
	}

	// nothing could be parsed
	return 0;
}
