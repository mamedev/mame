// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/***************************************************************************
Namco System 21 Video Hardware

- sprite hardware is identical to Namco System NB1
- there are no tilemaps
- 3d graphics are managed by DSP processors
*/

#include "emu.h"
#include "includes/namcoic.h"
#include "includes/namcos21.h"

#define FRAMEBUFFER_SIZE_IN_BYTES (sizeof(UINT16)*NAMCOS21_POLY_FRAME_WIDTH*NAMCOS21_POLY_FRAME_HEIGHT)

READ16_MEMBER(namcos21_state::winrun_gpu_color_r)
{
	return m_winrun_color;
}

WRITE16_MEMBER(namcos21_state::winrun_gpu_color_w)
{
	COMBINE_DATA( &m_winrun_color );
}

READ16_MEMBER(namcos21_state::winrun_gpu_register_r)
{
	return m_winrun_gpu_register[offset];
}

WRITE16_MEMBER(namcos21_state::winrun_gpu_register_w)
{
	COMBINE_DATA( &m_winrun_gpu_register[offset] );
}

WRITE16_MEMBER(namcos21_state::winrun_gpu_videoram_w)
{
	UINT8 *videoram = m_videoram.get();
	int color = data>>8;
	int mask  = data&0xff;
	int i;
	for( i=0; i<8; i++ )
	{
		if( mask&(0x01<<i) )
		{
			videoram[(offset+i)&0x7ffff] = color;
		}
	}
}

READ16_MEMBER(namcos21_state::winrun_gpu_videoram_r)
{
	UINT8 *videoram = m_videoram.get();
	return videoram[offset]<<8;
}

void namcos21_state::allocate_poly_framebuffer()
{
	m_mpPolyFrameBufferZ     = std::make_unique<UINT16[]>(FRAMEBUFFER_SIZE_IN_BYTES/2 );
	m_mpPolyFrameBufferPens  = std::make_unique<UINT16[]>(FRAMEBUFFER_SIZE_IN_BYTES/2 );

	m_mpPolyFrameBufferZ2    = std::make_unique<UINT16[]>(FRAMEBUFFER_SIZE_IN_BYTES/2 );
	m_mpPolyFrameBufferPens2 = std::make_unique<UINT16[]>(FRAMEBUFFER_SIZE_IN_BYTES/2 );

	clear_poly_framebuffer();
	clear_poly_framebuffer();
}

void namcos21_state::clear_poly_framebuffer()
{
	/* swap work and visible framebuffers */
	m_mpPolyFrameBufferZ.swap(m_mpPolyFrameBufferZ2);

	m_mpPolyFrameBufferPens.swap(m_mpPolyFrameBufferPens2);

	/* wipe work zbuffer */
	for( int i = 0; i < NAMCOS21_POLY_FRAME_WIDTH*NAMCOS21_POLY_FRAME_HEIGHT; i++ )
	{
		m_mpPolyFrameBufferZ[i] = 0x7fff;
	}
}

void namcos21_state::copy_visible_poly_framebuffer(bitmap_ind16 &bitmap, const rectangle &clip, int zlo, int zhi)
{
	/* blit the visible framebuffer */
	int sy;
	for( sy=clip.min_y; sy<=clip.max_y; sy++ )
	{
		UINT16 *dest = &bitmap.pix16(sy);
		const UINT16 *pPen = m_mpPolyFrameBufferPens2.get()+NAMCOS21_POLY_FRAME_WIDTH*sy;
		const UINT16 *pZ = m_mpPolyFrameBufferZ2.get()+NAMCOS21_POLY_FRAME_WIDTH*sy;
		int sx;
		for( sx=clip.min_x; sx<=clip.max_x; sx++ )
		{
			int z = pZ[sx];
			//if( pZ[sx]!=0x7fff )
			if( z>=zlo && z<=zhi )
			{
				dest[sx] = pPen[sx];
			}
		}
	}
}

VIDEO_START_MEMBER(namcos21_state,namcos21)
{
	if( m_gametype == NAMCOS21_WINRUN91 )
	{
		m_videoram = std::make_unique<UINT8[]>(0x80000);
	}
	allocate_poly_framebuffer();
	c355_obj_init(
		0,      /* gfx bank */
		0xf,    /* reverse palette mapping */
		namcos2_shared_state::c355_obj_code2tile_delegate() );
}


