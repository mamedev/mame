// license:BSD-3-Clause
// copyright-holders:Hau
#include "emu.h"
#include "includes/galastrm.h"

#define X_OFFSET 96
#define Y_OFFSET 60


galastrm_renderer::galastrm_renderer(galastrm_state& state)
	: poly_manager<float, gs_poly_data, 2, 10000>(state.machine())
	, m_state(state)
	, m_screenbits(state.m_screen->width(), state.m_screen->height())
{
}


/******************************************************************/

void galastrm_state::video_start()
{
	m_spritelist = std::make_unique<gs_tempsprite[]>(0x4000);

	m_poly = std::make_unique<galastrm_renderer>(*this);

	m_screen->register_screen_bitmap(m_tmpbitmaps);
	m_screen->register_screen_bitmap(m_poly->screenbits());
}


/************************************************************
            SPRITE DRAW ROUTINES

We draw a series of small tiles ("chunks") together to
create each big sprite. The spritemap rom provides the lookup
table for this. The game hardware looks up 16x16 sprite chunks
from the spritemap rom, creating a 64x64 sprite like this:

     0  1  2  3
     4  5  6  7
     8  9 10 11
    12 13 14 15

(where the number is the word offset into the spritemap rom).
It can also create 32x32 sprites.

NB: unused portions of the spritemap rom contain hex FF's.
It is a useful coding check to warn in the log if these
are being accessed. [They can be inadvertently while
spriteram is being tested, take no notice of that.]

Heavy use is made of sprite zooming.

        ***

    Sprite table layout (4 long words per entry)

    ------------------------------------------
     0 | ........ x....... ........ ........ | Flip X
     0 | ........ .xxxxxxx ........ ........ | ZoomX
     0 | ........ ........ .xxxxxxx xxxxxxxx | Sprite Tile
       |                                     |
     2 | ........ ....xx.. ........ ........ | Sprite/tile priority [*]
     2 | ........ ......xx xxxxxx.. ........ | Palette bank
     2 | ........ ........ ......xx xxxxxxxx | X position
       |                                     |
     3 | ........ .....x.. ........ ........ | Sprite size (0=32x32, 1=64x64)
     3 | ........ ......x. ........ ........ | Flip Y
     3 | ........ .......x xxxxxx.. ........ | ZoomY
     3 | ........ ........ ......xx xxxxxxxx | Y position
    ------------------------------------------

    [* 00=over BG1; 01=BG2; 10=BG3; 11=over text ???]

********************************************************/

void galastrm_state::draw_sprites_pre(int x_offs, int y_offs)
{
	UINT32 *spriteram32 = m_spriteram;
	UINT16 *spritemap = (UINT16 *)memregion("user1")->base();
	int offs, data, tilenum, color, flipx, flipy;
	int x, y, priority, dblsize, curx, cury;
	int sprites_flipscreen = 0;
	int zoomx, zoomy, zx, zy;
	int sprite_chunk,map_offset,code,j,k,px,py;
	int dimension,total_chunks,bad_chunks;

	/* pdrawgfx() needs us to draw sprites front to back, so we have to build a list
	   while processing sprite ram and then draw them all at the end */
	m_sprite_ptr_pre = m_spritelist.get();

	for (offs = (m_spriteram.bytes()/4-4);offs >= 0;offs -= 4)
	{
		data = spriteram32[offs+0];
		flipx =    (data & 0x00800000) >> 23;
		zoomx =    (data & 0x007f0000) >> 16;
		tilenum =  (data & 0x00007fff);

		if (!tilenum) continue;

		data = spriteram32[offs+2];
		priority = (data & 0x000c0000) >> 18;
		color =    (data & 0x0003fc00) >> 10;
		x =        (data & 0x000003ff);

		data = spriteram32[offs+3];
		dblsize =  (data & 0x00040000) >> 18;
		flipy =    (data & 0x00020000) >> 17;
		zoomy =    (data & 0x0001fc00) >> 10;
		y =        (data & 0x000003ff);

		bad_chunks = 0;
		dimension = ((dblsize*2) + 2);  // 2 or 4
		total_chunks = ((dblsize*3) + 1) << 2;  // 4 or 16
		map_offset = tilenum << 2;

		zoomx += 1;
		zoomy += 1;

		if (x > 713) x -= 1024;     /* 1024x512 */
		if (y < 117) y += 512;

		y = (-y & 0x3ff);
		x -= x_offs;
		y += y_offs;
		if (flipy) y += (128 - zoomy);

		for (sprite_chunk=0;sprite_chunk<total_chunks;sprite_chunk++)
		{
			j = sprite_chunk / dimension;   /* rows */
			k = sprite_chunk % dimension;   /* chunks per row */

			px = k;
			py = j;
			/* pick tiles back to front for x and y flips */
			if (flipx)  px = dimension-1-k;
			if (flipy)  py = dimension-1-j;

			code = spritemap[map_offset + px + (py<<(dblsize+1))];

			if (code==0xffff)
			{
				bad_chunks += 1;
				continue;
			}

			curx = x + ((k*zoomx)/dimension);
			cury = y + ((j*zoomy)/dimension);

			zx= x + (((k+1)*zoomx)/dimension) - curx;
			zy= y + (((j+1)*zoomy)/dimension) - cury;

			if (sprites_flipscreen)
			{
				/* -zx/y is there to fix zoomed sprite coords in screenflip.
				    drawgfxzoom does not know to draw from flip-side of sprites when
				    screen is flipped; so we must correct the coords ourselves. */

				curx = 320 - curx - zx;
				cury = 256 - cury - zy;
				flipx = !flipx;
				flipy = !flipy;
			}

			m_sprite_ptr_pre->gfx = 0;
			m_sprite_ptr_pre->code = code;
			m_sprite_ptr_pre->color = color;
			m_sprite_ptr_pre->flipx = !flipx;
			m_sprite_ptr_pre->flipy = flipy;
			m_sprite_ptr_pre->x = curx;
			m_sprite_ptr_pre->y = cury;
			m_sprite_ptr_pre->zoomx = zx << 12;
			m_sprite_ptr_pre->zoomy = zy << 12;
			m_sprite_ptr_pre->primask = priority;

			m_sprite_ptr_pre++;
		}
		if (bad_chunks)
			logerror("Sprite number %04x had %02x invalid chunks\n",tilenum,bad_chunks);
	}
}

