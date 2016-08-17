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

        Tiles       Layer size

        8 x 8 x 6   0x800 x 0x200
        8 x 8 x 6   0x200 x 0x100
        16x16 x 6   0x800 x 0x200
        32x32 x 6   0x800 x 0x200

    [ 256 Sprites ]

        Each sprite is made of N x M tiles (up to 16 x 16 tiles).
        Tiles are 16 x 16 x 6

    [ 2048 colors ]

    [ Priorities ]

        RAM based priorities, with a per tile priority code
        (the same sprite goes below some parts, and above others, of the same layer)

*************************************************************************************************************/

#include "emu.h"
#include "includes/lordgun.h"


/***************************************************************************

    Palette

***************************************************************************/

// xxxxBBBBGGGGRRRR, but repeat ecah color for each priority code (since we stuff it in the high bits of the pen)
WRITE16_MEMBER(lordgun_state::lordgun_paletteram_w)
{
	COMBINE_DATA(&m_generic_paletteram_16[offset]);
	for (int pri = 0; pri < 8; pri++)
		m_palette->set_pen_color(offset+0x800*pri, pal4bit(data >> 0), pal4bit(data >> 4), pal4bit(data >> 8));
}


/***************************************************************************

    Tilemaps

***************************************************************************/


inline void lordgun_state::get_tile_info(tile_data &tileinfo, tilemap_memory_index tile_index, int _N_)
{
	UINT16 attr = m_vram[_N_][tile_index * 2 + 0 ];
	UINT16 code = m_vram[_N_][ tile_index * 2 + 1 ];
	UINT16 pri  = (attr & 0x0e00) >> 9;
	SET_TILE_INFO_MEMBER(_N_, code, ((attr & 0x0030) >> 4) + 0x10 + 0x4 * ((_N_ + 1) & 3) + pri*0x800/0x40, TILE_FLIPXY(attr >> 14));
}

TILE_GET_INFO_MEMBER(lordgun_state::get_tile_info_0){ get_tile_info(tileinfo, tile_index, 0); }
TILE_GET_INFO_MEMBER(lordgun_state::get_tile_info_1){ get_tile_info(tileinfo, tile_index, 1); }
TILE_GET_INFO_MEMBER(lordgun_state::get_tile_info_2){ get_tile_info(tileinfo, tile_index, 2); }
TILE_GET_INFO_MEMBER(lordgun_state::get_tile_info_3){ get_tile_info(tileinfo, tile_index, 3); }

inline void lordgun_state::lordgun_vram_w(offs_t offset, UINT16 data, UINT16 mem_mask, int _N_)
{
	COMBINE_DATA(&m_vram[_N_][offset]);
	m_tilemap[_N_]->mark_tile_dirty(offset/2);
}

WRITE16_MEMBER(lordgun_state::lordgun_vram_0_w){ lordgun_vram_w(offset, data, mem_mask, 0); }
WRITE16_MEMBER(lordgun_state::lordgun_vram_1_w){ lordgun_vram_w(offset, data, mem_mask, 1); }
WRITE16_MEMBER(lordgun_state::lordgun_vram_2_w){ lordgun_vram_w(offset, data, mem_mask, 2); }
WRITE16_MEMBER(lordgun_state::lordgun_vram_3_w){ lordgun_vram_w(offset, data, mem_mask, 3); }

/***************************************************************************

    Video Init

***************************************************************************/


void lordgun_state::video_start()
{
	int i;
	int w = m_screen->width();
	int h = m_screen->height();

	// 0x800 x 200
	m_tilemap[0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(lordgun_state::get_tile_info_0),this), TILEMAP_SCAN_ROWS,8,8, 0x100, 0x40 );

	// 0x800 x 200
	m_tilemap[1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(lordgun_state::get_tile_info_1),this), TILEMAP_SCAN_ROWS,16,16, 0x80,0x20 );

	// 0x800 x 200
	m_tilemap[2] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(lordgun_state::get_tile_info_2),this), TILEMAP_SCAN_ROWS,32,32, 0x40,0x10 );

	// 0x200 x 100
	m_tilemap[3] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(lordgun_state::get_tile_info_3),this), TILEMAP_SCAN_ROWS,8,8, 0x40,0x20 );

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
	for (i = 0; i < 5; i++)
		m_bitmaps[i] = std::make_unique<bitmap_ind16>(w, h);
}