/*  Palette:
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


UINT32 namcos21_state::screen_update_namcos21(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 *videoram = m_videoram.get();
	int pivot = 3;
	int pri;
	bitmap.fill(0xff, cliprect );

	if( m_gametype != NAMCOS21_WINRUN91 )
	{ /* draw low priority 2d sprites */
		c355_obj_draw(screen, bitmap, cliprect, 2 );
		c355_obj_draw(screen, bitmap, cliprect, 14 );   //driver's eyes
	}

	copy_visible_poly_framebuffer(bitmap, cliprect, 0x7fc0, 0x7ffe);

	if( m_gametype != NAMCOS21_WINRUN91 )
	{ /* draw low priority 2d sprites */
		c355_obj_draw(screen, bitmap, cliprect, 0 );
		c355_obj_draw(screen, bitmap, cliprect, 1 );
	}

	copy_visible_poly_framebuffer(bitmap, cliprect, 0, 0x7fbf);


	if( m_gametype != NAMCOS21_WINRUN91 )
	{ /* draw high priority 2d sprites */
		for( pri=pivot; pri<8; pri++ )
		{
			c355_obj_draw(screen, bitmap, cliprect, pri );
		}
			c355_obj_draw(screen, bitmap, cliprect, 15 );   //driver's eyes
	}
	else
	{ /* winrun bitmap layer */
		int yscroll = -cliprect.min_y+(INT16)m_winrun_gpu_register[0x2/2];
		int base = 0x1000+0x100*(m_winrun_color&0xf);
		int sx,sy;
		for( sy=cliprect.min_y; sy<=cliprect.max_y; sy++ )
		{
			const UINT8 *pSource = &videoram[((yscroll+sy)&0x3ff)*0x200];
			UINT16 *pDest = &bitmap.pix16(sy);
			for( sx=cliprect.min_x; sx<=cliprect.max_x; sx++ )
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
}

/*********************************************************************************************/

#define SWAP(T,A,B) { const T *temp = A; A = B; B = temp; }

void namcos21_state::renderscanline_flat(const edge *e1, const edge *e2, int sy, unsigned color, int depthcueenable)
{
	if( e1->x > e2->x )
	{
		SWAP(edge,e1,e2);
	}

	{
		UINT16 *pDest = m_mpPolyFrameBufferPens.get() + sy*NAMCOS21_POLY_FRAME_WIDTH;
		UINT16 *pZBuf = m_mpPolyFrameBufferZ.get()    + sy*NAMCOS21_POLY_FRAME_WIDTH;
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
						if( m_gametype == NAMCOS21_WINRUN91 )
						{
							depth = (zz>>10)*0x100;
							pen += depth;
						}
						else if( m_gametype == NAMCOS21_DRIVERS_EYES )
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
}

void namcos21_state::rendertri(const n21_vertex *v0, const n21_vertex *v1, const n21_vertex *v2, unsigned color, int depthcueenable)
{
	int dy,ystart,yend,crop;

	/* first, sort so that v0->y <= v1->y <= v2->y */
	for(;;)
	{
		if( v0->y > v1->y )
		{
			SWAP(n21_vertex,v0,v1);
		}
		else if( v1->y > v2->y )
		{
			SWAP(n21_vertex,v1,v2);
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
				renderscanline_flat(&e1, &e2, y, color, depthcueenable);

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
				renderscanline_flat(&e1, &e2, y, color, depthcueenable);

				e2.x += dx2dy;
				e2.z += dz2dy;

				e1.x += dx1dy;
				e1.z += dz1dy;
			}
		}
	}
}

void namcos21_state::draw_quad(int sx[4], int sy[4], int zcode[4], int color)
{
	n21_vertex a,b,c,d;
	int depthcueenable = 1;
	/*
	    0x0000..0x1fff  sprite palettes (0x20 sets of 0x100 colors)
	    0x2000..0x3fff  polygon palette bank0 (0x10 sets of 0x200 colors or 0x20 sets of 0x100 colors)
	    0x4000..0x5fff  polygon palette bank1 (0x10 sets of 0x200 colors or 0x20 sets of 0x100 colors)
	    0x6000..0x7fff  polygon palette bank2 (0x10 sets of 0x200 colors or 0x20 sets of 0x100 colors)
	*/
	if( m_gametype == NAMCOS21_WINRUN91 )
	{
		color = 0x4000|(color&0xff);
	}
	else if ( m_gametype == NAMCOS21_DRIVERS_EYES )
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

	rendertri(&a, &b, &c, color, depthcueenable);
	rendertri(&c, &d, &a, color, depthcueenable);
}