void galastrm_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, const int *primasks, int priority)
{
	struct gs_tempsprite *sprite_ptr = m_sprite_ptr_pre;

	while (sprite_ptr != m_spritelist.get())
	{
		sprite_ptr--;

		if ((priority != 0 && sprite_ptr->primask != 0) ||
			(priority == 0 && sprite_ptr->primask == 0))
		{
			m_gfxdecode->gfx(sprite_ptr->gfx)->prio_zoom_transpen(bitmap,cliprect,
				sprite_ptr->code,
				sprite_ptr->color,
				sprite_ptr->flipx,sprite_ptr->flipy,
				sprite_ptr->x,sprite_ptr->y,
				sprite_ptr->zoomx,sprite_ptr->zoomy,
				screen.priority(),primasks[sprite_ptr->primask],0);
		}
	}
}

/**************************************************************
                POLYGON RENDERER
**************************************************************/

void galastrm_renderer::tc0610_draw_scanline(INT32 scanline, const extent_t& extent, const gs_poly_data& object, int threadid)
{
	UINT16 *framebuffer = &m_screenbits.pix16(scanline);
	const INT32 dudx = extent.param[0].dpdx;
	const INT32 dvdx = extent.param[1].dpdx;

	INT32 u = extent.param[0].start;
	INT32 v = extent.param[1].start;
	for (int x = extent.startx; x < extent.stopx; x++)
	{
		framebuffer[x] = object.texbase->pix16(v >> 16, u >> 16);
		u += dudx;
		v += dvdx;
	}
}

