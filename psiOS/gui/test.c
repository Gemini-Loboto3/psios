#include "../core.h"
#include "../encoding/wchar.h"
#include "../encoding/encoding.h"
#include "list.h"
#include "pbar.h"

const u8 dte_dictionary[256]={0};
u16 dte_flag;

u8* ReadSymbol(const u8* str, int *dst)
{
	// check if there's anything in the dte cache
	if(dte_flag)
	{
		// fill symbol with dte cache
		*dst=dte_flag;
		// clear cached char
		dte_flag=0;
		// seek forward
		return (u8*)&str[1];
	}
	// check for dte character
	if(*str>=0x80 && *str<=0xF8)
	{
		// fill dte cache with the next symbol to read
		dte_flag=dte_dictionary[*str*2+1];
		// out return value from the dte dictionary
		*dst=dte_dictionary[*str*2];
		// return no string seek
		return (u8*)str;
	}

	// if no DTE op is possible return a literal
	*dst=*str;
	// seek forward
	return (u8*)&str[1];
}

GFX_MAPPING ovr_font[]=
{
//	x	y	u	v	w	h	clut				tage
	0,	0,	0,	0,	256,224,getClut(0,480),		getTPage(0,0,320,0),
	0,	0,	0,	0,	256,224,getClut(0,481),		getTPage(0,0,320,0),
};

static FONT_DATA font;

//u8 width_tbl[]=
//{
////	  ! " # $ % & ' ( ) * + , - . /
//	3,2,2,6,4,7,7,1,3,3,4,5,2,4,2,4,	// 20
////	0 1 2 3 4 5 6 7 8 9 : ; < = > ?
//	4,5,4,4,5,4,4,5,4,4,2,2,4,5,4,4,	// 30
////	@ A B C D E F G H I J K L M N O
//	8,6,4,5,5,4,4,5,5,3,3,5,4,7,6,6,	// 40
////	P Q R S T U V W X Y Z [ \ ] ^ _
//	4,6,5,4,5,5,6,9,6,5,5,2,4,2,5,4,	// 50
////	` a b c d e f g h i j k l m n o
//	2,4,5,4,5,4,4,5,4,1,3,4,1,7,4,5,	// 60
////	p q r s t u v w x y z { | } ~
//	5,5,3,4,3,4,5,7,4,5,4,3,1,3,5,0,	// 70
//	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	// 80
//	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	// 90
//	6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,	// A0
//	6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,	// B0
//	6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,	// C0
//	6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,	// D0
//	6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,	// E0
//	6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6		// F0
//};

u8 font_color[][3]=
{
	{128,128,128},	// white
	{248,48,48},	// red
	{64,192,64},	// green
	{48,48,192},	// blue
	{64,64,64}		// grey
};

void draw_char(const int x, const int y, const u32 c, const u32 color, const GsOT_TAG *ot)
{
	int sub_col;
	SPRT_16 *p=(SPRT_16*)MallocGfx(sizeof(SPRT));
	
	setSprt16(p);
	setXY0(p,x,y);
	// border
	if(color&0x8000)
	{
		setRGB0(p,128,128,128);
		setClut(p,0,FRAME_Y*2+1);
	}
	// color math-based
	else
	{
		sub_col=color&0x7FFF;
		p->r0=font_color[sub_col][0];
		p->g0=font_color[sub_col][1];
		p->b0=font_color[sub_col][2];
		setClut(p,0,FRAME_Y*2);
	}
	setUV0(p,c%16*16,c/16*16);
	AddPrim((void*)ot,p);
}

int get_string_width(const u8* str)
{
	int w=0, max_w=0;

	while(1)
	{
		switch(*str)
		{
		case 0:		// kill process
			if(w>max_w) return w+1;
			return max_w+1;
		case 1:		// change color
			str++;
			break;
		case 10:	// '\n', line break
			if(w>max_w) max_w=w;
			w=0;
			break;
		default:
			w+=FontGetGlyph(&font,*str)->w+1;
		}
		str++;
	}

	return 0;
}

