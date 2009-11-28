/***************************************************************************
Namco System 21 Video Hardware

- sprite hardware is identical to Namco System NB1
- there are no tilemaps
- 3d graphics are managed by DSP processors
*/

#include "driver.h"
#include "namcos2.h"
#include "namcoic.h"
#include "namcos21.h"

#define FRAMEBUFFER_SIZE_IN_BYTES (sizeof(UINT16)*NAMCOS21_POLY_FRAME_WIDTH*NAMCOS21_POLY_FRAME_HEIGHT)

/* work (hidden) framebuffer */
static UINT16 *mpPolyFrameBufferPens;
static UINT16 *mpPolyFrameBufferZ;

/* visible framebuffer */
static UINT16 *mpPolyFrameBufferPens2;
static UINT16 *mpPolyFrameBufferZ2;

static UINT16 winrun_color;
static UINT16 winrun_gpu_register[0x10/2];

READ16_HANDLER(winrun_gpu_color_r)
{
	return winrun_color;
}

WRITE16_HANDLER(winrun_gpu_color_w)
{
	COMBINE_DATA( &winrun_color );
}

READ16_HANDLER(winrun_gpu_register_r)
{
	return winrun_gpu_register[offset];
}

WRITE16_HANDLER(winrun_gpu_register_w)
{
	COMBINE_DATA( &winrun_gpu_register[offset] );
}

WRITE16_HANDLER( winrun_gpu_videoram_w)
{
	int color = data>>8;
	int mask  = data&0xff;
	int i;
	for( i=0; i<8; i++ )
	{
		if( mask&(0x01<<i) )
		{
			space->machine->generic.videoram.u8[(offset+i)&0x7ffff] = color;
		}
	}
} /* winrun_gpu_videoram_w */

READ16_HANDLER( winrun_gpu_videoram_r )
{
	return space->machine->generic.videoram.u8[offset]<<8;
} /* winrun_gpu_videoram_r */

static void
AllocatePolyFrameBuffer( running_machine *machine )
{
	mpPolyFrameBufferZ     = auto_alloc_array(machine, UINT16, FRAMEBUFFER_SIZE_IN_BYTES/2 );
	mpPolyFrameBufferPens  = auto_alloc_array(machine, UINT16, FRAMEBUFFER_SIZE_IN_BYTES/2 );

	mpPolyFrameBufferZ2    = auto_alloc_array(machine, UINT16, FRAMEBUFFER_SIZE_IN_BYTES/2 );
	mpPolyFrameBufferPens2 = auto_alloc_array(machine, UINT16, FRAMEBUFFER_SIZE_IN_BYTES/2 );

	namcos21_ClearPolyFrameBuffer();
	namcos21_ClearPolyFrameBuffer();
} /* AllocatePolyFrameBuffer */

void
namcos21_ClearPolyFrameBuffer( void )
{
	int i;
	UINT16 *temp2;

	/* swap work and visible framebuffers */
	temp2 = mpPolyFrameBufferZ;
	mpPolyFrameBufferZ = mpPolyFrameBufferZ2;
	mpPolyFrameBufferZ2 = temp2;

	temp2 = mpPolyFrameBufferPens;
	mpPolyFrameBufferPens = mpPolyFrameBufferPens2;
	mpPolyFrameBufferPens2 = temp2;

	/* wipe work zbuffer */
	for( i=0; i<NAMCOS21_POLY_FRAME_WIDTH*NAMCOS21_POLY_FRAME_HEIGHT; i++ )
	{
		mpPolyFrameBufferZ[i] = 0x7fff;
	}
} /* namcos21_ClearPolyFrameBuffer */

