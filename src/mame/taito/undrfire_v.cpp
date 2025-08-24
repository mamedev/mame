// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Graves
#include "emu.h"
#include "undrfire.h"
#include "screen.h"



/******************************************************************/

void undrfire_state::video_start()
{
	m_spritelist = std::make_unique<uf_tempsprite[]>(0x4000);

	for (int i = 0; i < 16384; i++) /* Fix later - some weird colours in places */
		m_palette->set_pen_color(i, rgb_t(0,0,0));

	m_frame_counter = 0;
}

/***************************************************************
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

    [*  00=over BG0, 01=BG1, 10=BG2, 11=BG3 ]
    [or 00=over BG1, 01=BG2, 10=BG3, 11=BG3 ]
    [or controlled by TC0360PRI             ]

***************************************************************/

void undrfire_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect,const u32 *primasks,int x_offs,int y_offs)
{
	int sprites_flipscreen = 0;

	/* pdrawgfx() needs us to draw sprites front to back, so we have to build a list
	   while processing sprite ram and then draw them all at the end */
	struct uf_tempsprite *sprite_ptr = m_spritelist.get();

	for (int offs = (m_spriteram.bytes()/4 - 4); offs >= 0; offs -= 4)
	{
		u32 data = m_spriteram[offs+0];
		int flipx =          (data & 0x00800000) >> 23;
		int zoomx =          (data & 0x007f0000) >> 16;
		const u32 tilenum =  (data & 0x00007fff);

		data = m_spriteram[offs+2];
		const int priority = (data & 0x000c0000) >> 18;
		int color =          (data & 0x0003fc00) >> 10;
		int x =              (data & 0x000003ff);

		data = m_spriteram[offs+3];
		const int dblsize =  (data & 0x00040000) >> 18;
		int flipy =          (data & 0x00020000) >> 17;
		int zoomy =          (data & 0x0001fc00) >> 10;
		int y =              (data & 0x000003ff);

		color |= (0x100 + (priority << 6));     /* priority bits select color bank */
		color /= 2;     /* as sprites are 5bpp */
		flipy = !flipy;
		y = (-y & 0x3ff);

		if (!tilenum) continue;

		flipy = !flipy;
		zoomx += 1;
		zoomy += 1;

		y += y_offs;

		/* treat coords as signed */
		if (x > 0x340) x -= 0x400;
		if (y > 0x340) y -= 0x400;

		x -= x_offs;

		int bad_chunks = 0;
		const int dimension = ((dblsize*2) + 2);  // 2 or 4
		const int total_chunks = ((dblsize*3) + 1) << 2;  // 4 or 16
		const int map_offset = tilenum << 2;

		for (int sprite_chunk = 0; sprite_chunk < total_chunks; sprite_chunk++)
		{
			const int j = sprite_chunk / dimension;   /* rows */
			const int k = sprite_chunk % dimension;   /* chunks per row */

			int px = k;
			int py = j;
			/* pick tiles back to front for x and y flips */
			if (flipx)  px = dimension - 1 - k;
			if (flipy)  py = dimension - 1 - j;

			const u16 code = m_spritemap[map_offset + px + (py << (dblsize + 1))];

			if (code == 0xffff)
			{
				bad_chunks += 1;
				continue;
			}

			int curx = x + ((k * zoomx) / dimension);
			int cury = y + ((j * zoomy) / dimension);

			const int zx = x + (((k + 1) * zoomx) / dimension) - curx;
			const int zy = y + (((j + 1) * zoomy) / dimension) - cury;

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

			sprite_ptr->gfx = 0;
			sprite_ptr->code = code;
			sprite_ptr->color = color;
			sprite_ptr->flipx = !flipx;
			sprite_ptr->flipy = flipy;
			sprite_ptr->x = curx;
			sprite_ptr->y = cury;
			sprite_ptr->zoomx = zx << 12;
			sprite_ptr->zoomy = zy << 12;

			if (primasks)
			{
				sprite_ptr->primask = primasks[priority];

				sprite_ptr++;
			}
			else
			{
				m_gfxdecode->gfx(sprite_ptr->gfx)->zoom_transpen(bitmap,cliprect,
						sprite_ptr->code,
						sprite_ptr->color,
						sprite_ptr->flipx,sprite_ptr->flipy,
						sprite_ptr->x,sprite_ptr->y,
						sprite_ptr->zoomx,sprite_ptr->zoomy,0);
			}
		}

		if (bad_chunks)
logerror("Sprite number %04x had %02x invalid chunks\n",tilenum,bad_chunks);
	}

	/* this happens only if primsks != nullptr */
	while (sprite_ptr != m_spritelist.get())
	{
		sprite_ptr--;

		m_gfxdecode->gfx(sprite_ptr->gfx)->prio_zoom_transpen(bitmap,cliprect,
				sprite_ptr->code,
				sprite_ptr->color,
				sprite_ptr->flipx,sprite_ptr->flipy,
				sprite_ptr->x,sprite_ptr->y,
				sprite_ptr->zoomx,sprite_ptr->zoomy,
				screen.priority(),sprite_ptr->primask,0);
	}
}


