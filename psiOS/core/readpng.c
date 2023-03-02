#include "readpng.h"
#include "MALLOC.H"

static void png_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
	png_size_t i;
	PngData *pngData=(PngData*)png_get_io_ptr(png_ptr);

	//DEBUGPRINT(("Reading %d bytes at %x\n",length,pngData->seek));

	if(pngData)
	{
		for(i=0; i<length; i++)
		{
			//if(pngData->seek>=pngData->size) break;
			data[i]=pngData->data[pngData->seek++];
			//DEBUGPRINT(("%x ",data[i]));
		}
	}
	//DEBUGPRINT(("\n"));
}

//static void png_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
//{
//	png_size_t i;
//	PngData *pngData=(PngData*)png_get_io_ptr(png_ptr);
//
//	if(pngData)
//	{
//		for(i=0; i<length; i++)
//		{
//			if(pngData->seek>=pngData->size) break;
//			pngData->data[pngData->seek++]=data[i];
//		}
//	}
//}
//
//static void png_flush(png_structp png_ptr)
//{
//}

png_infop read_png_info(png_structp png_ptr)
{
	png_infop info_ptr=png_create_info_struct(png_ptr);
	if(!info_ptr)
	{
		DEBUGPRINT(("Could not create info_ptr\n",info_ptr));
		return NULL;
	}
	if(setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct(&png_ptr,&info_ptr,NULL);
		return 0;
	}
	png_set_sig_bytes(png_ptr,0);
	png_read_info(png_ptr,info_ptr);

	return info_ptr;
}

//static u16 palette[256];
static u8 *line, *pline;
static u8 *pimage;
static int width, height, depth, color_type, palsize;