static void
CopyVisiblePolyFrameBuffer( bitmap_t *bitmap, const rectangle *clip, int zlo, int zhi )
{ /* blit the visible framebuffer */
	int sy;
	for( sy=clip->min_y; sy<=clip->max_y; sy++ )
	{
		UINT16 *dest = BITMAP_ADDR16(bitmap, sy, 0);
		const UINT16 *pPen = mpPolyFrameBufferPens2+NAMCOS21_POLY_FRAME_WIDTH*sy;
		const UINT16 *pZ = mpPolyFrameBufferZ2+NAMCOS21_POLY_FRAME_WIDTH*sy;
		int sx;
		for( sx=clip->min_x; sx<=clip->max_x; sx++ )
		{
			int z = pZ[sx];
			//if( pZ[sx]!=0x7fff )
			if( z>=zlo && z<=zhi )
			{
				dest[sx] = pPen[sx];
			}
		}
	}
} /* CopyVisiblePolyFrameBuffer */

static int objcode2tile( int code )
{ /* callback for sprite drawing code in namcoic.c */
	return code;
} /* objcode2tile */

VIDEO_START( namcos21 )
{
	if( namcos2_gametype == NAMCOS21_WINRUN91 )
	{
		machine->generic.videoram.u8 = auto_alloc_array(machine, UINT8, 0x80000);
	}
	AllocatePolyFrameBuffer(machine);
	namco_obj_init(machine,
		0,		/* gfx bank */
		0xf,	/* reverse palette mapping */
		objcode2tile );
} /* VIDEO_START( namcos21 ) */

static void
update_palette( running_machine *machine )
{
	int i;
	INT16 data1,data2;
	int r,g,b;

	/*
    Palette:
        0x0000..0x1fff  sprite palettes (0x10 sets of 0x100 colors)

        0x2000..0x3fff  polygon palette bank0 (0x10 sets of 0x200 colors)
            (in starblade, some palette animation effects are performed here)

        0x4000..0x5fff  polygon palette bank1 (0x10 sets of 0x200 colors)

        0x6000..0x7fff  polygon palette bank2 (0x10 sets of 0x200 colors)

        The polygon-dedicated color sets within a bank typically increase in
        intensity from very dark to full intensity.

        Probably the selected palette is determined by most significant bits of z-code.
        This is not yet hooked up.
    */
	for( i=0; i<NAMCOS21_NUM_COLORS; i++ )
	{
		data1 = machine->generic.paletteram.u16[0x00000/2+i];
		data2 = machine->generic.paletteram.u16[0x10000/2+i];

		r = data1>>8;
		g = data1&0xff;
		b = data2&0xff;

		palette_set_color( machine,i, MAKE_RGB(r,g,b) );
	}
} /* update_palette */


VIDEO_UPDATE( namcos21 )
{
	int pivot = 3;
	int pri;
	update_palette(screen->machine);
	bitmap_fill( bitmap, cliprect , 0xff);

	if( namcos2_gametype != NAMCOS21_WINRUN91 )
	{ /* draw low priority 2d sprites */
		namco_obj_draw(screen->machine, bitmap, cliprect, 2 );
		namco_obj_draw(screen->machine, bitmap, cliprect, 14 );	//driver's eyes
	}

	CopyVisiblePolyFrameBuffer( bitmap, cliprect,0x7fc0,0x7ffe );

	if( namcos2_gametype != NAMCOS21_WINRUN91 )
	{ /* draw low priority 2d sprites */
		namco_obj_draw(screen->machine, bitmap, cliprect, 0 );
		namco_obj_draw(screen->machine, bitmap, cliprect, 1 );
	}

	CopyVisiblePolyFrameBuffer( bitmap, cliprect,0,0x7fbf );


	if( namcos2_gametype != NAMCOS21_WINRUN91 )
	{ /* draw high priority 2d sprites */
		for( pri=pivot; pri<8; pri++ )
		{
			namco_obj_draw(screen->machine, bitmap, cliprect, pri );
		}
			namco_obj_draw(screen->machine, bitmap, cliprect, 15 );	//driver's eyes
	}
	else
	{ /* winrun bitmap layer */
		int yscroll = -cliprect->min_y+(INT16)winrun_gpu_register[0x2/2];
		int base = 0x1000+0x100*(winrun_color&0xf);
		int sx,sy;
		for( sy=cliprect->min_y; sy<=cliprect->max_y; sy++ )
		{
			const UINT8 *pSource = &screen->machine->generic.videoram.u8[((yscroll+sy)&0x3ff)*0x200];
			UINT16 *pDest = BITMAP_ADDR16(bitmap, sy, 0);
			for( sx=cliprect->min_x; sx<=cliprect->max_x; sx++ )
			{
				int pen = pSource[sx];
				switch( pen )
				{
				case 0xff:
					break;
				case 0x00:
					pDest[sx] = (pDest[sx]&0x1fff)+0x4000;
					break;
				case 0x01:
					pDest[sx] = (pDest[sx]&0x1fff)+0x6000;
					break;
				default:
					pDest[sx] = base|pen;
					break;
				}
			}
		}
	} /* winrun bitmap layer */
	return 0;
} /* VIDEO_UPDATE( namcos21 ) */

