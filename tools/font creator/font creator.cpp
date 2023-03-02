// font creator.cpp : definisce il punto di ingresso dell'applicazione console.
//
#include "stdafx.h"
#include "glib.h"
#include <vector>
#include "quantize.h"

RGBQUAD grey_pal[]=
{
	{255,255,255},
	{8,8,8},
	{64,64,64},
	{160,160,160},
	{255,0,255},
	{255,0,255},
	{255,0,255},
	{255,0,255},
	{255,0,255},
	{255,0,255},
	{255,0,255},
	{255,0,255},
	{255,0,255},
	{255,0,255},
	{255,0,255},
	{255,0,255}
};

typedef struct tagGImageInfo
{
	RGBQUAD last_c;				///< for GetNearestIndex optimization
	BYTE	last_c_index;
	bool	last_c_isvalid;
} GIMAGEINFO;

u32 GetImgCrc32(Image &img)
{
	return GetCrc32(img.image,PadWidth(img.width,img.depth)*img.height);
}

bool IsPictureEqual(Image &img, u32 crc)
{
	if(GetImgCrc32(img)==crc) return true;

	return false;
}

#include "font.h"
#include "bitstream.h"

BYTE GetNearestIndex(Image &img, RGBQUAD c, GIMAGEINFO &info)
{
	// <RJ> check matching with the previous result
	if(info.last_c_isvalid && (*(long*)&info.last_c == *(long*)&c)) return info.last_c_index;
	info.last_c = c;
	info.last_c_isvalid = true;

	BYTE* iDst=(BYTE*)img.palette;
	int distance=200000;
	int j=0;
	for(int i=0, l=0; i<(1<<img.depth); i++, l+=sizeof(RGBQUAD))
	{
		int k=(iDst[l]-c.rgbBlue)*(iDst[l]-c.rgbBlue)+
			(iDst[l+1]-c.rgbGreen)*(iDst[l+1]-c.rgbGreen)+
			(iDst[l+2]-c.rgbRed)*(iDst[l+2]-c.rgbRed);
		if(k==0) {j=i;break;}
		if(k<distance) {distance=k;j=i;}
	}
	info.last_c_index=(BYTE)j;
	return (BYTE)j;
}

RGBQUAD BlindGetPixelColor(Image &img, int x, int y)
{
	RGBQUAD rgb;
	u32 p=img.GetPixelIndex(x,y);
	memcpy(&rgb,&p,sizeof(RGBQUAD));
	return rgb;
}

inline void BlindSetPixelColor(Image &dest, int x, int y, RGBQUAD c, GIMAGEINFO &info)
{
	dest.SetPixelAt(x,y,GetNearestIndex(dest,c,info));
}

bool DecreaseBpp(Image &img, Image &dest, int nbit, bool errordiffusion, RGBQUAD* ppal)
{
	int er, eg, eb;
	RGBQUAD c, ce;

	GIMAGEINFO info={0,0,false};
	dest.Create(img.width,img.height,nbit,ppal);

	for(int y=0; y<img.height; y++)
	{
		for(int x=0; x<img.width; x++)
		{
			if(!errordiffusion)
				BlindSetPixelColor(dest,x,y,BlindGetPixelColor(img,x,y),info);
			else
			{
				c=BlindGetPixelColor(img,x,y);
				BlindSetPixelColor(dest,x,y,c,info);

				ce=BlindGetPixelColor(dest,x,y);
				er=(int)c.rgbRed-(int)ce.rgbRed;
				eg=(int)c.rgbGreen-(int)ce.rgbGreen;
				eb=(int)c.rgbBlue-(int)ce.rgbBlue;

				u32 p=img.GetPixelIndex(x+1,y);
				memcpy(&c,&p,sizeof(RGBQUAD));
				c.rgbRed=(BYTE)min(255L,max(0L,(long)c.rgbRed+((er*7)/16)));
				c.rgbGreen=(BYTE)min(255L,max(0L,(long)c.rgbGreen+((eg*7)/16)));
				c.rgbBlue = (BYTE)min(255L,max(0L,(long)c.rgbBlue+((eb*7)/16)));
				img.SetPixelAt(x+1,y,c.rgbBlue|(c.rgbGreen<<8)|(c.rgbRed<<16));
				int coeff=1;
				for(int i=-1; i<2; i++)
				{
					switch(i)
					{
					case -1:
						coeff=2; break;
					case 0:
						coeff=4; break;
					case 1:
						coeff=1; break;
					}
					p=img.GetPixelIndex(x+i,y+1);
					memcpy(&c,&p,sizeof(RGBQUAD));
					c.rgbRed=(BYTE)min(255L,max(0L,(long)c.rgbRed+((er*coeff)/16)));
					c.rgbGreen=(BYTE)min(255L,max(0L,(long)c.rgbGreen+((eg*coeff)/16)));
					c.rgbBlue=(BYTE)min(255L,max(0L,(long)c.rgbBlue+((eb*coeff)/16)));
					img.SetPixelAt(x+i,y+1,c.rgbBlue|(c.rgbGreen<<8)|(c.rgbRed<<16));
				}
			}
		}
	}

	return true;
}

