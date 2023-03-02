#include <sys/types.h>
#include <sys/file.h>
#include <stdio.h>
#include <kernel.h>			// kernel library
#include <libgte.h>
#include <libgpu.h>
#include "mdec.h"

#define FONT_W	12
#define FONT_H	12

#define TEMP_BUF	0x801EF480
#define FONT_BUF	0x80096000
#define COL_BUFF	0x8000F800
#define MOVIE_ID	0x8000F804
#define CHAR_BUF	0x8000F808
#define TEXT_BUF	0x8000FB80

#define GetPtrULong(x)	*((u_long*)(x))

void FF7Lzss(u_long *src, u_long *dst)
{
}

u_long InitSubtitles(int null, int movie)
{
	u_char *col_buf;
	*((u_long*)MOVIE_ID)=movie;

	if(movie==36) FF7Lzss((u_long*)FONT_BUF,(u_long*)TEXT_BUF);
	else return;
	// decompress font
	FF7Lzss((u_long*)FONT_BUF,(u_long*)TEMP_BUF);
	// define color table
	col_buf=(u_char*)COL_BUFF;
	col_buf[0]=32;
	col_buf[1]=128;
	col_buf[2]=224;

	return *((u_long*)0x800718C8);
}

int CopyCharImage(u_char *image, u_char c, u_long x, int y, int imgx)
{
	int y1, x1, width, pixel, extra;
	u_int line;
	SUB_CHAR *str;
	u_char *timg, *ch, *col_buf=(u_char*)COL_BUFF;
	extra=x>>31;
	x=(x<<1)>>1;

	// first pixel to alter
	image+=(y*16+(x-imgx))*3;
	// retrieve character data
	str=(SUB_CHAR*)CHAR_BUF;
	ch=(u_char*)str+str[c].pos;
	width=str[c].width<<extra;

	// write vertically
	for(x1=0; x1<width; x1++)
	{
		// break when strip border is reached
		if(!(x+x1<imgx+16)) break;
		// write only the pixels contained in this strip
		if(x+x1>=imgx)
		{
			timg=image;
			line=ch[0]|(ch[1]<<8)|(ch[2]<<16);
			// impress vertical line
			for(y1=0; y1<12; y1++)
			{
				pixel=line&3;
				if(pixel!=0)
				{
					timg[0]=timg[1]=timg[2]=col_buf[pixel-1];
					if(extra) timg[3]=timg[4]=timg[5]=col_buf[pixel-1];
				}
				timg+=16*3;
				line>>=2;
			}
		}
		// next line
		ch+=3;
		image+=3<<extra;
		x1+=extra;
	}
	return width;
}

void DrawSubtitle(RECT *area, u_char* image)
{
	int movie=*((int*)0x8000F880);
	int total, i, x, y, frame, c, curx;
	u_char *data, *string;
	SUB_DATA *sub;
	SUB_CHAR *sub_ch=(SUB_CHAR*)CHAR_BUF;

	// check if it's intro or ending
	if(movie!=36) goto do_upload;	// escape if not true

	// get slice width
	curx=area->x/3*2;
	// get pointer to text
	data=(u_char*)TEXT_BUF;

	// text info
	sub=(SUB_DATA*)data;
	total=sub[0].ptr/sizeof(SUB_DATA);

	// current frame value
	frame=*((u_long*)(*((int*)/*(GetSp()+*/0x204/*)*/)+2));
	// search all text entries for a match
	for(i=0; i<total; i++)
	{
		if(frame>=sub[i].start && frame<=sub[i].end)
		{
			// get text
			string=data+sub[i].ptr+1;
			// get centered X
			x=string[-1];
			for(y=16+sub[i].extra*180;; string++)
			{
				// current character of the string
				c=string[0];
				if(c==0xFF) break;		// end of string
				else if(c==0xFE)		// line break
				{
					y+=14;
					x=string[+1];
					string++;
				}
				else if(c==0xFD) x+=3;	// space
				else if(x>=curx-16 && x<curx+16) x+=CopyCharImage(image,c,x|sub[i].extra<<31,y,curx);	// write pixels to image
				else x+=sub_ch[c].width;	// increase x anyway
			}
			// no need to search anymore
			break;
		}
	}
do_upload:
	LoadImage(area,(u_long*)image);
}