/*********************************************************************************************/

typedef struct
{
	double x,y;
	double z;
} vertex;

typedef struct
{
	double x;
	double z;
} edge;

#define SWAP(T,A,B) { const T *temp = A; A = B; B = temp; }

static void
renderscanline_flat( const edge *e1, const edge *e2, int sy, unsigned color, int depthcueenable )
{
	if( e1->x > e2->x )
	{
		SWAP(edge,e1,e2);
	}

	{
		UINT16 *pDest = mpPolyFrameBufferPens + sy*NAMCOS21_POLY_FRAME_WIDTH;
		UINT16 *pZBuf = mpPolyFrameBufferZ    + sy*NAMCOS21_POLY_FRAME_WIDTH;
		int x0 = (int)e1->x;
		int x1 = (int)e2->x;
		int w = x1-x0;
		if( w )
		{
			double z = e1->z;
			double dz = (e2->z - e1->z)/w;
			int x, crop;
			crop = - x0;
			if( crop>0 )
			{
				z += crop*dz;
				x0 = 0;
			}
			if( x1>NAMCOS21_POLY_FRAME_WIDTH-1 )
			{
				x1 = NAMCOS21_POLY_FRAME_WIDTH-1;
			}

			for( x=x0; x<x1; x++ )
			{
				UINT16 zz = (UINT16)z;
				if( zz<pZBuf[x] )
				{
					int pen = color;
					if( depthcueenable && zz>0 )
					{
						int depth = 0;
						if( namcos2_gametype == NAMCOS21_WINRUN91 )
						{
							depth = (zz>>10)*0x100;
							pen += depth;
						}
						else if( namcos2_gametype == NAMCOS21_DRIVERS_EYES )
						{
							depth = (zz>>10)*0x100;
							pen -= depth;
						}
						else
						{
							depth = (zz>>11)*0x200;
							pen -= depth;
						}
					}
					pDest[x] = pen;
					pZBuf[x] = zz;
				}
				z += dz;
			}
		}
	}
} /* renderscanline_flat */