void draw_string_color(int x, int y, const u8* str, int color)
{
	u32 c;
	GsOT_TAG *ot=GetOTag();
	DR_TPAGE *tpage=(DR_TPAGE*)MallocGfx(sizeof(DR_TPAGE));
	int base_x=x, read=1;
	FONT_GLYPH *g;

	while(read)
	{
		switch(*str)
		{
		case 0:		// kill process
			read=0;
			break;
		case 1:		// change color
			color=str[1];
			str++;
			break;
		case 10:	// '\n', line break
			y+=12;
			x=base_x;
			break;
		default:
			g=FontGetGlyph(&font,*str);
			if(g->h > 0)
			{
				c=*str-0x20;
				// regular
				draw_char(x+1,y+1,c,color,ot);
				// border
				draw_char(x,y+1,c,0x8000,ot);
				draw_char(x+2,y+1,c,0x8000,ot);
				draw_char(x+1,y,c,0x8000,ot);
				draw_char(x+1,y+2,c,0x8000,ot);
			}
			// increase spacing
			x+=g->w+1;
		}
		str++;
	}

	setDrawTPage(tpage,0,0,ovr_font->tpage);
	AddPrim(ot,tpage);
}

void draw_string(int x, int y, const u8* str)
{
	draw_string_color(x,y,str,0);
}

/*
	Tests cp932 to ucs conversion for memory card file names.
	Converts a given S-JIS string ('Persona 2 TSUMI' in this case) to unicode
	and then caches the font glyphs for it in VRAM. This is going to be very
	useful for the memory card backup manager.
*/
void mc_test()
{
	int i, j;
	FONT_GLYPH *g;
	RECT rect;
	u8 str[]={"\x83\x79\x83\x8b\x83\x5c\x83\x69\x82\x51\x81\x40\x8d\xdf\x81\x5e\x82\x65\x82\x68\x82\x6b\x82\x64\x82\x51\x81\x5e\x82\x6b\x82\x75\x81\x40\x82\x50"};
	u16 ucs_str[64];
	u16 sjis;
	u32 glyph[16][2];

	// sjis to ucs
	for(i=0, j=0; str[i]; j++)
	{
		i+=read_cp932(&str[i],&sjis);
		ucs_str[j]=char_to_ucs(sjis,CODEPAGE_932);
		DEBUGPRINT(("Converting %x as %x\n",sjis,ucs_str[j]));
	}
	ucs_str[j]='\0';

#if 1
	// expand glyphs
	for(i=0, j=wcslen(ucs_str); i<j; i++)
	{
		g=FontGetGlyph(&font,ucs_str[i]);
		// convert only what's in the font
		if(!g) FontGetGlyph(&font,OUTRNG);
		// expand and send to vram
		FontExpandGlyph(g,(u32*)glyph);
		setRECT(&rect,320+i*4,256+g->y,align(g->w,8)/4,g->h);
		LoadImage(&rect,(u_long*)glyph);
	}
#endif
}