void undrfire_state::draw_sprites_cbombers(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect,const u8 *pritable,int x_offs,int y_offs)
{
	int sprites_flipscreen = 0;

	/* pdrawgfx() needs us to draw sprites front to back, so we have to build a list
	   while processing sprite ram and then draw them all at the end */
	struct uf_tempsprite *sprite_ptr = m_spritelist.get();

	for (int offs = (m_spriteram.bytes()/4 - 4); offs >= 0; offs -= 4)
	{
		u32 data = m_spriteram[offs+0];
		int flipx =          (data & 0x00800000) >> 23;
		int zoomx =          (data & 0x007f0000) >> 16;
		const u32 tilenum =  (data & 0x0000ffff);

		data = m_spriteram[offs+2];
		const int priority = (data & 0x000c0000) >> 18;
		u16 color =          (data & 0x0003fc00) >> 10;
		int x =              (data & 0x000003ff);

		data = m_spriteram[offs+3];
		const int dblsize = (data & 0x00040000) >> 18;
		int flipy =         (data & 0x00020000) >> 17;
		int zoomy =         (data & 0x0001fc00) >> 10;
		int y =             (data & 0x000003ff);

		color |= (/*0x100 +*/ (priority << 6));     /* priority bits select color bank */

		color /= 2;     /* as sprites are 5bpp */
		flipy = !flipy;

		if (!tilenum) continue;

		zoomx += 1;
		zoomy += 1;

		y += y_offs;

		/* treat coords as signed */
		if (x > 0x340) x -= 0x400;
		if (y > 0x340) y -= 0x400;

		x -= x_offs;

		const int dimension = ((dblsize * 2) + 2);  // 2 or 4
		const int total_chunks = ((dblsize * 3) + 1) << 2;  // 4 or 16
		const int map_offset = tilenum << 2;

		for (int sprite_chunk = 0; sprite_chunk < total_chunks; sprite_chunk++)
		{
			const int j = sprite_chunk / dimension;   /* rows */
			const int k = sprite_chunk % dimension;   /* chunks per row */

			int px = k;
			int py = j;
			/* pick tiles back to front for x and y flips */
			if (flipx)  px = dimension - 1 - k;
			if (flipy)  py = dimension - 1 - j;

			const u32 map_addr = map_offset + px + (py << (dblsize + 1));
			const u32 code     = (m_spritemaphi[map_addr] << 16) | m_spritemap[map_addr];

			int curx = x + ((k * zoomx) / dimension);
			int cury = y + ((j * zoomy) / dimension);

			const int zx = x + (((k + 1) * zoomx) / dimension) - curx;
			const int zy = y + (((j + 1) * zoomy) / dimension) - cury;

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

			sprite_ptr->gfx = 0;
			sprite_ptr->code = code;
			sprite_ptr->color = color;
			sprite_ptr->flipx = !flipx;
			sprite_ptr->flipy = flipy;
			sprite_ptr->x = curx;
			sprite_ptr->y = cury;
			sprite_ptr->zoomx = zx << 12;
			sprite_ptr->zoomy = zy << 12;

			if (pritable)
			{
				sprite_ptr->primask = u32(~1) << pritable[priority];
				sprite_ptr++;
			}
			else
			{
				m_gfxdecode->gfx(sprite_ptr->gfx)->zoom_transpen(bitmap,cliprect,
						sprite_ptr->code,
						sprite_ptr->color,
						sprite_ptr->flipx,sprite_ptr->flipy,
						sprite_ptr->x,sprite_ptr->y,
						sprite_ptr->zoomx,sprite_ptr->zoomy,0);
			}
		}
	}

	/* this happens only if primsks != nullptr */
	while (sprite_ptr != m_spritelist.get())
	{
		sprite_ptr--;

		m_gfxdecode->gfx(sprite_ptr->gfx)->prio_zoom_transpen(bitmap,cliprect,
				sprite_ptr->code,
				sprite_ptr->color,
				sprite_ptr->flipx,sprite_ptr->flipy,
				sprite_ptr->x,sprite_ptr->y,
				sprite_ptr->zoomx,sprite_ptr->zoomy,
				screen.priority(),sprite_ptr->primask,0);
	}
}


