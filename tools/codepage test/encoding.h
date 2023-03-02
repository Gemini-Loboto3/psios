#include "types.h"

#define OUTRNG					'?'

#define bsizeof(argument)		(sizeof(argument)/sizeof(*(argument)))
#define DIMENSION_OF(argument)	bsizeof((argument))

typedef struct tagUnicodeConv
{
	u16 chr;		// locale symbol
	u16 ucs;		// unicode symbol
} UNICODE_CONV;

typedef struct tagUnicodeTable
{
	u16 type;		// see ENCODING_TYPE
	u16 min, max;	// symbol table range, for faster lookup
	u16 count;		// symbol count (used only if type=ENCTYPE_MATCH)
	u16 *data;		// encoding data, content varies with 'type'
} UNICODE_TABLE;

typedef struct tagEncodeTable
{
	const int count;			// how many entries the table has
	const UNICODE_TABLE* tbl;	// the array of tables
} ENCODE_TABLE;

enum ENCODING_TYPE
{
	ENCTYPE_DIRECT,	// direct add/sub, no lookup necessary
	ENCTYPE_RANGE,	// correspondences
	ENCTYPE_MATCH	// lookup tables
};

enum LOCALE_TYPE
{
	CODEPAGE_874,	// Thai
	CODEPAGE_932,	// Japanese
	CODEPAGE_936,	// Chinese (simplified) (PRC, Singapore)
	CODEPAGE_949,	// Korean
	CODEPAGE_950,	// Chinese (traditional) (Taiwan, Hong Kong)
	CODEPAGE_1250,	// Latin (Central European languages)
	CODEPAGE_1251,	// Cyrillic
	CODEPAGE_1252,	// Latin (Western European languages)
	CODEPAGE_1253,	// Greek
	CODEPAGE_1254,	// Turkish
	CODEPAGE_1255,	// Hebrew
	CODEPAGE_1256,	// Arabic
	CODEPAGE_1257,	// Latin (Baltic languages)
	CODEPAGE_1258	// Vietnamese
};

/*
	Get an input character and convert it to UCS. 'encoding' specifies
	the codepage to which the input char corresponds to. Encodings are
	listed in the enum 'ENCODING_TYPE'.
	Return the UCS converted character or '?' if the conversion failed.
*/
u16 char_to_ucs(const u16 ch, const int encoding);

/*
	Get a UTF8 string as input and parse an encoded symbol to UCS.
	Return byte size of the UTF8 symbol. Return 0 if parsing failed.
*/
int utf8_to_ucs(const u8 *str, u32 *ucs);
