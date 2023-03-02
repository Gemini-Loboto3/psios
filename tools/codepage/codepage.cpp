// codepage.cpp : definisce il punto di ingresso dell'applicazione console.
//

#include "stdafx.h"
#include "glib.h"
#include <vector>

typedef struct tagUnicodeChar
{
	u16 code;
	u16 ucs;
} UNICODE_CHAR;

typedef struct tagRange
{
	int min, max;
	int mini, maxi;
} RANGE;

void init_array(LPCTSTR name, FILE *out)
{
	GString str;
	str.Format(_T("%s[]=\r\n{\r\n"));
	WriteUtf8(str,out);
}

void append_array(LPCTSTR val, int col, FILE *out)
{
	GString str;

	col%=16;

	if(col==0) str=_T("\t");
	if(col==15)
	{
		str+=val;
		str+=+_T(",\r\n");
	}
	else
	{
		str+=val;
		str+=_T(",");
	}

	WriteUtf8(str,out);
};

void close_array(FILE *out)
{
	WriteUtf8(_T("};\r\n"),out);
}

void cptbl_to_c(LPCTSTR infile, LPCTSTR outname, LPCTSTR cp_name)
{
	std::vector<UNICODE_CHAR> ch;
	std::vector<RANGE> range;
	TCHAR *text, *next;
	wchar_t *token;
	UNICODE_CHAR ucs;
	GString line;

	StoreAsciiText(infile,text);
	for(next=text; *next; )
	{
		line.Empty();
		while(*next!=0xA && *next!=0)
		{
			line+=*next;
			next++;
		}
		next++;

		token=_tcstok((wchar_t *)line.GetBuffer(),_T("\t"));
		for(u32 i=0, val; token; i++)
		{
			// skip comments
			if(*token=='#')
			{
				// get next
				token=_tcstok(NULL,_T("\t"));
				continue;
			}
			// get cases
			switch(i)
			{
			case 0:
				val=0;
				_stscanf(token,_T("0x%x"),&val);
				ucs.code=val;
				break;
			case 1:
				val=0;
				_stscanf(token,_T("0x%x"),&val);
				ucs.ucs=val;

				ch.push_back(ucs);

				break;
			}
			// get next
			token=_tcstok(NULL,_T("\t"));
		}
	}
	delete[] text;

	RANGE rng;
	// define ranges
	for(int i=0, cnt=(int)ch.size(), status=0; i<cnt; i++)
	{
		if(status==0)
		{
			rng.min=ch[i].code;
			rng.mini=i;
			//range_min.push_back(ch[i].code);
			status++;
		}
		if(status==1)
		{
			if(i+1>=cnt)
			{
				rng.max=ch[i].code;
				rng.maxi=i;
				//range_max.push_back(ch[i].code);
				range.push_back(rng);
				break;
			}
			u16 cur=ch[i].code;
			u16 nex=ch[i+1].code;
			// there's a gap
			if(cur+0x40<nex)
			{
				while(1)
				{
					if(cur+0x40<ch[i+1].code) break;
					i++;
				}
				status++;
			}
		}
		if(status==2)
		{
			rng.max=ch[i].code;
			rng.maxi=i;
			//range_max.push_back(ch[i].code);
			range.push_back(rng);
			status=0;
		}
	}

	FILE *tbl_out=_tfopen(outname,_T("wb+"));
	GString str;
	// write tables now
	for(int i=0, cnt=(int)range.size(); i<cnt; i++)
	{
		str.Format(_T("u16 %s_%.4X_%.4X[]=\r\n{\r\n"),cp_name,range[i].min,range[i].max);
		WriteUtf8(str,tbl_out);

		for(int j=range[i].mini, row=0; j<=range[i].maxi; j++, row++)
		{
			for(u16 c=ch[j].code; j<range[i].maxi && c+1<ch[j+1].code; c++, row++)
			{
				str.Format(_T("OUTRNG"));
				append_array(str,row,tbl_out);
			}

			str.Format(_T("0x%.4X"),ch[j].ucs);
			append_array(str,row,tbl_out);
		}

		WriteUtf8(_T("\r\n};\r\n\r\n"),tbl_out);
	}

	str.Format(_T("\r\nUNICODE_TABLE %s[]=\r\n{\r\n"),cp_name);
	WriteUtf8(str,tbl_out);
	for(int i=0, cnt=(int)range.size(); i<cnt; i++)
	{
		str.Format(_T("\t{ENCTYPE_RANGE,\t0x%.4X,\t0x%.4X,\tbsizeof(%s_%.4X_%.4X),\t%s_%.4X_%.4X},	// 0x%.4X-0x%.4X\r\n"),range[i].min,range[i].max,
			cp_name,range[i].min,range[i].max,
			cp_name,range[i].min,range[i].max,
			range[i].min,range[i].max);
		WriteUtf8(str,tbl_out);
	}
	WriteUtf8(_T("};\r\n"),tbl_out);

	fclose(tbl_out);
}

int _tmain(int argc, _TCHAR* argv[])
{
	cptbl_to_c(_T("CP874.TXT"),_T("cp874.c"),_T("cp874"));
	cptbl_to_c(_T("CP932.TXT"),_T("cp932.c"),_T("cp932"));
	cptbl_to_c(_T("CP936.TXT"),_T("cp936.c"),_T("cp936"));
	cptbl_to_c(_T("CP949.TXT"),_T("cp949.c"),_T("cp949"));
	cptbl_to_c(_T("CP950.TXT"),_T("cp950.c"),_T("cp950"));
	cptbl_to_c(_T("CP1250.TXT"),_T("cp1250.c"),_T("cp1250"));
	cptbl_to_c(_T("CP1251.TXT"),_T("cp1251.c"),_T("cp1251"));
	cptbl_to_c(_T("CP1252.TXT"),_T("cp1252.c"),_T("cp1252"));
	cptbl_to_c(_T("CP1253.TXT"),_T("cp1253.c"),_T("cp1253"));
	cptbl_to_c(_T("CP1254.TXT"),_T("cp1254.c"),_T("cp1254"));
	cptbl_to_c(_T("CP1255.TXT"),_T("cp1255.c"),_T("cp1255"));
	cptbl_to_c(_T("CP1256.TXT"),_T("cp1256.c"),_T("cp1256"));
	cptbl_to_c(_T("CP1257.TXT"),_T("cp1257.c"),_T("cp1257"));
	cptbl_to_c(_T("CP1258.TXT"),_T("cp1258.c"),_T("cp1258"));

	return 0;
}

