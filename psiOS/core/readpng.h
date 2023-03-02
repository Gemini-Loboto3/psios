#ifndef __READ_PNG
#define __READ_PNG

#include "core.h"
#include "../libpng/png.h"
#include "../libpng/pngconf.h"

#define RgbToClut(r,g,b)	(u16)(((r)>>3)|(((g)>>3)<<5)|(((b)>>3)<<10))

typedef struct PngData
{
	u8 *data;
	u32 size;
	u32 seek;
} PngData;

typedef struct tagGsPngImage
{
	u8 *pixel;
	u16 *clut;
	u16 w, h;
	int depth;
} GsPngImage;

typedef struct tagPngUpload
{
	short px, py;
	short cx, cy;
} PNG_UPLOAD;

#define setPngUpload(u,_px,_py,_cx,_cy)	(u)->px=(_px), (u)->py=(_py), (u)->cx=(_cx), (u)->cy=(_cy)

#define MASK_DEFAULT			0	// enable color transparency math
#define MASK_BLACK_OPAQUE		1	// disable black transparency math
#define MASK_COLOR_OPAQUE		2	// disable color transparency math

#define PadWidth(width,depth)	(((depth*width+31)&~31)/8)

void* load_png(BYTE *buffer, int size, GsPngImage *dst, u32 mask);
void free_png(const GsPngImage *dst);
void upload_png(const GsPngImage *png, const PNG_UPLOAD *settings);

#endif