static void
rendertri(
		const vertex *v0,
		const vertex *v1,
		const vertex *v2,
		unsigned color,
		int depthcueenable )
{
	int dy,ystart,yend,crop;

	/* first, sort so that v0->y <= v1->y <= v2->y */
	for(;;)
	{
		if( v0->y > v1->y )
		{
			SWAP(vertex,v0,v1);
		}
		else if( v1->y > v2->y )
		{
			SWAP(vertex,v1,v2);
		}
		else
		{
			break;
		}
	}

	ystart = v0->y;
	yend   = v2->y;
	dy = yend-ystart;
	if( dy )
	{
		int y;
		edge e1; /* short edge (top and bottom) */
		edge e2; /* long (common) edge */

		double dx2dy = (v2->x - v0->x)/dy;
		double dz2dy = (v2->z - v0->z)/dy;

		double dx1dy;
		double dz1dy;

		e2.x = v0->x;
		e2.z = v0->z;
		crop = -ystart;
		if( crop>0 )
		{
			e2.x += dx2dy*crop;
			e2.z += dz2dy*crop;
		}

		ystart = v0->y;
		yend = v1->y;
		dy = yend-ystart;
		if( dy )
		{
			e1.x = v0->x;
			e1.z = v0->z;

			dx1dy = (v1->x - v0->x)/dy;
			dz1dy = (v1->z - v0->z)/dy;

			crop = -ystart;
			if( crop>0 )
			{
				e1.x += dx1dy*crop;
				e1.z += dz1dy*crop;
				ystart = 0;
			}
			if( yend>NAMCOS21_POLY_FRAME_HEIGHT-1 ) yend = NAMCOS21_POLY_FRAME_HEIGHT-1;

			for( y=ystart; y<yend; y++ )
			{
				renderscanline_flat( &e1, &e2, y, color, depthcueenable );

				e2.x += dx2dy;
				e2.z += dz2dy;

				e1.x += dx1dy;
				e1.z += dz1dy;
			}
		}

		ystart = v1->y;
		yend = v2->y;
		dy = yend-ystart;
		if( dy )
		{
			e1.x = v1->x;
			e1.z = v1->z;

			dx1dy = (v2->x - v1->x)/dy;
			dz1dy = (v2->z - v1->z)/dy;

			crop = -ystart;
			if( crop>0 )
			{
				e1.x += dx1dy*crop;
				e1.z += dz1dy*crop;
				ystart = 0;
			}
			if( yend>NAMCOS21_POLY_FRAME_HEIGHT-1 )
			{
				yend = NAMCOS21_POLY_FRAME_HEIGHT-1;
			}
			for( y=ystart; y<yend; y++ )
			{
				renderscanline_flat( &e1, &e2, y, color, depthcueenable );

				e2.x += dx2dy;
				e2.z += dz2dy;

				e1.x += dx1dy;
				e1.z += dz1dy;
			}
		}
	}
} /* rendertri */

void
namcos21_DrawQuad( int sx[4], int sy[4], int zcode[4], int color )
{
	vertex a,b,c,d;
	int depthcueenable = 1;
	/*
        0x0000..0x1fff  sprite palettes (0x20 sets of 0x100 colors)
        0x2000..0x3fff  polygon palette bank0 (0x10 sets of 0x200 colors or 0x20 sets of 0x100 colors)
        0x4000..0x5fff  polygon palette bank1 (0x10 sets of 0x200 colors or 0x20 sets of 0x100 colors)
        0x6000..0x7fff  polygon palette bank2 (0x10 sets of 0x200 colors or 0x20 sets of 0x100 colors)
    */
	if( namcos2_gametype == NAMCOS21_WINRUN91 )
	{
		color = 0x4000|(color&0xff);
	}
	else if ( namcos2_gametype == NAMCOS21_DRIVERS_EYES )
	{
		color = 0x3f00|(color&0xff);
	}
	else
	{ /* map color code to hardware pen */
		int code = color>>8;
		if( code&0x80 )
		{
			color = color&0xff;
//          color = 0x3e00|color;
			color = 0x2100|color;
			depthcueenable = 0;
		}
		else
		{
			color&=0xff;
			color = 0x3e00|color;
			if( (code&0x02)==0 )
			{
				color|=0x100;
			}
		}
	}
	a.x = sx[0];
	a.y = sy[0];
	a.z = zcode[0];

	b.x = sx[1];
	b.y = sy[1];
	b.z = zcode[1];

	c.x = sx[2];
	c.y = sy[2];
	c.z = zcode[2];

	d.x = sx[3];
	d.y = sy[3];
	d.z = zcode[3];

	rendertri( &a, &b, &c, color, depthcueenable );
	rendertri( &c, &d, &a, color, depthcueenable );
} /* namcos21_DrawQuad */
