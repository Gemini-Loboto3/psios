#include <windows.h>
#include <stdio.h>
#include "types.h"
#include "encoding.h"

extern int read_cp932(u8 *str, u16 *ch);

int main()
{
	//char test[11]="—Í‚©ƒJ‚Q‚`";
	u8 test[]={0xc3,0xa0,0xc3,0xa8,0xe5,0x8a,0x9b,0x61,0x62,0x61,0x5f,0x5f,0x31,0x21,0x22,0xc2,0xa3,0};
	wchar_t test_ucs[22];

	wcslen(L"");

	wchar_t* t=test_ucs;
	u8* r=(u8*)test;
	for(; *r; t++)
	{
		//u16 sj_ch;
		//r+=read_cp932((u8*)r,&sj_ch);
		//*t=char_to_ucs(sj_ch,1);
		u32 ucs;
		r+=utf8_to_ucs(r,&ucs);
		*t=ucs;
	}
	*t='\0';

	return 0;
}
