// license:BSD-3-Clause
// copyright-holders:Luca Elia
/*************************************************************************************************************

                                                -= IGS Lord Of Gun =-

                                          driver by   Luca Elia (l.elia@tin.it)


Note:   if MAME_DEBUG is defined, pressing Z with:

        Q / W / E / R   Shows Layer 0 / 1 / 2 / 3
        A               Shows Sprites

        Keys can be used together!

    [ 4 Scrolling Layers ]

        Tiles          Layer size (pixels)

        8 x 8 x 6bpp   0x800 x 0x200
        8 x 8 x 6bpp   0x200 x 0x100
        16x16 x 6bpp   0x800 x 0x200
        32x32 x 6bpp   0x800 x 0x200

    [ 256 Sprites ]

        Each sprite is made of N x M tiles (up to 16 x 16 tiles).
        Tiles are 16 x 16 x 6bpp

    [ 2048 colors ]

    [ Priorities ]

        RAM based priorities, with a per tile priority code
        (the same sprite goes below some parts, and above others, of the same layer)

*************************************************************************************************************/

#include "emu.h"
#include "lordgun.h"


/***************************************************************************

    Tilemaps

***************************************************************************/

template<int Layer>
TILE_GET_INFO_MEMBER(lordgun_base_state::get_tile_info)
{
	const uint16_t attr  = m_vram[Layer][tile_index * 2 + 0];
	const uint16_t code  = m_vram[Layer][tile_index * 2 + 1];
	const uint16_t color = (attr & 0x0030) >> 4;
	const uint8_t pri    = (attr & 0x0e00) >> 9;
	tileinfo.set(Layer, code, color + 0x10 + 0x4 * ((Layer + 1) & 3) + pri * 0x800 / 0x40, TILE_FLIPXY(attr >> 14));
}

/***************************************************************************

    Video Init

***************************************************************************/


void lordgun_base_state::video_start()
{
	const int w = m_screen->width();
	const int h = m_screen->height();

	// 0x800 x 200
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lordgun_base_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS,  8, 8, 0x100, 0x40);

	// 0x800 x 200
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lordgun_base_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 16,16,  0x80, 0x20);

	// 0x800 x 200
	m_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lordgun_base_state::get_tile_info<2>)), TILEMAP_SCAN_ROWS, 32,32,  0x40, 0x10);

	// 0x200 x 100
	m_tilemap[3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lordgun_base_state::get_tile_info<3>)), TILEMAP_SCAN_ROWS,  8, 8,  0x40, 0x20);

	m_tilemap[0]->set_scroll_rows(1);
	m_tilemap[0]->set_scroll_cols(1);
	m_tilemap[0]->set_transparent_pen(0x3f);

	// Has line scroll
	m_tilemap[1]->set_scroll_rows(0x200);
	m_tilemap[1]->set_scroll_cols(1);
	m_tilemap[1]->set_transparent_pen(0x3f);

	m_tilemap[2]->set_scroll_rows(1);
	m_tilemap[2]->set_scroll_cols(1);
	m_tilemap[2]->set_transparent_pen(0x3f);

	m_tilemap[3]->set_scroll_rows(1);
	m_tilemap[3]->set_scroll_cols(1);
	m_tilemap[3]->set_transparent_pen(0x3f);

	// Buffer bitmaps for 4 tilemaps (0-3) + sprites (4)
	for (int i = 0; i < 5; i++)
		m_bitmaps[i].allocate(w, h);
}

/***************************************************************************

    Gun screen position

***************************************************************************/