void* load_png(BYTE *buffer, int size, GsPngImage *dst, u32 mask)
{
	int i, j, _w;
	png_structp png_ptr;
	png_infop info_ptr;
	PngData data;
	png_colorp pal __attribute__((aligned(1)));

	// set png buffer
	data.data=buffer;
	data.seek=0;
	data.size=size;
	// empty dest
	zmemset(dst,0,sizeof(GsPngImage));

	// check the PNG signature
	if(!png_check_sig(buffer,8)) return 0;

	png_ptr=png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png_ptr)
	{
		DEBUGPRINT(("Cannot create png_ptr.\n"));
		return 0;
	}

	png_set_read_fn(png_ptr,(void*)&data,png_read_data);
	DEBUGPRINT(("Reading png info from %x\n",png_ptr));
	if(!(info_ptr=read_png_info(png_ptr)))
	{
		DEBUGPRINT(("Cannot read png information.\n"));
		return 0;
	}

	// define all crap
	if(!png_get_IHDR(png_ptr,info_ptr,(png_uint_32*)&width,(png_uint_32*)&height,&depth,&color_type,NULL,NULL,NULL))
	{
		DEBUGPRINT(("Could not get IHDR.\n"));
		return 0;
	}

	png_set_strip_16(png_ptr);
	//png_set_swap(png_ptr);
	//png_set_packing(png_ptr);

	switch(color_type)
	{
	case PNG_COLOR_TYPE_GRAY:
		DEBUGPRINT(("Type is PNG_COLOR_TYPE_GRAY, but it's not for real...\n"));
		break;
	case PNG_COLOR_MASK_ALPHA:
		if(depth<=8) png_set_expand(png_ptr);
		png_set_gray_to_rgb(png_ptr);
		depth=16;
		break;
	case PNG_COLOR_MASK_COLOR:
	case PNG_COLOR_TYPE_RGB_ALPHA:
		png_set_expand(png_ptr);
		depth=24;
		break;
	case PNG_COLOR_TYPE_PALETTE:
		png_get_PLTE(png_ptr,info_ptr,&pal,&palsize);
		dst->clut=(u16*)malloc3(palsize*sizeof(u16));
		//png_get_tRNS(png_ptr,info_ptr,&alpha,&alphasize,&alphaval);
		DEBUGPRINT(("Converting palette %x - %d colors\n",pal,palsize));
		for(i=0; i<palsize; i++)
		{
			dst->clut/*palette*/[i]=RgbToClut(pal[i].red,pal[i].green,pal[i].blue);
			if(mask&MASK_BLACK_OPAQUE && dst->clut/*palette*/[i]==0) dst->clut/*palette*/[i]=0x8000;
			if(mask&MASK_COLOR_OPAQUE) dst->clut/*palette*/[i]|=0x8000;
		}
		break;
	default:
		DEBUGPRINT(("Unrecognized color type %d!",color_type));
	}
	_w=PadWidth(width,depth);	// bytes per row
	DEBUGPRINT(("w=%d (%d), h=%d, depth=%d, type=%d\n",width,PadWidth(width,depth),height,depth,color_type));

	if(png_get_valid(png_ptr,info_ptr,PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png_ptr);
	//png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);

	// allocate image
	//if(image) free3(image);
	if(depth!=1)
		dst->pixel/*image*/=(u8*)malloc3(_w*height);
	else
		dst->pixel/*image*/=(u8*)malloc3(_w*4*height);

	pimage=dst->pixel;//image;
	// copy decompressed image
	switch(depth)
	{
	case 1:		// upscale to 4 bit
		dst->w=width/4;
		// temporary scanline buffer
		line=(u8*)malloc3(_w);
		for(i=0; i<height; i++)
		{
			png_read_row(png_ptr,line,NULL);
			pline=line;
			for(j=0; j<_w; j++, pimage+=4, pline++)
			{
				*((u32*)pimage)=(*pline&1)<<28
					| ((*pline>>1)&1)<<24
					| ((*pline>>2)&1)<<20
					| ((*pline>>3)&1)<<16
					| ((*pline>>4)&1)<<12
					| ((*pline>>5)&1)<<8
					| ((*pline>>6)&1)<<4
					| ((*pline>>7)&1)<<0;
			}
		}
		free3(line);
		break;
	case 4:		// nibbles have to be swapped
		dst->w=width/4;
		for(i=0; i<height; i++)
		{
			png_read_row(png_ptr,pimage,NULL);
			// swap nibbles
			for(j=0; j<_w; j++, pimage++)
				*pimage=(*pimage>>4)|(*pimage<<4);
		}
		break;
	case 8:		// straight copy
		dst->w=width/2;
		for(i=0; i<height; i++, pimage+=_w)
			png_read_row(png_ptr,pimage,NULL);
		break;
	case 16:	// convert from RGB888 to RGBA555
		dst->w=width;
		// temporary scanline buffer
		line=(u8*)malloc3(width);
		for(i=0; i<height; i++)
		{
			png_read_row(png_ptr,line,NULL);
			pline=line;
			for(j=0; j<width; j++)
			{
				*((u16*)pimage)=RgbToClut(pline[0],pline[1],pline[2]);
				if(mask&MASK_BLACK_OPAQUE && *((u16*)pimage)==0) *((u16*)pimage)=0x8000;
				if(mask&MASK_COLOR_OPAQUE) *((u16*)pimage)|=0x8000;
				pimage+=2;
				pline+=3;
			}
		}
		free3(line);
		break;
	case 24:	// convert from RGB888 to RGBA555
		dst->w=width;
		dst->depth=16;
		// temporary scanline buffer
		line=(u8*)malloc3(width);
		for(i=0; i<height; i++)
		{
			png_read_row(png_ptr,line,NULL);
			pline=line;
			for(j=0; j<width; j++)
			{
				*((u16*)pimage)=RgbToClut(pline[0],pline[1],pline[2]);
				if(mask&MASK_BLACK_OPAQUE && *((u16*)pimage)==0) *((u16*)pimage)=0x8000;
				if(mask&MASK_COLOR_OPAQUE) *((u16*)pimage)|=0x8000;
				pimage+=2;
				pline+=4;
			}
		}
		free3(line);
		break;
	default:
		DEBUGPRINT(("Unsupported color depth (%d)\n",depth));
	}

	// free all unnecessary buffers
	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);

	//dst->pixel=image;
	//dst->clut=palette;
	dst->depth=depth;
	dst->h=height;

	// return a correct pointer
	return (void*)dst->pixel;	//image;
}

void free_png(const GsPngImage *dst)
{
	if(dst->pixel) free3(dst->pixel);
	if(dst->clut) free3(dst->clut);
}

void upload_png(const GsPngImage *png, const PNG_UPLOAD *settings)
{
	RECT rect;

	// has picture
	if(png->pixel)
	{
		setRECT(&rect,settings->px,settings->py,png->w,png->h);
		LoadImage(&rect,(u_long*)png->pixel);
	}
	// has clut
	if(png->clut)
	{
		setRECT(&rect,settings->cx,settings->cy,1<<png->depth,1);
		LoadImage(&rect,(u_long*)png->clut);
	}
}