void init_test()
{
	u8 *png_data, *fnt_data;
	GsPngImage img;
	RECT rect;
	int i/*, j, k*/;
	FONT_GLYPH *glyph;
	u32 g[16][2];
	PNG_UPLOAD upload;
	u16 clut[2][4]=
	{
		{0,RgbToClut(248,248,248),RgbToClut(160,160,160),RgbToClut(96,96,96)},
		{0,RgbToClut(16,16,16)|0x8000,RgbToClut(16,16,16)|0x8000,RgbToClut(16,16,16)|0x8000}
	};

//#if 0
	// FONT
	//png_data=(u8*)CdLoadFile("FNT_BASE.PNG",NULL);
	//load_png(png_data,0,&img,MASK_DEFAULT);
	//free3(png_data);
	// load font and clut
	//upload_png(&img,&upload);
	// make black font palette
	//for(i=0; i<3; i++) img.clut[i]=RgbToClut(16,16,16)|0x8000;	//;~img.clut[i]&0x7FFF;
	//setRECT(&rect,upload.cx,upload.cy+1,16,1);
	//LoadImage(&rect,(u_long*)img.clut);
//#endif

	// ICONS
	setPngUpload(&upload,FRAME_X,224,0,FRAME_Y*2);
	png_data=(u8*)CdLoadFile("BUTTONS.PNG",NULL);
	load_png(png_data,0,&img,MASK_DEFAULT);
	free3(png_data);
	//// PROGRESS BAR
	//setPngUpload(&upload,FRAME_X+10,224,0,FRAME_Y*2+1);
	//png_data=(u8*)CdLoadFile("PBAR.PNG",NULL);
	//load_png(png_data,0,&img,MASK_DEFAULT);
	//PBarInitGfx(&upload);
	//free3(png_data);
	// load font and clut
	setPngUpload(&upload,FRAME_X,224,16,FRAME_Y*2);
	upload_png(&img,&upload);
	free_png(&img);

	// BACKGROUND
	png_data=(u8*)CdLoadFile("BG_XMB.PNG",NULL);
	load_png(png_data,0,&img,MASK_DEFAULT);
	free3(png_data);
	// load picture
	setPngUpload(&upload,FRAME_X+64,0,64,480);
	upload_png(&img,&upload);
	free_png(&img);

	// FONT
	fnt_data=(u8*)CdLoadFile("FONT.BIN",NULL);
	FontOpen(fnt_data,&font);
	// upload clut
	setRECT(&rect,0,480,4,2);
	LoadImage(&rect,(u_long*)clut);
	// expand ascii glyphs
	for(i=0; i<224; i++)
	{
		glyph=FontGetGlyph(&font,i+0x20);
		if(!glyph) glyph=FontGetGlyph(&font,OUTRNG);
		FontExpandGlyph(glyph,(u32*)g);
		setRECT(&rect,FRAME_X+((i%16)*4),((i/16)*16)+glyph->y,align(glyph->w,8)/4,glyph->h);
		LoadImage(&rect,(u_long*)g);
	}

	mc_test();

//#if 0
	//// CKJ test
	//for(i=0, k=0x6000; i<11; i++)
	//{
	//	for(j=0; j<256; k++)
	//	{
	//		glyph=FontGetGlyph(&font,k);
	//		if(!glyph) continue;
	//		FontUpscaleGlyph(glyph,(u32*)g);
	//		setRECT(&rect,FRAME_X+i*64+((j%16)*4),256+((j/16)*16)+glyph->y,align(glyph->w,8)/4,glyph->h);
	//		LoadImage(&rect,(u_long*)g);

	//		j++;
	//	}
	//}
//#endif
}

void draw_hseparator(int x, int y, int w)
{
	int i;
	u8 rgb[3][3]={{32,32,32},{248,248,248},{32,32,32}};
	LINE_F2 *line=(LINE_F2*)MallocGfx(sizeof(LINE_F2)*3);
	GsOT_TAG *ot=GetOTag();

	for(i=0; i<3; i++, line++)
	{
		setLineF2(line);
		setXY2(line,x,y+i,x+w,y+i);
		setRGB0(line,rgb[i][0],rgb[i][1],rgb[i][2]);
		AddPrim(ot,line);
	}
}