/***************************************************************************

    Gun screen position

***************************************************************************/

static const int lordgun_gun_x_table[] =
{
	-100, 0x001,0x001,0x002,0x002,0x003,0x003,0x004,0x005,0x006,0x007,0x008,0x009,0x00A,0x00B,0x00C,
	0x00D,0x00E,0x00F,0x010,0x011,0x012,0x013,0x014,0x015,0x016,0x017,0x018,0x019,0x01A,0x01B,0x01C,
	0x01D,0x01E,0x01F,0x020,0x021,0x022,0x023,0x024,0x025,0x026,0x027,0x028,0x029,0x02A,0x02B,0x02C,
	0x02D,0x02E,0x02F,0x030,0x031,0x032,0x033,0x034,0x035,0x036,0x037,0x038,0x039,0x03A,0x03B,0x03C,
	0x03D,0x03E,0x03F,0x040,0x041,0x043,0x044,0x045,0x046,0x047,0x048,0x049,0x04A,0x04B,0x04C,0x04E,
	0x04F,0x050,0x051,0x052,0x053,0x054,0x055,0x056,0x057,0x059,0x05A,0x05B,0x05C,0x05D,0x05E,0x05F,
	0x060,0x061,0x05A,0x063,0x065,0x066,0x067,0x068,0x069,0x06A,0x06B,0x06C,0x06D,0x06E,0x06F,0x071,
	0x072,0x074,0x075,0x077,0x078,0x07A,0x07B,0x07D,0x07E,0x080,0x081,0x083,0x085,0x087,0x089,0x08B,
	0x08D,0x08E,0x08F,0x090,0x092,0x093,0x095,0x097,0x098,0x099,0x09A,0x09B,0x09C,0x09D,0x09E,0x0A0,
	0x0A1,0x0A2,0x0A3,0x0A4,0x0A5,0x0A6,0x0A7,0x0A8,0x0A9,0x0AA,0x0AC,0x0AD,0x0AE,0x0AF,0x0B0,0x0B1,
	0x0B2,0x0B3,0x0B4,0x0B5,0x0B6,0x0B8,0x0B9,0x0BA,0x0BB,0x0BC,0x0BD,0x0BE,0x0BF,0x0C0,0x0C1,0x0C2,
	0x0C4,0x0C5,0x0C6,0x0C7,0x0C8,0x0C9,0x0CA,0x0CB,0x0CC,0x0CD,0x0CF,0x0D0,0x0D1,0x0D2,0x0D3,0x0D4,
	0x0D5,0x0D6,0x0D7,0x0D8,0x0D9,0x0DB,0x0DC,0x0DD,0x0DE,0x0DF,0x0E0,0x0E1,0x0E2,0x0E3,0x0E4,0x0E5,
	0x0E7,0x0E8,0x0E9,0x0EA,0x0EB,0x0EC,0x0ED,0x0EE,0x0EF,0x0F0,0x0F1,0x0F3,0x0F4,0x0F5,0x0F6,0x0F7,
	0x0F8,0x0F9,0x0FA,0x0FB,0x0FC,0x0FE,0x0FF,0x100,0x101,0x102,0x103,0x104,0x105,0x106,0x107,0x108,
	0x10A,0x10B,0x10C,0x10D,0x10E,0x10F,0x110,0x111,0x112,0x113,0x114,0x116,0x117,0x118,0x119,0x11A,
	0x11B,0x11C,0x11D,0x11E,0x11F,0x120,0x122,0x123,0x124,0x125,0x126,0x127,0x128,0x129,0x12A,0x12B,
	0x12C,0x12E,0x12F,0x130,0x131,0x132,0x133,0x134,0x135,0x136,0x137,0x139,0x13A,0x13B,0x13C,0x13D,
	0x13E,0x13F,0x140,0x141,0x142,0x143,0x145,0x146,0x147,0x148,0x149,0x14A,0x14B,0x14C,0x14D,0x14E,
	0x14F,0x151,0x152,0x153,0x154,0x155,0x156,0x157,0x158,0x159,0x15A,0x15B,0x15D,0x15E,0x15F,0x160,
	0x161,0x162,0x163,0x164,0x165,0x166,0x167,0x169,0x16A,0x16B,0x16C,0x16D,0x16E,0x16F,0x170,0x171,
	0x172,0x174,0x175,0x176,0x177,0x178,0x179,0x17A,0x17B,0x17C,0x17D,0x17E,0x17F,0x180,0x181,0x182,
	0x183,0x184,0x185,0x186,0x187,0x188,0x189,0x18A,0x18B,0x18C,0x18D,0x18E,0x18F,0x190,0x191,0x192,
	0x193,0x194,0x195,0x196,0x197,0x198,0x199,0x19A,0x19B,0x19C,0x19D,0x19E,0x19F,0x1A0,0x1A1,0x1A2,
	0x1A3,0x1A4,0x1A5,0x1A6,0x1A7,0x1A8,0x1A9,0x1AA,0x1AB,0x1AC,0x1AD,0x1AE,0x1AF,0x1B0,0x1B1,0x1B2,
	0x1B3,0x1B4,0x1B5,0x1B6,0x1B7,0x1B8,0x1B9,0x1BA,0x1BB,0x1BC,0x1BD,0x1BE,0x1BF,0x1BF
};