/**************************************************************
                SCREEN REFRESH
**************************************************************/

u32 undrfire_state::screen_update_undrfire(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u8 layer[5];
	u8 scclayer[3];

#ifdef MAME_DEBUG
	if (machine().input().code_pressed_once (KEYCODE_X))
	{
		m_dislayer[5] ^= 1;
		popmessage("scc text: %01x",m_dislayer[5]);
	}
	if (machine().input().code_pressed_once (KEYCODE_C))
	{
		m_dislayer[0] ^= 1;
		popmessage("bg0: %01x",m_dislayer[0]);
	}

	if (machine().input().code_pressed_once (KEYCODE_V))
	{
		m_dislayer[1] ^= 1;
		popmessage("bg1: %01x",m_dislayer[1]);
	}

	if (machine().input().code_pressed_once (KEYCODE_B))
	{
		m_dislayer[2] ^= 1;
		popmessage("bg2: %01x",m_dislayer[2]);
	}

	if (machine().input().code_pressed_once (KEYCODE_N))
	{
		m_dislayer[3] ^= 1;
		popmessage("bg3: %01x",m_dislayer[3]);
	}

	if (machine().input().code_pressed_once (KEYCODE_M))
	{
		m_dislayer[4] ^= 1;
		popmessage("sprites: %01x",m_dislayer[4]);
	}
#endif

	m_tc0620scc->tilemap_update();
	m_tc0480scp->tilemap_update();

	const u16 priority = m_tc0480scp->get_bg_priority();

	layer[0] = (priority & 0xf000) >> 12;   /* tells us which bg layer is bottom */
	layer[1] = (priority & 0x0f00) >>  8;
	layer[2] = (priority & 0x00f0) >>  4;
	layer[3] = (priority & 0x000f) >>  0;   /* tells us which is top */
	layer[4] = 4;   /* text layer always over bg layers */

	scclayer[0] = m_tc0620scc->bottomlayer();
	scclayer[1] = scclayer[0] ^ 1;
	scclayer[2] = 2;

	screen.priority().fill(0, cliprect);
	bitmap.fill(0, cliprect);   /* wrong color? */


/* The "SCC" chip seems to be a 6bpp TC0100SCN. It has a
   bottom layer usually full of bright garish colors that
   vaguely mimic the structure of the layers on top. Seems
   pointless - it's always hidden by other layers. Does it
   serve some blending pupose ? */

	m_tc0620scc->tilemap_draw(screen, bitmap, cliprect, scclayer[0], TILEMAP_DRAW_OPAQUE, 0);
	m_tc0620scc->tilemap_draw(screen, bitmap, cliprect, scclayer[1], 0, 0);

#ifdef MAME_DEBUG
	if (m_dislayer[layer[0]]==0)
#endif
	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[0], 0, 1);