void galastrm_renderer::tc0610_rotate_draw(bitmap_ind16 &srcbitmap, const rectangle &clip)
{
	struct polyVert
	{
		float x;
		float y;
//      float z;
	} tmpz[4];

	vertex_t vert[4];
	int rsx = m_state.m_tc0610_ctrl_reg[1][0];
	int rsy = m_state.m_tc0610_ctrl_reg[1][1];
	const int rzx = m_state.m_tc0610_ctrl_reg[1][2];
	const int rzy = m_state.m_tc0610_ctrl_reg[1][3];
	const int ryx = m_state.m_tc0610_ctrl_reg[1][5];
	const int ryy = m_state.m_tc0610_ctrl_reg[1][4];
	const int lx  = srcbitmap.width();
	const int ly  = srcbitmap.height();

	int yx, /*yy,*/ zx, zy, pxx, pxy, pyx, pyy;
	float /*ssn, scs, ysn, ycs,*/ zsn, zcs;

	pxx = 0;
	pxy = 0;
	pyx = 0;
	pyy = 0;
	zx  = 0;
	zy  = 0;

	if (rzx != 0 || rzy != 0)
	{
		while (sqrtf(powf((float)pxx/4096.0f, 2.0f) + powf((float)pxy/4096.0f, 2.0f)) < (float)(lx / 2))
		{
			pxx += rzx;
			pxy += rzy;
			zx++;
		}
		while (sqrtf(powf((float)pyy/4096.0f, 2.0f) + powf((float)pyx/4096.0f, 2.0f)) < (float)(ly / 2))
		{
			pyy += rzx;
			pyx += -rzy;
			zy++;
		}
	}
	zsn = ((float)pyx/4096.0f) / (float)(ly / 2);
	zcs = ((float)pxx/4096.0f) / (float)(lx / 2);


	if ((rsx == -240 && rsy == 1072) || !m_state.m_tc0610_ctrl_reg[1][7])
	{
		m_state.m_rsxoffs = 0;
		m_state.m_rsyoffs = 0;
	}
	else
	{
		if (rsx > m_state.m_rsxb && m_state.m_rsxb < 0 && rsx-m_state.m_rsxb > 0x8000)
		{
			if (m_state.m_rsxoffs == 0)
				m_state.m_rsxoffs = -0x10000;
			else
				m_state.m_rsxoffs = 0;
		}
		if (rsx < m_state.m_rsxb && m_state.m_rsxb > 0 && m_state.m_rsxb-rsx > 0x8000)
		{
			if (m_state.m_rsxoffs == 0)
				m_state.m_rsxoffs = 0x10000-1;
			else
				m_state.m_rsxoffs = 0;
		}
		if (rsy > m_state.m_rsyb && m_state.m_rsyb < 0 && rsy-m_state.m_rsyb > 0x8000)
		{
			if (m_state.m_rsyoffs == 0)
				m_state.m_rsyoffs = -0x10000;
			else
				m_state.m_rsyoffs = 0;
		}
		if (rsy < m_state.m_rsyb && m_state.m_rsyb > 0 && m_state.m_rsyb-rsy > 0x8000)
		{
			if (m_state.m_rsyoffs == 0)
				m_state.m_rsyoffs = 0x10000-1;
			else
				m_state.m_rsyoffs = 0;
		}
	}
	m_state.m_rsxb = rsx;
	m_state.m_rsyb = rsy;
	if (m_state.m_rsxoffs) rsx += m_state.m_rsxoffs;
	if (m_state.m_rsyoffs) rsy += m_state.m_rsyoffs;
	if (rsx < -0x14000 || rsx >= 0x14000) m_state.m_rsxoffs = 0;
	if (rsy < -0x14000 || rsy >= 0x14000) m_state.m_rsyoffs = 0;


	pxx = 0;
	pxy = 0;
	pyx = 0;
	pyy = 0;
	yx  = 0;
	//yy  = 0;
	//ssn = 0.0;
	//scs = 0.0;
	//ysn = 0.0;
	//ycs = 0.0;

	if (m_state.m_tc0610_ctrl_reg[1][7])
	{
		if (ryx != 0 || ryy != 0)
		{
			while (sqrtf(powf((float)pxx/4096.0f, 2.0f) + powf((float)pxy/4096.0f, 2.0f)) < (float)(lx / 2))
			{
				pxx += ryx;
				pxy += ryy;
				yx++;
			}
			while (sqrtf(powf((float)pyy/4096.0f, 2.0f) + powf((float)pyx/4096.0f, 2.0f)) < (float)(ly / 2))
			{
				pyy += ryx;
				pyx += -ryy;
				//yy++;
			}
			if (yx >= 0.0)
			{
				yx = (int)((8.0 - log((double)yx) / log(2.0)) * 6.0);
				//ysn = sin(DEGREE_TO_RADIAN(yx));
				//ycs = 1.0 - ysn*ysn;
			}
		}

		pxx = 0;
		pxy = 0;
		pyx = 0;
		pyy = 0;

		if (rsx != 0 || rsy != 0)
		{
			while (sqrtf(powf((float)pxx/65536.0f, 2.0) + powf((float)pxy/65536.0f, 2.0f)) < (float)(lx / 2))
			{
				pxx += rsx;
				pxy += rsy;
			}
			while (sqrtf(powf((float)pyy/65536.0f, 2.0f) + powf((float)pyx/65536.0f, 2.0f)) < (float)(ly / 2))
			{
				pyy += rsx;
				pyx += -rsy;
			}
		}
		//ssn = ((float)pxy/65536.0) / (float)(lx / 2);
		//scs = ((float)pyy/65536.0) / (float)(ly / 2);
	}

	{
//      polyVert tmpz[4];
		tmpz[0].x = ((float)(-zx)  * zcs) - ((float)(-zy)  * zsn);
		tmpz[0].y = ((float)(-zx)  * zsn) + ((float)(-zy)  * zcs);
//      tmpz[0].z = 0.0;
		tmpz[1].x = ((float)(-zx)  * zcs) - ((float)(zy-1) * zsn);
		tmpz[1].y = ((float)(-zx)  * zsn) + ((float)(zy-1) * zcs);
//      tmpz[1].z = 0.0;
		tmpz[2].x = ((float)(zx-1) * zcs) - ((float)(zy-1) * zsn);
		tmpz[2].y = ((float)(zx-1) * zsn) + ((float)(zy-1) * zcs);
//      tmpz[2].z = 0.0;
		tmpz[3].x = ((float)(zx-1) * zcs) - ((float)(-zy)  * zsn);
		tmpz[3].y = ((float)(zx-1) * zsn) + ((float)(-zy)  * zcs);
//      tmpz[3].z = 0.0;

		vert[0].x = tmpz[0].x + (float)(lx / 2);
		vert[0].y = tmpz[0].y + (float)(ly / 2);
		vert[1].x = tmpz[1].x + (float)(lx / 2);
		vert[1].y = tmpz[1].y + (float)(ly / 2);
		vert[2].x = tmpz[2].x + (float)(lx / 2);
		vert[2].y = tmpz[2].y + (float)(ly / 2);
		vert[3].x = tmpz[3].x + (float)(lx / 2);
		vert[3].y = tmpz[3].y + (float)(ly / 2);
	}

	vert[0].p[0] = 0.0;
	vert[0].p[1] = 0.0;
	vert[1].p[0] = 0.0;
	vert[1].p[1] = (float)(ly - 1) * 65536.0f;
	vert[2].p[0] = (float)(lx - 1) * 65536.0f;
	vert[2].p[1] = (float)(ly - 1) * 65536.0f;
	vert[3].p[0] = (float)(lx - 1) * 65536.0f;
	vert[3].p[1] = 0.0;

	gs_poly_data& extra = object_data_alloc();
	extra.texbase = &srcbitmap;

	render_polygon<4>(clip, render_delegate(FUNC(galastrm_renderer::tc0610_draw_scanline), this), 2, vert);
	wait();
}