void ProcessFont(CFont &font, LPCTSTR bitmap, const int min, const int max, int unmap[2], int unmap_code=-1)
{
	Image img, /*unmap,*/ ch, ch_dither;
	std::vector<u16> index;
	u32 unmap_crc;

	img.LoadFromFile(bitmap);
	ch.Create(16,16,4,img.palette);
	ch.BitBlit(&img,unmap[0],unmap[1],16,16,0,0,0);
	unmap_crc=GetImgCrc32(ch);

	//CQuantizer q(4,8);
	//q.SetColorTable(grey_pal);
	//q.ProcessImage(img);
	//DecreaseBpp(img,ch_dither,4,false,grey_pal);
	//ch_dither.SaveBitmap(_T("test.bmp"));

	//int fsize=0;
	//FILE *file=_tfopen(_T("font__.bin"),_T("wb+"));
	for(int i=0, k=min; i<max-min+1; i++, k++)
	{
		ch.BitBlit(&img,i%16*16,i/16*16,16,16,0,0,0);
		if(!IsPictureEqual(ch,unmap_crc) || k==unmap_code)
		{
			FONT_GLYPH g;
			//index.push_back(i);
			//index.push_back((u16)ftell(file));

			int left=0, right=0, top=0, bottom=0;

			// check left border
			for(int x=0; x<16; x++)
			{
				for(int y=0; y<16; y++)
				{
					if(ch.GetPixelAt(x,y))
					{
						left=x;
						x=16;	// kill loop
						break;
					}
				}
			}
			// check right border
			for(int x=15; x>=0; x--)
			{
				for(int y=15; y>=0; y--)
				{
					if(ch.GetPixelAt(x,y))
					{
						right=x+1;
						x=0;	// kill loop
						break;
					}
				}
			}
			// check top border
			for(int y=0; y<16; y++)
			{
				for(int x=0; x<16; x++)
				{
					if(ch.GetPixelAt(x,y))
					{
						top=y;
						y=16;	// kill loop
						break;
					}
				}
			}
			// check botton border
			for(int y=15; y>=0; y--)
			{
				for(int x=15; x>=0; x--)
				{
					if(ch.GetPixelAt(x,y))
					{
						bottom=y+1;
						y=0;	// kill loop
						break;
					}
				}
			}

			memset(&g,0,sizeof(g));
			// determine size now
			g.w=right-left;
			g.h=bottom-top;
			// misc attributes
			g.x=0;	// this should always be ZERO
					// we don't use monospace fonts
			g.y=top;
			g.ucs=k;

			// override special cases
			switch(k)
			{
			case ' ':
			case 0xA0:		// no-break space
				g.w=3;
				break;
			case 0x3000:	// wide japanese space
				g.w=12;
				break;
			}

			//if(k==0x30DA)
			//	k=k;

			// skip chars don't need a glyph
			if(g.w==0 || g.h==0)
			{
				g.size=0;
				font.AddGlyph(&g);
				//fwrite(&g,4,1,file);
				continue;
			}
			//fwrite(&g,6,1,file);
			BIN_BUFFER buf;
			Buf_Bopen(&buf,(u8*)g.data,sizeof(g.data));
			for(int y=0; y<g.h; y++)
			{
				for(int x=0; x<g.w; x++)
					Buf_Bwrite_M(ch.GetPixelAt(left+x,top+y),2,&buf);
			}
			Buf_Bflush(&buf);
			//g.size=align(Buf_Btell(&buf),2);
			g.size=Buf_Btell(&buf);	// do not align, it's useless
			_tprintf(_T("Appending %d (%c)!\n"),i,(wchar_t)k);

			//fsize+=g.size;
			//fwrite(&g,6,1,file);
			//fwrite(&g.data,g.size,1,file);
			//if(ftell(file)&1) fwrite("\0",1,1,file);
			font.AddGlyph(&g);
		}
	}
	//for(int i=0; i<index.size(); i++) fwrite(&index[i],2,1,file);
	//fclose(file);
}

int _tmain(int argc, _TCHAR* argv[])
{
	CFont font;

	int unmap[2]={16,0};
	ProcessFont(font,_T("..\\..\\unifonts\\segoe.png"),0,0x2FFF,unmap,0x7F);
	// asian fonts
	unmap[0]=0,unmap[1]=3376;
	ProcessFont(font,_T("..\\..\\unifonts\\asian_0.png"),0x3000,0x5FFF,unmap,-1);
	unmap[0]=0,unmap[1]=16304;
	ProcessFont(font,_T("..\\..\\unifonts\\asian_1.png"),0x6000,0x9FFF,unmap,-1);
	unmap[0]=0,unmap[1]=0;
	ProcessFont(font,_T("..\\..\\unifonts\\asian_2.png"),0xA000,0xFFFF,unmap,-1);

	font.Save(_T("font.bin"));

	return 0;
}