static const int lordgun_gun_x_table[] =
{
	-100, 0x001,0x001,0x002,0x002,0x003,0x003,0x004,0x005,0x006,0x007,0x008,0x009,0x00a,0x00b,0x00c,
	0x00d,0x00e,0x00f,0x010,0x011,0x012,0x013,0x014,0x015,0x016,0x017,0x018,0x019,0x01a,0x01b,0x01c,
	0x01d,0x01e,0x01f,0x020,0x021,0x022,0x023,0x024,0x025,0x026,0x027,0x028,0x029,0x02a,0x02b,0x02c,
	0x02d,0x02e,0x02f,0x030,0x031,0x032,0x033,0x034,0x035,0x036,0x037,0x038,0x039,0x03a,0x03b,0x03c,
	0x03d,0x03e,0x03f,0x040,0x041,0x043,0x044,0x045,0x046,0x047,0x048,0x049,0x04a,0x04b,0x04c,0x04e,
	0x04f,0x050,0x051,0x052,0x053,0x054,0x055,0x056,0x057,0x059,0x05a,0x05b,0x05c,0x05d,0x05e,0x05f,
	0x060,0x061,0x05a,0x063,0x065,0x066,0x067,0x068,0x069,0x06a,0x06b,0x06c,0x06d,0x06e,0x06f,0x071,
	0x072,0x074,0x075,0x077,0x078,0x07a,0x07b,0x07d,0x07e,0x080,0x081,0x083,0x085,0x087,0x089,0x08b,
	0x08d,0x08e,0x08f,0x090,0x092,0x093,0x095,0x097,0x098,0x099,0x09a,0x09b,0x09c,0x09d,0x09e,0x0a0,
	0x0a1,0x0a2,0x0a3,0x0a4,0x0a5,0x0a6,0x0a7,0x0a8,0x0a9,0x0aa,0x0ac,0x0ad,0x0ae,0x0af,0x0b0,0x0b1,
	0x0b2,0x0b3,0x0b4,0x0b5,0x0b6,0x0b8,0x0b9,0x0ba,0x0bb,0x0bc,0x0bd,0x0be,0x0bf,0x0c0,0x0c1,0x0c2,
	0x0c4,0x0c5,0x0c6,0x0c7,0x0c8,0x0c9,0x0ca,0x0cb,0x0cc,0x0cd,0x0cf,0x0d0,0x0d1,0x0d2,0x0d3,0x0d4,
	0x0d5,0x0d6,0x0d7,0x0d8,0x0d9,0x0db,0x0dc,0x0dd,0x0de,0x0df,0x0e0,0x0e1,0x0e2,0x0e3,0x0e4,0x0e5,
	0x0e7,0x0e8,0x0e9,0x0ea,0x0eb,0x0ec,0x0ed,0x0ee,0x0ef,0x0f0,0x0f1,0x0f3,0x0f4,0x0f5,0x0f6,0x0f7,
	0x0f8,0x0f9,0x0fa,0x0fb,0x0fc,0x0fe,0x0ff,0x100,0x101,0x102,0x103,0x104,0x105,0x106,0x107,0x108,
	0x10a,0x10b,0x10c,0x10d,0x10e,0x10f,0x110,0x111,0x112,0x113,0x114,0x116,0x117,0x118,0x119,0x11a,
	0x11b,0x11c,0x11d,0x11e,0x11f,0x120,0x122,0x123,0x124,0x125,0x126,0x127,0x128,0x129,0x12a,0x12b,
	0x12c,0x12e,0x12f,0x130,0x131,0x132,0x133,0x134,0x135,0x136,0x137,0x139,0x13a,0x13b,0x13c,0x13d,
	0x13e,0x13f,0x140,0x141,0x142,0x143,0x145,0x146,0x147,0x148,0x149,0x14a,0x14b,0x14c,0x14d,0x14e,
	0x14f,0x151,0x152,0x153,0x154,0x155,0x156,0x157,0x158,0x159,0x15a,0x15b,0x15d,0x15e,0x15f,0x160,
	0x161,0x162,0x163,0x164,0x165,0x166,0x167,0x169,0x16a,0x16b,0x16c,0x16d,0x16e,0x16f,0x170,0x171,
	0x172,0x174,0x175,0x176,0x177,0x178,0x179,0x17a,0x17b,0x17c,0x17d,0x17e,0x17f,0x180,0x181,0x182,
	0x183,0x184,0x185,0x186,0x187,0x188,0x189,0x18a,0x18b,0x18c,0x18d,0x18e,0x18f,0x190,0x191,0x192,
	0x193,0x194,0x195,0x196,0x197,0x198,0x199,0x19a,0x19b,0x19c,0x19d,0x19e,0x19f,0x1a0,0x1a1,0x1a2,
	0x1a3,0x1a4,0x1a5,0x1a6,0x1a7,0x1a8,0x1a9,0x1aa,0x1ab,0x1ac,0x1ad,0x1ae,0x1af,0x1b0,0x1b1,0x1b2,
	0x1b3,0x1b4,0x1b5,0x1b6,0x1b7,0x1b8,0x1b9,0x1ba,0x1bb,0x1bc,0x1bd,0x1be,0x1bf,0x1bf
};