u8* game_list[]=
{
	(u8*)"Crash Bandicoot [SCES-00344]",
	(u8*)"Destruction Derby [SCES-00008]",
	(u8*)"STR Video example [no code]",
	(u8*)"[SLPS-01304] Bass Fisherman (NTSC-J)",
	(u8*)"[SLPS-00408] Bass Fishing Game - Lake Masters (NTSC-J)",
	(u8*)"[SLPS-01608] Bass Fishing Game - Lake Masters [Reprint] (NTSC-J)",
	(u8*)"[SLPS-01765] Bass Landing (NTSC-J)",
	(u8*)"[SLPS-02647] Bass Landing 2 (NTSC-J)",
	(u8*)"[SLPS-01227] Bassing Beat (NTSC-J)",
	(u8*)"[SLPS-02859] Bassing Beat 2 (NTSC-J)",
	(u8*)"[SLPS-00542] Bastard!! Utsuronaru Kamigami no Utsuwa (NTSC-J)",
	(u8*)"[SLPS-00698] Batman Forever - The Arcade Game (NTSC-J)",
	(u8*)"[SLPS-00485] Battle Arena Nitoshinden (NTSC-J)",
	(u8*)"[SLPS-00025] Battle Arena Toshinden (NTSC-J)",
	(u8*)"[SLPS-00200] Battle Arena Toshinden 2 (NTSC-J)",
	(u8*)"[SLPM-80028] Battle Arena Toshinden 2 - Gyouten Kyougaku no Shishoku-ban (NTSC-J)",
	(u8*)"[SLPS-91006] Battle Arena Toshinden 2 Plus [PlayStation the Best] (NTSC-J)",
	(u8*)"[SLPS-00650] Battle Arena Toshinden 3 (NTSC-J)",
	(u8*)"[SLPS-01133] Battle Athletess - Daiundoukai Alternative (NTSC-J)",
	(u8*)"[SLPM-86525] Battle Athletess - Daiundoukai Alternative [MajorWave 1500 Series] (NTSC-J)",
	(u8*)"[SLPS-01548] Battle Athletess - Daiundoukai GTO (NTSC-J)",
	(u8*)"[SLPM-86526] Battle Athletess - Daiundoukai GTO [MajorWave 1500 Series] (NTSC-J)",
	(u8*)"[SLPM-80361] Battle Bonbee (NTSC-J)",
	(u8*)"[SLPS-00464] Battle Bugs (NTSC-J)",
	(u8*)"[SLPS-00991] Battle Bugs (NTSC-J)",
	(u8*)"[SLBM-00001] Battle for Saint (NTSC-J)",
	(u8*)"[SLPS-00968] Battle Formation (NTSC-J)",
	(u8*)"[SLPS-01779] Battle Konchuuden (NTSC-J)",
	(u8*)"[SLPS-01064] Battle Master (NTSC-J)",
	(u8*)"[SLPM-86519] Battle Master [MajorWave 1500 Series] (NTSC-J)",
	(u8*)"[SLPM-87165] Battle Qix [SuperLite 1500 Series] (NTSC-J)",
	(u8*)"[SLPS-00934] Battle Sports (NTSC-J)",
	(u8*)"[SLPM-86400] Battle Sugoroku - The Hunter [SuperLite 1500 Series] (NTSC-J)",
	(u8*)"[SLPS-01127] Battleround USA (NTSC-J)",
	(u8*)"[SCPS-10138] Bealphareth (NTSC-J)",
	(u8*)"[SLPS-02468] Beast Wars Metals (NTSC-J)",
	(u8*)"[SCPS-18013] Beat Planet Music (NTSC-J)"
};

u8* gui_str[]=
{
	(u8*)"Version 1.00 alpha",
	(u8*)"Running on PSIO Hardware v1.00",
	(u8*)"LOAD",
	(u8*)"OPTIONS",
	(u8*)"ABOUT",
	//(u8*)"RESTART",
};

u8* title_str=(u8*)"psiOS - PlayStation Input Output Operating System";

int gui_strxy[][2]=
{
	16,FRAME_Y-20,
	153,FRAME_Y-20,
	26,192,
	104,192,
	182,192,
	//262,192
};

GFX_MAPPING button_pict[]=
{
//	x	y	u	v	w	h	clut					tage
	0,	3,	0,	224,10,	10,	getClut(16,FRAME_Y*2),	getTPage(0,1,FRAME_X,0),	// CROSS
	0,	3,	10,	224,10,	10,	getClut(16,FRAME_Y*2),	getTPage(0,1,FRAME_X,0),	// TRIANGLE
	0,	3,	20,	224,10,	10,	getClut(16,FRAME_Y*2),	getTPage(0,1,FRAME_X,0),	// SQUARE
	0,	3,	30,	224,10,	10,	getClut(16,FRAME_Y*2),	getTPage(0,1,FRAME_X,0),	// CIRCLE
};

GFX_MAPPING bg_pict[]=
{
//	x		y	u	v	w	h	clut				tage
	{0,		0,	0,	0,	256,240,getClut(64,480),	getTPage(1,1,320+64,0)},
	{256,	0,	0,	0,	64,	240,getClut(64,480),	getTPage(1,1,320+64+128,0)}
};

//GFX_MAPPING test_char=
//{
////	x		y	u	v	w	h	clut			tage
//	0,		0,	0,	0,	256,256,getClut(0,482),	getTPage(0,0,320,256)
//};