static const char *const gunnames[] = { "LIGHT0_X", "LIGHT1_X", "LIGHT0_Y", "LIGHT1_Y" };

float lordgun_crosshair_mapper(ioport_field *field, float linear_value)
{
	int x = linear_value - 0x3c;

	if ( (x < 0) || (x > ARRAY_LENGTH(lordgun_gun_x_table)) )
		x = 0;

	return lordgun_gun_x_table[x] * 1.0f / 0x1BF;
}

void lordgun_state::lorddgun_calc_gun_scr(int i)
{
//  popmessage("%03x, %02x", machine, "LIGHT0_X"), ioport("LIGHT0_Y")->read());

	int x = ioport(gunnames[i])->read() - 0x3c;

	if ( (x < 0) || (x > ARRAY_LENGTH(lordgun_gun_x_table)) )
		x = 0;

	m_gun[i].scr_x = lordgun_gun_x_table[x];
	m_gun[i].scr_y = ioport(gunnames[i+2])->read();
}

void lordgun_state::lordgun_update_gun(int i)
{
	const rectangle &visarea = m_screen->visible_area();

	m_gun[i].hw_x = ioport(gunnames[i])->read();
	m_gun[i].hw_y = ioport(gunnames[i+2])->read();

	lorddgun_calc_gun_scr(i);

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

void lordgun_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT16 *s       =   m_spriteram;
	UINT16 *end     =   m_spriteram + m_spriteram.bytes()/2;

	for ( ; s < end; s += 8/2 )
	{
		int attr, code, color, pri;
		int sx, nx, x, x0, x1, dx, flipx;
		int sy, ny, y, y0, y1, dy, flipy;

		sy      =       s[ 0 ];
		attr    =       s[ 1 ];
		code    =       s[ 2 ];
		sx      =       s[ 3 ];

		// End of sprite list
		if (attr & 0x0100)
			break;

		flipx   =    attr & 0x8000;
		flipy   =    attr & 0x4000;
		pri     =   (attr & 0x0e00) >> 9;
		color   =   (attr & 0x00f0) >> 4;
		nx      =   (attr & 0x000f) + 1;

		ny      =   ((sy & 0xf000) >> 12) + 1;

		if ( flipx )    {   x0 = nx - 1;    x1 = -1;    dx = -1;    }
		else            {   x0 = 0;         x1 = nx;    dx = +1;    }

		if ( flipy )    {   y0 = ny - 1;    y1 = -1;    dy = -1;    }
		else            {   y0 = 0;         y1 = ny;    dy = +1;    }

		// Sign extend the position
		sx  -=  0x18;
		sy  =   (sy & 0x7ff) - (sy & 0x800);

		for (y = y0; y != y1; y += dy)
		{
			for (x = x0; x != x1; x += dx)
			{
				m_gfxdecode->gfx(4)->transpen(bitmap,cliprect,
									code, color + pri * 0x800/0x40,
									flipx, flipy,
									sx + x * 0x10, sy + y * 0x10,
									0x3f    );
				code += 0x10;
			}

			code += 1 - 0x10 * nx;
		}
	}
}