void lordgun_state::calc_gun_scr(int i)
{
	int x = m_in_lightgun_x[i]->read() - 0x3c;

	if ((x < 0) || (x > std::size(lordgun_gun_x_table)))
		x = 0;

	m_gun[i].scr_x = lordgun_gun_x_table[x];
	m_gun[i].scr_y = m_in_lightgun_y[i]->read();
}

void lordgun_state::update_gun(int i)
{
	const rectangle &visarea = m_screen->visible_area();

	m_gun[i].hw_x = m_in_lightgun_x[i]->read();
	m_gun[i].hw_y = m_in_lightgun_y[i]->read();

	calc_gun_scr(i);

	if (!visarea.contains(m_gun[i].scr_x, m_gun[i].scr_y))
		m_gun[i].hw_x = m_gun[i].hw_y = 0;
}


/***************************************************************************

    Sprites


    Offset:     Bits:                   Value:

    0.w         fedc ---- ---- ----     Number of Y Tiles - 1
                ---- ba98 7654 3210     Y

    2.w         f--- ---- ---- ----     Flip X
                -e-- ---- ---- ----     Flip Y
                --dc ---- ---- ----
                ---- ba9- ---- ----     Priority
                ---- ---8 ---- ----     End of Sprite List
                ---- ---- 7654 ----     Color
                ---- ---- ---- 3210     Number of X Tiles - 1

    4.w                                 Tile Code

    6.w                                 X

***************************************************************************/

void lordgun_base_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const uint16_t *s   = m_spriteram;
	const uint16_t *end = m_spriteram + m_spriteram.bytes() / 2;

	for (; s < end; s += 8 / 2)
	{
		int32_t sy          = s[0];
		const uint16_t attr = s[1];
		uint32_t code       = s[2];
		int16_t sx          = s[3];

		// End of sprite list
		if (BIT(attr, 8))
			break;

		const bool flipx     = BIT(attr, 15);
		const bool flipy     = BIT(attr, 14);
		const uint8_t pri    = (attr & 0x0e00) >> 9;
		const uint32_t color = (attr & 0x00f0) >> 4;
		const uint8_t nx     = (attr & 0x000f) + 1;

		const uint8_t ny     = ((sy & 0xf000) >> 12) + 1;

		int x0, x1, dx;
		if (flipx) { x0 = nx - 1; x1 = -1; dx = -1; }
		else       { x0 = 0;      x1 = nx; dx = +1; }

		int y0, y1, dy;
		if (flipy) { y0 = ny - 1; y1 = -1; dy = -1; }
		else       { y0 = 0;      y1 = ny; dy = +1; }

		// Sign extend the position
		sx  -=  0x18;
		sy  =   util::sext(sy & 0xfff, 12);

		for (int y = y0; y != y1; y += dy)
		{
			for (int x = x0; x != x1; x += dx)
			{
				m_gfxdecode->gfx(4)->transpen(bitmap, cliprect,
									code, color + pri * 0x800 / 0x40,
									flipx, flipy,
									sx + x * 0x10, sy + y * 0x10,
									0x3f);
				code += 0x10;
			}

			code += 1 - 0x10 * nx;
		}
	}
}

/***************************************************************************

    Video Update

    Priorities are similar to those in igs/igs011.cpp

    There are 4 scrolling layers, plus sprites, with a per tile priority
    code (0-7).

    Then there are 0x20000 bytes of priority RAM. Each word contains a
    layer code (3-7), where 3 means sprites, and the rest are the tilemaps.
    Actually, the layer code is repeated in both nibbles (e.g. 0x0033).

    For each screen position, to determine which pixel to display, the video
    chip associates a bit to the opacity of that pixel for each layer
    (1 = transparent) to form an address into priority RAM.
    So the bottom 5 bits of the priority RAM address depend on the layer opacities.

    The higher order bits come from the priority of the pixels from each layer
    (not all layers, actually, I guess to save on RAM size).

    The priority RAM value at that address selects the topmost layer, that
    gets sent to the screen.

***************************************************************************/