/**************************************************************
                SCREEN REFRESH
**************************************************************/

UINT32 galastrm_state::screen_update_galastrm(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 layer[5];
	UINT8 pivlayer[3];
	UINT16 priority;
	static const int primasks[4] = {0xfffc, 0xfff0, 0xff00, 0x0};
	bitmap_ind8 &priority_bitmap = screen.priority();

	rectangle clip(0, screen.width() -1, 0, screen.height() -1);

	m_tc0100scn->tilemap_update();
	m_tc0480scp->tilemap_update();

	priority = m_tc0480scp->get_bg_priority();
	layer[0] = (priority & 0xf000) >> 12;   /* tells us which bg layer is bottom */
	layer[1] = (priority & 0x0f00) >>  8;
	layer[2] = (priority & 0x00f0) >>  4;
	layer[3] = (priority & 0x000f) >>  0;   /* tells us which is top */
	layer[4] = 4;   /* text layer always over bg layers */

	pivlayer[0] = m_tc0100scn->bottomlayer();
	pivlayer[1] = pivlayer[0] ^ 1;
	pivlayer[2] = 2;

	bitmap.fill(0, cliprect);
	priority_bitmap.fill(0, clip);
	m_tmpbitmaps.fill(0, clip);

	m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, pivlayer[0], 0, 0);
	m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, pivlayer[1], 0, 0);