#ifdef MAME_DEBUG
	if (m_dislayer[layer[1]]==0)
#endif
	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 2);

#ifdef MAME_DEBUG
	if (m_dislayer[layer[2]]==0)
#endif
	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[2], 0, 4);

#ifdef MAME_DEBUG
	if (m_dislayer[layer[3]]==0)
#endif
	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[3], 0, 8);

#ifdef MAME_DEBUG
	if (m_dislayer[4]==0)
#endif
	/* Sprites have variable priority (we kludge this on road levels) */
	{
		if ((m_tc0480scp->pri_reg_r() & 0x3) == 3)  /* on road levels kludge sprites up 1 priority */
		{
			static const u32 primasks[4] = {0xfff0, 0xff00, 0x0, 0x0};
			draw_sprites(screen, bitmap, cliprect, primasks, 44, -574);
		}
		else
		{
			static const u32 primasks[4] = {0xfffc, 0xfff0, 0xff00, 0x0};
			draw_sprites(screen, bitmap, cliprect, primasks, 44, -574);
		}
	}

#ifdef MAME_DEBUG
	if (m_dislayer[5]==0)
#endif
	m_tc0620scc->tilemap_draw(screen, bitmap, cliprect, scclayer[2], 0, 0); /* TC0620SCC text layer */

	m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[4], 0, 0);    /* TC0480SCP text layer */

/* Enable this to see rotation (?) control words */
#if 0
	{
		for (auto i = 0; i < 8; i += 1)
		{
			auto buf = util::string_format("%02x: %04x", i, m_rotate_ctrl[i]);
			ui_draw_text (buf, 0, i*8);
		}
	}
#endif
	return 0;
}


/*
    TC0360PRI Priority format for chase bombers

    Offset Bits      Description
           7654 3210
    00     0001 1100 Unknown
    01     0000 1111 Unknown
    04     xxxx ---- TC0480SCP Layer 3 Priority
           ---- xxxx TC0480SCP Layer 2 Priority
    05     xxxx ---- TC0480SCP Layer 1 Priority
           ---- xxxx TC0480SCP Layer 0 Priority
    06     xxxx ---- TC0480SCP Text Layer Priority
           ---- 0000 Unknown
    07     xxxx ---- TC0620SCC Layer 0 Priority
           ---- xxxx TC0620SCC Layer 1 Priority
    08     xxxx ---- Sprite Priority Bank 1
           ---- xxxx Sprite Priority Bank 0
    09     xxxx ---- Sprite Priority Bank 3
           ---- xxxx Sprite Priority Bank 2

    Values are 0 (Bottommost) ... f (Topmost)
    Other registers are unknown/unused
*/