uint32_t lordgun_base_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// priority code (3-7) -> layer index (0-3, 4 for sprites)
	constexpr int pri2layer[8] = {0, 0, 0, 4, 3, 0, 1, 2};
	// layer index (0-3, 4 for sprites) -> priority address bit
	constexpr int layer2bit[5] = {0, 1, 2, 4, 3};

	int layers_ctrl = -1;

#ifdef MAME_DEBUG
	if (machine().input().code_pressed(KEYCODE_Z))
	{
		int msk = 0;

		if (machine().input().code_pressed(KEYCODE_Q))  msk |= 1;
		if (machine().input().code_pressed(KEYCODE_W))  msk |= 2;
		if (machine().input().code_pressed(KEYCODE_E))  msk |= 4;
		if (machine().input().code_pressed(KEYCODE_R))  msk |= 8;
		if (machine().input().code_pressed(KEYCODE_A))  msk |= 16;
		if (msk != 0) layers_ctrl &= msk;
	}
#endif

	if (m_whitescreen)
	{
		bitmap.fill(m_palette->white_pen(), cliprect);
		return 0;
	}

	// Scrolling

	m_tilemap[0]->set_scrollx(0, *m_scroll_x[0]);
	m_tilemap[0]->set_scrolly(0, *m_scroll_y[0]);

	for (int y = 0; y < 0x200; y++)
		m_tilemap[1]->set_scrollx(y, (*m_scroll_x[1]) + m_scrollram[y * 4/2 + 2/2]);
	m_tilemap[1]->set_scrolly(0, *m_scroll_y[1]);

	m_tilemap[2]->set_scrollx(0, *m_scroll_x[2]);
	m_tilemap[2]->set_scrolly(0, *m_scroll_y[2]);

	m_tilemap[3]->set_scrollx(0, *m_scroll_x[3]);
	m_tilemap[3]->set_scrolly(0, *m_scroll_y[3]);

	// Rendering:

	// render each layer (0-3 tilemaps, 4 sprites) into a buffer bitmap.
	// The priority code of each pixel will be stored into the high 3 bits of the pen

	const pen_t trans_pen = 0 * 0x800 + 0x3f; // pri = 0, pen = 3f (transparent)

	for (int l = 0; l < 5; l++)
		m_bitmaps[l].fill(trans_pen, cliprect);

	if (layers_ctrl & 1)  m_tilemap[0]->draw(screen, m_bitmaps[0], cliprect, 0, 0);
	if (layers_ctrl & 2)  m_tilemap[1]->draw(screen, m_bitmaps[1], cliprect, 0, 0);
	if (layers_ctrl & 4)  m_tilemap[2]->draw(screen, m_bitmaps[2], cliprect, 0, 0);
	if (layers_ctrl & 8)  m_tilemap[3]->draw(screen, m_bitmaps[3], cliprect, 0, 0);
	if (layers_ctrl & 16) draw_sprites(m_bitmaps[4], cliprect);

	// copy to screen bitmap

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			uint16_t pens[5] = {0};

			int pri_addr = 0;

			// bits 0-4: layer transparency
			for (int l = 0; l < 5; l++)
			{
				pens[l] = m_bitmaps[l].pix(y, x);
				if (pens[l] == trans_pen)
					pri_addr |= 1 << layer2bit[l];
			}

			// bits 05-07: layer 1 priority
			pri_addr |= (pens[1] >> 11) << 5;
			// bits 08-10: sprites priority
			pri_addr |= (pens[4] >> 11) << 8;
			// bits 11-13: layer 0 priority
			pri_addr |= (pens[0] >> 11) << 11;
			// bit     14: layer 3 priority
			pri_addr |= (pens[3] >> 11) << 14;

			pri_addr &= 0x7fff;

			const int l = pri2layer[m_priority_ram[pri_addr] & 7];

			bitmap.pix(y, x) = pens[l] & 0x7ff;
		}
	}

	return 0;
}