#if 0
	if (layer[0]==0 && layer[1]==3 && layer[2]==2 && layer[3]==1)
	{
		if (!machine().input().code_pressed(KEYCODE_Z)) m_tc0480scp->tilemap_draw(screen, m_tmpbitmaps, clip, layer[0], 0, 1);
		if (!machine().input().code_pressed(KEYCODE_X)) m_tc0480scp->tilemap_draw(screen, m_tmpbitmaps, clip, layer[1], 0, 4);
		if (!machine().input().code_pressed(KEYCODE_C)) m_tc0480scp->tilemap_draw(screen, m_tmpbitmaps, clip, layer[2], 0, 4);
		if (!machine().input().code_pressed(KEYCODE_V)) m_tc0480scp->tilemap_draw(screen, m_tmpbitmaps, clip, layer[3], 0, 4);
	}
	else
	{
		if (!machine().input().code_pressed(KEYCODE_Z)) m_tc0480scp->tilemap_draw(screen, m_tmpbitmaps, clip, layer[0], 0, 1);
		if (!machine().input().code_pressed(KEYCODE_X)) m_tc0480scp->tilemap_draw(screen, m_tmpbitmaps, clip, layer[1], 0, 2);
		if (!machine().input().code_pressed(KEYCODE_C)) m_tc0480scp->tilemap_draw(screen, m_tmpbitmaps, clip, layer[2], 0, 4);
		if (!machine().input().code_pressed(KEYCODE_V)) m_tc0480scp->tilemap_draw(screen, m_tmpbitmaps, clip, layer[3], 0, 8);
	}

	if (layer[0]==3 && layer[1]==0 && layer[2]==1 && layer[3]==2)
	{
		int x,y;
		UINT8 *pri;

		for (y=0; y < priority_bitmap.height; y++)
		{
			for (x=0; x < priority_bitmap.width; x++)
			{
				pri = &priority_bitmap.pix8(y, x);
				if (!(*pri & 0x02) && m_tmpbitmaps.pix16(y, x))
						*pri |= 0x04;
			}
		}
	}

	draw_sprites_pre(machine(), 42-X_OFFSET, -571+Y_OFFSET);
	draw_sprites(screen,m_tmpbitmaps,clip,primasks,1);

	copybitmap_trans(bitmap,m_polybitmap,0,0, 0,0,cliprect,0);
	m_polybitmap->fill(0, clip);
	tc0610_rotate_draw(machine(),m_polybitmap,m_tmpbitmaps,cliprect);

	priority_bitmap.fill(0, cliprect);
	draw_sprites(screen,bitmap,cliprect,primasks,0);

	if (!machine().input().code_pressed(KEYCODE_B)) m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[4], 0, 0);
	if (!machine().input().code_pressed(KEYCODE_M)) m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, pivlayer[2], 0, 0);



#else
	if (layer[0]==0 && layer[1]==3 && layer[2]==2 && layer[3]==1)
	{
		m_tc0480scp->tilemap_draw(screen, m_tmpbitmaps, clip, layer[0], 0, 1);
		m_tc0480scp->tilemap_draw(screen, m_tmpbitmaps, clip, layer[1], 0, 4);
		m_tc0480scp->tilemap_draw(screen, m_tmpbitmaps, clip, layer[2], 0, 4);
		m_tc0480scp->tilemap_draw(screen, m_tmpbitmaps, clip, layer[3], 0, 4);
	}
	else
	{
		m_tc0480scp->tilemap_draw(screen, m_tmpbitmaps, clip, layer[0], 0, 1);
		m_tc0480scp->tilemap_draw(screen, m_tmpbitmaps, clip, layer[1], 0, 2);
		m_tc0480scp->tilemap_draw(screen, m_tmpbitmaps, clip, layer[2], 0, 4);
		m_tc0480scp->tilemap_draw(screen, m_tmpbitmaps, clip, layer[3], 0, 8);
	}

	if (layer[0]==3 && layer[1]==0 && layer[2]==1 && layer[3]==2)
	{
		int x,y;
		UINT8 *pri;

		for (y=0; y < priority_bitmap.height(); y++)
		{
			for (x=0; x < priority_bitmap.width(); x++)
			{
				pri = &priority_bitmap.pix8(y, x);
				if (!(*pri & 0x02) && m_tmpbitmaps.pix16(y, x))
					*pri |= 0x04;
			}
		}
	}

	draw_sprites_pre(42-X_OFFSET, -571+Y_OFFSET);
	draw_sprites(screen,m_tmpbitmaps,clip,primasks,1);

	copybitmap_trans(bitmap, m_poly->screenbits(), 0,0, 0,0, cliprect, 0);
	m_poly->screenbits().fill(0, clip);
	m_poly->tc0610_rotate_draw(m_tmpbitmaps, cliprect);

	priority_bitmap.fill(0, cliprect);
	draw_sprites(screen,bitmap,cliprect,primasks,0);

	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[4], 0, 0);
	m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, pivlayer[2], 0, 0);
#endif

	return 0;
}
