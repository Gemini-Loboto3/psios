#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <inline_c.h>
#include <gtemac.h>
#include <MALLOC.H>
#include "gte.h"
#include "graph.h"
#include "general.h"
#include "field\map.h"

#define TEXTURED	0

typedef struct tagObjF3
{
	int n;			// primitive number
	VERT_F3 *vert;	// vertex
#if TEXTURED
	POLY_FT3 *prim;
#else
	POLY_F3 *prim;	// primitive
#endif
} OBJ_F3;

static OBJ_F3 cube;
#include "cube_tmd.c"
//#include "GIULIETA.C"

long clp, flg;	/* dummy */
long otz;

/* body of RotTransPers() */
#if 1
#define rotTransPers3(ot, p, v0, v1, v2)	{\
	gte_ldv3(v0,v1,v2);\
	gte_rtpt();\
	gte_stflg(&flg);\
	gte_nclip();\
	if(flg>0)\
	{\
		gte_stopz(&clp);\
		gte_avsz3();\
		if(clp>0)\
		{\
			gte_stotz(&otz);\
			if(otz>0 && otz<(1<<OT_LENGTH))\
			{\
				gte_stsxy3((long*)&p->x0,(long*)&p->x1,(long*)&p->x2);\
				addPrim(&ot[otz], p);\
			}\
		}\
	}\
}
#else
#if TEXTURED
void rotTransPers3(GsOT_TAG *ot, POLY_FT3 *p, SVECTOR *v0, SVECTOR *v1, SVECTOR *v2)
#else
void rotTransPers3(GsOT_TAG *ot, POLY_F3 *p, SVECTOR *v0, SVECTOR *v1, SVECTOR *v2)
#endif
{
	gte_ldv3(v0,v1,v2);
	gte_rtpt();
	gte_stflg(&flg);
	gte_nclip();
	if(flg>0)
	{
		gte_stopz(&clp);
		gte_avsz3();
		if(clp>0)
		{
			gte_stotz(&otz);
			if(otz>0 && otz<(1<<OT_LENGTH))
			{
				gte_stsxy3((long*)&p->x0,(long*)&p->x1,(long*)&p->x2);
				addPrim(&ot[otz], p);
			}
		}
	}
}
#endif

int freeTMD_F3(OBJ_F3 *obj)
{
	free3(obj->prim);
	free3(obj->vert);
}

int loadTMD_F3(u_long *tmd, OBJ_F3 *obj)
{
	VERT_F3 *vert;
#if TEXTURED
	POLY_FT3 *prim0, *prim1;
#else
	int col;
	POLY_F3 *prim0, *prim1;
#endif
	TMD_PRIM tmdprim;
	int /*col,*/ i, n_prim = 0;

	// open TMD
	if((n_prim=OpenTMD(tmd, 0))==0)
	{
		DEBUGPRINT(("Failed to open TMD %x\n",tmd));
		return 0;
	}
	// allocate all the necessary polygons and vectors
	vert=obj->vert=(VERT_F3*)malloc3(sizeof(VERT_F3)*n_prim);
#if TEXTURED
	prim0=obj->prim=(POLY_FT3*)malloc3(sizeof(POLY_FT3)*n_prim*2);
#else
	prim0=obj->prim=(POLY_F3*)malloc3(sizeof(POLY_F3)*n_prim*2);
#endif
	prim1=&obj->prim[n_prim];
	/* Set unchanged member of primitive here to deliminate main memory write access */	 
	for(i=0; i<n_prim && ReadTMD(&tmdprim)!=0; i++, vert++, prim0++)
	{
		/* initialize primitive */
#if TEXTURED
		setPolyFT3(prim0);
		setTPage(prim0,1,0,64,256);
		setClut(prim0,320,240);
		setUV3(prim0,1,128,128,128,1,255);
#else
		setPolyF3(prim0);
#endif
		/* copy normal and vertex */
		copyVector(&vert->n0,&tmdprim.n0);
		copyVector(&vert->v0,&tmdprim.x0);
		copyVector(&vert->v1,&tmdprim.x1);
		copyVector(&vert->v2,&tmdprim.x2);

#if TEXTURED
		setRGB0(prim0,128,128,128);
#else
		col=(tmdprim.n0.vx+tmdprim.n0.vy)*128/ONE/2+128;
		setRGB0(prim0,col,col,col);
#endif
	}
	DEBUGPRINT(("TMD %x (%d primitives) now open.\n",tmd,n_prim));
	/* duplicate primitive for primitive double buffering */
	zmemcpy(prim1,obj->prim,sizeof(prim1[0])*n_prim);
	return obj->n=i;
}

/********************************
 * LOCAL PROTOTYPES				*
 ********************************/
//static void view_init();
//static void light_init();
//static void coord_init();

/********************************
 * GTE MANAGEMENT				*
 ********************************/
GsRVIEW2 view;						// View Point Handler
GsF_LIGHT pslt[3];					// Flat Lighting Handler
GsCOORDINATE2 DWorld;				// Coordinate for GsDOBJ5
SVECTOR PWorld;						// work short vector for making Coordinate parmeter

extern MATRIX GsIDMATRIX;			// unit matrix

void Init3D()
{
	//GsInit3D();

	//coord_init();	// init coordinate
	//view_init();	// setting view point
	//light_init();	// setting flat light

	InitGeom();
	SetGeomOffset(0,0);
	SetGeomScreen(ONE);
}

