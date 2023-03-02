#ifndef __GTE_H
#define __GTE_H

#include "common.h"

#define OT_Z	1024

typedef struct tagVertF3
{
	SVECTOR	n0;			// normal
	SVECTOR	v0, v1, v2;	// vertex
} VERT_F3;

typedef struct tagPolyHeader
{
	u32 count;
} PolyHeader, *P_HEADER;

typedef struct tagPolyCount
{
	u16 count;
	u16 tpage;
} PolyCount, *P_COUNT;
	// 2 bytes

typedef struct tagPolyIndeces
{
	u16 a;
	u16 b;
	u16 c;
	u16 d;
} PolyIndices, *P_INDEX;
	// 8 bytes

typedef struct tagColorTriangle
{
	PolyIndices vertex;
	CVECTOR colors[3];
} ColorTriangle;
	// 8+4*3=20 bytes

typedef struct tagColorQuadric
{
	PolyIndices vertex;
	CVECTOR colors[4];
} ColorQuadric;
	// 8+4*4=24 bytes

typedef struct tagTexTriangle
{
	PolyIndices vertex;		// 0-7
	u8	u0, v0;				// 8-9
	u16	clut;				// 10-11
	u8	u1, v1;				// 12-13
	u8	u2, v2;				// 14-15
} TexTriangle;
	// 8+8=16 bytes

typedef struct tagTexQuadric
{
	PolyIndices vertex;
	u8 u0, v0;
	u16 clut;
	u8 u1, v1;
	u8 u2, v2;
	u8 u3, v3;
	u16 pad;
} TexQuadric;
	// 8+12=20 bytes

void Init3D();
void Draw3DTest(GsOT_TAG *ot);
void gte_init_prim();
void gte_free_prim();

#endif