u32 undrfire_state::screen_update_cbombers(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u8 layer[5];
	u8 scclayer[3];

#ifdef MAME_DEBUG
	if (machine().input().code_pressed_once (KEYCODE_C))
	{
		m_dislayer[0] ^= 1;
		popmessage("bg0: %01x",m_dislayer[0]);
	}

	if (machine().input().code_pressed_once (KEYCODE_V))
	{
		m_dislayer[1] ^= 1;
		popmessage("bg1: %01x",m_dislayer[1]);
	}

	if (machine().input().code_pressed_once (KEYCODE_B))
	{
		m_dislayer[2] ^= 1;
		popmessage("bg2: %01x",m_dislayer[2]);
	}

	if (machine().input().code_pressed_once (KEYCODE_N))
	{
		m_dislayer[3] ^= 1;
		popmessage("bg3: %01x",m_dislayer[3]);
	}

	if (machine().input().code_pressed_once (KEYCODE_X))
	{
		m_dislayer[4] ^= 1;
		popmessage("text: %01x",m_dislayer[4]);
	}

	if (machine().input().code_pressed_once (KEYCODE_M))
	{
		m_dislayer[5] ^= 1;
		popmessage("sprites: %01x",m_dislayer[5]);
	}
#endif

	m_tc0620scc->tilemap_update();
	m_tc0480scp->tilemap_update();

	const u16 priority = m_tc0480scp->get_bg_priority();

	layer[0] = (priority & 0xf000) >> 12;   /* tells us which bg layer is bottom */
	layer[1] = (priority & 0x0f00) >>  8;
	layer[2] = (priority & 0x00f0) >>  4;
	layer[3] = (priority & 0x000f) >>  0;   /* tells us which is top */
	layer[4] = 4;   /* text layer always over bg layers */

	scclayer[0] = m_tc0620scc->bottomlayer();
	scclayer[1] = scclayer[0] ^ 1;
	scclayer[2] = 2;

	u8 tc0480scp_pri[5];
	u8 tc0620scc_pri[2];
	u8 sprite_pri[4];

	// parse priority values
	tc0480scp_pri[layer[0]] = m_tc0360pri->read(5) & 0x0f;
	tc0480scp_pri[layer[1]] = (m_tc0360pri->read(5) >> 4) & 0x0f;
	tc0480scp_pri[layer[2]] = m_tc0360pri->read(4) & 0x0f;
	tc0480scp_pri[layer[3]] = (m_tc0360pri->read(4) >> 4) & 0x0f;
	tc0480scp_pri[layer[4]] = (m_tc0360pri->read(6) >> 4) & 0x0f;

	tc0620scc_pri[scclayer[0]] = (m_tc0360pri->read(7) >> 4) & 0x0f;
	tc0620scc_pri[scclayer[1]] = m_tc0360pri->read(7) & 0x0f;

	sprite_pri[0] = m_tc0360pri->read(8) & 0x0f;
	sprite_pri[1] = (m_tc0360pri->read(8) >> 4) & 0x0f;
	sprite_pri[2] = m_tc0360pri->read(9) & 0x0f;
	sprite_pri[3] = (m_tc0360pri->read(9) >> 4) & 0x0f;

	screen.priority().fill(0, cliprect);
	bitmap.fill(0, cliprect);   /* wrong color? */

	for (int p = 0; p < 16; p++)
	{
		// TODO: verify layer order when multiple tilemap layers has same priority value
		const u8 prival = p + 1; // +1 for pdrawgfx

		for (int scc = 0; scc < 2; scc++)
		{
			if (tc0620scc_pri[scclayer[scc]] == p)
				m_tc0620scc->tilemap_draw(screen, bitmap, cliprect, scclayer[scc], (scc == 0) ? TILEMAP_DRAW_OPAQUE : 0, prival, 0);
		}

		for (int scp = 0; scp < 4; scp++)
		{
#ifdef MAME_DEBUG
			if (m_dislayer[layer[scp]]==0)
#endif
			if (tc0480scp_pri[layer[scp]] == p)
				m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[scp], 0, prival, 0);
		}
#ifdef MAME_DEBUG
		if (m_dislayer[layer[4]]==0)
#endif
		if (tc0480scp_pri[layer[4]] == p)
			m_tc0480scp->tilemap_draw(screen, bitmap, cliprect, layer[4], 0, prival, 0);
	}

	/* Sprites have variable priority */
#ifdef MAME_DEBUG
	if (m_dislayer[5]==0)
#endif
	draw_sprites_cbombers(screen, bitmap, cliprect, sprite_pri, 80, -208);

	m_tc0620scc->tilemap_draw(screen, bitmap, cliprect, scclayer[2], 0, 0); // TODO: correct?

/* Enable this to see rotation (?) control words */
#if 0
	{
		for (auto i = 0; i < 8; i += 1)
		{
			auto buf = util::string_format("%02x: %04x", i, m_rotate_ctrl[i]);
			ui_draw_text (buf, 0, i*8);
		}
	}
#endif
	return 0;
}