void draw_about()
{
	DR_TPAGE *m=(DR_TPAGE*)MallocGfx(sizeof(DR_TPAGE));
	POLY_G4 *p=(POLY_G4*)MallocGfx(sizeof(POLY_G4));
	u8 *about_txt=(u8*)"\1\1psiOS - created by Gemini (PSY-Q, gcc 2.8.1)\n\1\0This is a test \1\3About \1\2Dialog\1\0.\nFill in whatever text necessary for testing this rubbish.\nAll text is globally center-aligned.";

	setDrawTPage(m,1,1,getTPage(0,2,0,0));
	setPolyG4(p);
	//setSemiTrans(p,1);
	setXYWH(p,0,FRAME_Y/2-32,FRAME_X,64);
	setRGB0(p,32,32,64);
	setRGB1(p,64,64,128);
	setRGB2(p,64,64,128);
	setRGB3(p,32,32,64);

	draw_string((FRAME_X-get_string_width(about_txt))/2,FRAME_Y/2-24,about_txt);
	draw_hseparator(0,FRAME_Y/2-32,FRAME_X);
	draw_hseparator(0,FRAME_Y/2+32,FRAME_X);
	AddPrim(GetOTag(),p);
	AddPrim(GetOTag(),m);
}

void draw_ribbon_bg()
{
	POLY_G4 *p=(POLY_G4*)MallocGfx(sizeof(POLY_G4));

	setPolyG4(p);
	setRGB0(p,0x15,0x6A,0xBB);
	setRGB1(p,0x9A,0xB9,0xE5);
	setRGB2(p,0x9A,0xB9,0xE5);
	setRGB3(p,0x0D,0x3F,0x7C);
	setXYWH(p,0,0,FRAME_X+1,FRAME_Y+1);

	AddPrim(GetOTag(),p);
}

enum LOOP_ACTIONS
{
	ACTION_IDLE,
	ACTION_RESET,
	ACTION_ABOUT
};

void draw_test()
{
	//u8 str[256];
	int i, pos, action;
	u32 pad_read;
	SCROLL_LIST slist;
	PROGRESS_BAR pbar;

	InitSList(&slist,1,10,DIMENSION_OF(game_list));
	//PBarInit(&pbar);
	//InitSList(&slist,10,10,203);

	//test_char.y=glyph->y;
	//test_char.h=glyph->h;

	while(1)
	{
		switch(action)
		{
		case ACTION_IDLE:	// do nothing, basically
		case ACTION_ABOUT:
			break;
		case ACTION_RESET:
			_boot();		// seems to be totally useless
		}

		BeginDraw();

		PBarDraw(&pbar,40,40,GetOTag());
		//SortMapSprt(4,4,&test_char,GetOTag());
		if(action==ACTION_ABOUT) draw_about();

		// manage scrolling list input
		pad_read=GetPadValueLocked();
		if(pad_read&UP_KEY || pad_read&LEFT_KEY) SListMoveBackward(&slist);
		if(pad_read&DOWN_KEY || pad_read&RIGHT_KEY) SListMoveForward(&slist);
		if(pad_read&R1_KEY) SListMovePForward(&slist);
		if(pad_read&L1_KEY) SListMovePBackward(&slist);
		if(pad_read&SQUARE_KEY) action=ACTION_RESET;
		if(pad_read&CIRCLE_KEY)
		{
			if(action==ACTION_ABOUT) action=ACTION_IDLE;
			else action=ACTION_ABOUT;
		}

		// draw top&bottom separators
		draw_hseparator(0,28,FRAME_X);
		draw_hseparator(0,FRAME_Y-28,FRAME_X);

		//sprintf((char*)str,"\1\1pos=%d, cur_pos=%d, scroll=%d, max=%d, smax=%d",slist.pos,slist.curs_pos,slist.scroll,slist.max,slist.smax);
		//draw_string(0,0,str);

		// draw interface strings
		for(i=0; i<DIMENSION_OF(gui_str); i++)
			draw_string(gui_strxy[i][0],gui_strxy[i][1],gui_str[i]);
		// draw command buttons
		for(i=2; i<DIMENSION_OF(gui_strxy); i++)
			SortMapSprt(gui_strxy[i][0]-10,gui_strxy[i][1],&button_pict[i-2],GetOTag());
		// draw title at the top of the screen
		draw_string((FRAME_X-get_string_width(title_str))/2,8,title_str);

		// draw game list
		draw_string(12,32+slist.curs_pos*16,">");
		for(i=0, pos=slist.scroll; i<10; i++, pos++)
		{
			if(i==slist.curs_pos) draw_string(20,32+i*16,game_list[pos]);
			else draw_string_color(24,32+i*16,game_list[pos],4);
		}

		draw_ribbon_bg();
		//SortMapSprt(0,0,&bg_pict[0],GetOTag());
		//SortMapSprt(0,0,&bg_pict[1],GetOTag());

		EndDraw(0,128,128);
	}
}
