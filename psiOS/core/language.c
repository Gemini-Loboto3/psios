#include "../core.h"
#include "../encoding/encoding.h"

#define MESSAGE_MAX	256

static u16* messages[MESSAGE_MAX];

int parse_string(u8* str, int index)
{
	u32 ch;
	u16 *dst;
	int pos=0, ret;

	dst=messages[index];
	while(1)
	{
		ch=str[pos++];
		// skip comments
		if(ch==';') while(str[pos]!='\n' && str[pos]!='\0') pos++;
		// hit line break, stop parsing
		if(ch=='\r' && str[pos+1]=='\n') {pos++; break;}
		// hit EOF, return negative result
		if(ch=='\0') return -1;

		// parse a symbol now
		ret=utf8_to_ucs(&str[pos],&ch);
		*dst++=ch;

		if(ret==0)
		{
			DEBUGPRINT(("Parsed unsupported symbol %c"),ch);
			pos++;
		}
		else if(ret==4)
		{
			DEBUGPRINT(("Parsed ucs32 symbol %x"),ch);
			pos+=4;
		}
		else pos+=ret;
	}
}