///* Setting view point */
//static void view_init()
//{
//	/* Set projection,view */
//	GsSetProjection(1000);
//	/* Setting view point location */
//	view.vpx = 0; view.vpy = 0;
//	view.vpz = 2000;
//	/* Setting focus point location */
//	view.vrx = 0; view.vry = 0; view.vrz = 0;
//	/* Setting bank of SCREEN */
//	view.rz=0;
//	/* Setting parent of viewing coordinate */
//	view.super = WORLD;
//	/* Calculate World-Screen Matrix from viewing paramter */
//	GsSetRefView2(&view);
//	/* Set Near Clip*/
//	GsSetNearClip(100);
//}
//
///* init Flat light */
//static void light_init()
//{
//	/* Setting Light ID 0 */
//	/* Setting direction vector of Light 0 */
//	pslt[0].vx = 20; pslt[0].vy= -100; pslt[0].vz= -100;
//	/* Setting color of Light 0 */
//	pslt[0].r=0xd0; pslt[0].g=0xd0; pslt[0].b=0xd0;
//	/* Set Light0 from parameters */
//	GsSetFlatLight(0,&pslt[0]);
//	/* Setting Light ID 1 */
//	pslt[1].vx = 20; pslt[1].vy= -50; pslt[1].vz= 100;
//	pslt[1].r=0x80; pslt[1].g=0x80; pslt[1].b=0x80;
//	GsSetFlatLight(1,&pslt[1]);
//	/* Setting Light ID 2 */
//	pslt[2].vx = -20; pslt[2].vy= 20; pslt[2].vz= -100;
//	pslt[2].r=0x60; pslt[2].g=0x60; pslt[2].b=0x60;
//	GsSetFlatLight(2,&pslt[2]);
//	/* Setting Ambient */
//	GsSetAmbient(0,0,0);
//	/* Setting default light mode */
//	GsSetLightMode(0);
//}
//
///* Setting coordinate */
//static void coord_init()
//{
//	/* Setting hierarchy of Coordinate */
//	GsInitCoordinate2(WORLD,&DWorld);
//	/* Init work vector */
//	PWorld.vx=-320;
//	PWorld.vy=-240;
//	PWorld.vz=0;
//	/* the org point of DWold is set to Z = -40000 */
//	DWorld.coord.t[2] = -4000;
//}

///////////////////////////////////////////////////////////////
static SVECTOR	objang={0,0,0};
static MATRIX	objmat;
//static MATRIX	lgtmat;
//static MATRIX	rotmat;

SVECTOR lgt[3];		/* light source's position */
SVECTOR ypl[3];		/* plane's position */
CVECTOR c={128,128,128,1};

/*initial light position*/
//static SVECTOR	l_init[3]= {{150, -210, 0},{160, -210, 0},{160, -200, 10}};

void Draw3DTest(GsOT_TAG *ot)
{
	int i;
	VECTOR vec={320*4,208*4,ONE*2};
	extern int outputBufferIndex;
	extern CAMERA camera;
	VERT_F3 *vp;
#if TEXTURED
	POLY_FT3 *pp;
#else
	POLY_F3 *pp;
#endif

	objang.vx=objang.vy=camera.xroom*8;
	PushMatrix();
	//RotMatrix(&lgtang,&lgtmat);
	//for(i=0; i<3; i++) ApplyMatrixSV(&lgtmat,&l_init[i],&lgt[i]);
	RotMatrix(&objang,&objmat);
	TransMatrix(&objmat,&vec);
	SetRotMatrix(&objmat);
	SetTransMatrix(&objmat);
	//for(i=0; i<3; i++) ApplyMatrixSV(&objmat,&y_init[i],&ypl[i]);

	vp=cube.vert;
	pp=&cube.prim[outputBufferIndex*cube.n];
	for(i=0; i<cube.n; i++, vp++, pp++)
	{
#if 0
		RotAverageNclip3(
		//RotTransPers3(
			&vp->v0,&vp->v1,&vp->v2,
			(long*)&pp->x0,
			(long*)&pp->x1,
			(long*)&pp->x2,
			&clp,
			&otz,
			&flg);
		addPrim(&ot[otz],pp);
#else
		rotTransPers3(ot,pp,&vp->v0,&vp->v1,&vp->v2);
#endif
	}
	//for(i=0;i<6;i++)
	//{
	//	for(j=0;j<2;j++)
	//	{
	//		RotAverageNclip3(&cube[i][j][0],&cube[i][j][1],&cube[i][j][2],
	//		(long*)&wall[outputBufferIndex][i][j].x0,
	//		(long*)&wall[outputBufferIndex][i][j].x1,
	//		(long*)&wall[outputBufferIndex][i][j].x2,
	//		&clp, &otz, &flg);
	//		NormalColorCol(&n[i][j],&c,(CVECTOR*)&wall[outputBufferIndex][i][j].r0);
	//		addPrim(&ot[0],&wall[outputBufferIndex][i][j]);
	//	}
	//}
	PopMatrix();
}

void gte_init_prim()
{
	loadTMD_F3((u_long*)cube_tmd,&cube);
}

void gte_free_prim()
{
	freeTMD_F3(&cube);
}