/***************************************************************************

    Video Update

    Priorities are similar to those in igs011.c

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

UINT32 lordgun_state::screen_update_lordgun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
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

	int x, y;

	m_tilemap[0]->set_scrollx(0, *m_scroll_x[0] );
	m_tilemap[0]->set_scrolly(0, *m_scroll_y[0] );

	for (y = 0; y < 0x200; y++)
		m_tilemap[1]->set_scrollx(y, (*m_scroll_x[1]) + m_scrollram[y * 4/2 + 2/2]);
	m_tilemap[1]->set_scrolly(0, *m_scroll_y[1] );

	m_tilemap[2]->set_scrollx(0, *m_scroll_x[2] );
	m_tilemap[2]->set_scrolly(0, *m_scroll_y[2] );

	m_tilemap[3]->set_scrollx(0, *m_scroll_x[3] );
	m_tilemap[3]->set_scrolly(0, *m_scroll_y[3] );

	// Rendering:

	// render each layer (0-3 tilemaps, 4 sprites) into a buffer bitmap.
	// The priority code of each pixel will be stored into the high 3 bits of the pen

	pen_t trans_pen = 0 * 0x800 + 0x3f; // pri = 0, pen = 3f (transparent)

	int l;
	for (l = 0; l < 5; l++)
		m_bitmaps[l]->fill(trans_pen, cliprect);

	if (layers_ctrl & 1)    m_tilemap[0]->draw(screen, *m_bitmaps[0], cliprect, 0, 0);
	if (layers_ctrl & 2)    m_tilemap[1]->draw(screen, *m_bitmaps[1], cliprect, 0, 0);
	if (layers_ctrl & 4)    m_tilemap[2]->draw(screen, *m_bitmaps[2], cliprect, 0, 0);
	if (layers_ctrl & 8)    m_tilemap[3]->draw(screen, *m_bitmaps[3], cliprect, 0, 0);
	if (layers_ctrl & 16)   draw_sprites(*m_bitmaps[4], cliprect);

	// copy to screen bitmap

	// priority code (3-7) -> layer index (0-3, 4 for sprites)
	const int pri2layer[8] = {0,0,0,4,3,0,1,2};
	// layer index (0-3, 4 for sprites) -> priority address bit
	const int layer2bit[5] = {0,1,2,4,3};

	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		for (x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			UINT16 pens[5];

			int pri_addr = 0;

			// bits 0-4: layer transparency
			for (l = 0; l < 5; l++)
			{
				pens[l] = m_bitmaps[l]->pix16(y, x);
				if (pens[l] == trans_pen)
					pri_addr |= 1 << layer2bit[l];
			}

			// bits 05-07: layer 1 priority
			pri_addr |= (pens[1]>>11) << 5;
			// bits 08-10: sprites priority
			pri_addr |= (pens[4]>>11) << 8;
			// bits 11-13: layer 0 priority
			pri_addr |= (pens[0]>>11) << 11;
			// bit     14: layer 3 priority
			pri_addr |= (pens[3]>>11) << 14;

			pri_addr &= 0x7fff;

			l   =   pri2layer[m_priority_ram[pri_addr] & 7];

			bitmap.pix16(y, x) = pens[l];
		}
	}

	return 0;
}
