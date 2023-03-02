#include "stdafx.h"
#include "font.h"
#include "bitstream.h"

typedef struct tagUcsRange
{
	int idx;
	int count;
	u16 min, max;
	bool ref;
} UCS_RANGE;

CFont::CFont()
{
}

CFont::~CFont()
{
}

void CFont::AddGlyph(FONT_GLYPH* glyph)
{
	g.push_back(*glyph);
}

void CFont::Save(LPCTSTR filename)
{
	vector<UCS_RANGE> cmp;
	UCS_RANGE cmp_c={0,0,0,0,false};

	// create range lists
	cmp_c.min=g[0].ucs;		// init range list
	for(int i=0; i<(int)g.size()-1; i++)
	{
		// check consecutive order
		if(g[i].ucs+1<g[i+1].ucs)
		{
			// add last range
			cmp_c.max=g[i].ucs;
			cmp_c.count=i-cmp_c.idx+1;
			cmp.push_back(cmp_c);

			// append new range
			cmp_c.min=g[i+1].ucs;
			cmp_c.idx=i+1;
			continue;
		}
	}
	// close range list
	cmp_c.max=g[g.size()-1].ucs;
	cmp_c.count=(int)g.size()-cmp_c.idx;
	cmp.push_back(cmp_c);

	// analyze lists
	for(int i=0; i<(int)cmp.size(); i++)
	{
		// check for fragmentation
		if(cmp[i].count<0x20)
		{
			// exclude from range tables
			cmp[i].ref=true;
		}
	}

	// create optimized tables
	vector<FONT_SET_RANGE> set_rng;
	FONT_SET_RANGE set_r;
	for(int i=0, s=(int)cmp.size(); i<s; i++)
	{
		set_r.idx.clear();
		if(!cmp[i].ref)
		{
			set_r.min=cmp[i].min;
			set_r.max=cmp[i].max;
			set_r.count=cmp[i].count;
			set_r.type=FT_REFERENCE;
		}
		else
		{
			set_r.min=cmp[i].min;
			// create fragmentation indexes
			for(set_r.count=0; i<s && cmp[i].ref; i++)
			{
				set_r.max=cmp[i].max;
				set_r.count+=cmp[i].count;
				for(int j=0; j<cmp[i].count; j++)
					set_r.idx.push_back(j+cmp[i].min);
			}
			i--;	// adjust
			set_r.type=FT_INDEX;
		}
		set_rng.push_back(set_r);
	}

	// --- FINALLY, WRITE THE FONT FILE! ---
	FILE *file=_tfopen(filename,_T("wb+"));

	int ptr_cnt=(int)set_rng.size();
	FONT_SET_DATA *ptr=new FONT_SET_DATA[ptr_cnt];
	fseek(file,sizeof(FONT_HEADER)+ptr_cnt*sizeof(*ptr),SEEK_SET);

	int k=0;
	// write sections
	for(int i=0; i<ptr_cnt; i++)
	{
		ptr[i].ptr=(u32)ftell(file);
		ptr[i].min=set_rng[i].min;
		ptr[i].max=set_rng[i].max;
		ptr[i].type=set_rng[i].type;
		if(ptr[i].type==FT_INDEX)
		{
			u32* sptr=new u32[set_rng[i].count];
			// write count
			fwrite(&set_rng[i].count,2,1,file);
			fwrite("\0\0",2,1,file);	// dword align
			// cache room for pointers
			u32 cur=ftell(file);
			fwrite(sptr,set_rng[i].count,sizeof(*sptr),file);
			// write indexes
			for(int j=0; j<set_rng[i].count; j++)
				fwrite(&set_rng[i].idx[j],2,1,file);
			for(int j=0; j<set_rng[i].count; j++, k++)
			{
				// avoid unnecessary data
				if(g[k].w==0 && g[k].h==0)
				{
					sptr[j]=NULL;
					continue;
				}
				sptr[j]=ftell(file);
				GLYPH_REF r;
				r.w=g[k].w;
				r.h=g[k].h;
				// otherwise write a full glyph
				r.x=2;	// 2bpp
				r.y=g[k].y;
				fwrite(&r,sizeof(r),1,file);
				fwrite(g[k].data,g[k].size,1,file);
			}

			fseek(file,cur,SEEK_SET);
			fwrite(sptr,set_rng[i].count,sizeof(*sptr),file);
			delete[] sptr;
			fseek(file,0,SEEK_END);
		}
		else
		{
			u32 cur=ftell(file);
			u32* sptr=new u32[set_rng[i].count];
			fwrite(sptr,set_rng[i].count,sizeof(*sptr),file);
			for(int j=0; j<set_rng[i].count; j++, k++)
			{
				// avoid unnecessary data
				if(g[k].w==0 && g[k].h==0)
				{
					sptr[j]=NULL;
					continue;
				}
				sptr[j]=ftell(file);
				GLYPH_REF r;
				r.w=g[k].w;
				r.h=g[k].h;
				r.x=2;	// 2bpp
				r.y=g[k].y;
				fwrite(&r,sizeof(r),1,file);
				fwrite(g[k].data,g[k].size,1,file);
			}
			fseek(file,cur,SEEK_SET);
			fwrite(sptr,set_rng[i].count,sizeof(*sptr),file);
			delete[] sptr;
			fseek(file,0,SEEK_END);
		}
		// maintain dword alignment
		fseek(file,align(ftell(file),4),SEEK_SET);
	}

	FONT_HEADER hdr;
	hdr.magic='G'|('F'<<8)|('N'<<16)|('T'<<24);
	hdr.version=1;
	hdr.count=(u16)g.size();

	fseek(file,0,SEEK_SET);
	fwrite(&hdr,sizeof(hdr),1,file);
	fwrite(ptr,ptr_cnt,sizeof(*ptr),file);
	fclose(file);

	delete[] ptr;
}
