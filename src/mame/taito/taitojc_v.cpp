// license:LGPL-2.1+
// copyright-holders:Ville Linde, Angelo Salese, hap
/*************************************************************************

  Taito JC System

  functions to emulate the video hardware

*************************************************************************/

#include "emu.h"
#include "video/poly.h"
#include "taitojc.h"

static const gfx_layout char_layout =
{
	16,16,
	0x80,
	4,
	{ 0,1,2,3 },
	{ 24,28,16,20,8,12,0,4, 56,60,48,52,40,44,32,36 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,8*64,9*64,10*64,11*64,12*64,13*64,14*64,15*64 },
	16*64
};

TILE_GET_INFO_MEMBER(taitojc_state::get_tile_info)
{
	uint32_t const val = m_tile_ram[tile_index];
	int const color = (val >> 22) & 0xff;
	int const tile = (val >> 2) & 0x7f;
	tileinfo.set(0, tile, color, 0);
}

void taitojc_state::tile_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_tile_ram[offset]);
	m_tilemap->mark_tile_dirty(offset);
}

void taitojc_state::char_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_char_ram[offset]);
	m_gfxdecode->gfx(0)->mark_dirty(offset/32);
}

uint32_t taitojc_state::char_r(offs_t offset)
{
	return m_char_ram[offset];
}

// Object data format:
//
// 0x00:   xxxxxx-- -------- -------- --------   Height
// 0x00:   ------xx xxxxxxxx -------- --------   Y
// 0x00:   -------- -------- xxxxxx-- --------   Width
// 0x00:   -------- -------- ------xx xxxxxxxx   X
// 0x01:   ---xxxxx xx------ -------- --------   Palette
// 0x01:   -------- --x----- -------- --------   Priority (0 = below 3D, 1 = above 3D)
// 0x01:   -------- -------x -------- --------   Color depth (0) 4bpp / (1) 8bpp
// 0x01:   -------- -------- -xxxxxxx xxxxxxxx   VRAM data address

/*
    Object RAM is grouped in three different banks (0-0x400 / 0x400-0x800 / 0x800-0xc00),
    Initial 6 dwords aren't surely for object stuff (setting global object flags?)
    0xd00-0xdff seems to be a per-bank vregister. Usage of this is mostly unknown, the only
    clue we have so far is this config change in dendego:
    0x2000db3f 0x3f3f3f3f 0xfec00090 0x403f00ff 0xd20-0xd2f on Taito logo
    0x2000db3f 0x3f3f3f3f 0xff600090 0x207f00ff 0xd20-0xd2f on intro FMV

    dword 0 bits 14-15 looks up to the object RAM for the given bank. (it's mostly fixed to 0,
    1 and 2 for each bank). Then dwords 2 and 3 should presumably configure bank 1 to a bigger
    (doubled?) height and width and a different x/y start point.


    0xfc0-0xfff is global vregs. 0xfc4 bit 13 is used to swap between bank 0 and bank 1?
    It's unknown at current time how bank 2 should show up.

        fc0 00000000   always
        fc4 c01f0000   boot-up, testmode, sidebs always, sidebs2 always
            c0100000   landgear in-game, dendego2 in-game
            c0310000   dendego in-game
            c0312000   dendego intro 3d parts
            c031f000   dendego disclaimer screen (only for a few frames)

        fc4 11000000 00------ ----0000 00000000   always 0/1
            -------- --xx---- -------- --------   ?
            -------- ----xxxx -------- --------   one of these probably disables textlayer, unknown function otherwise
            -------- -------- xxxx---- --------   object bank related

        fc8 40000000   always
        fcc 00000000   always
        fd0 c0000000   always
        ...
        ffc c0000000   always

*/

void taitojc_state::draw_object(bitmap_ind16 &bitmap, const rectangle &cliprect, uint32_t w1, uint32_t w2, uint8_t bank_type)
{
	uint8_t const color_depth = BIT(w2, 16);
	uint8_t const mask_screen = BIT(w2, 17);

	uint32_t address = (w2 & 0x7fff) << 5;
	if (BIT(w2, 14))
		address |= 0x40000;

	int x = util::sext(w1, 10);
	int y = util::sext(w1 >> 16, 10);

	int width         = ((w1 >> 10) & 0x3f) << 4;
	int height        = ((w1 >> 26) & 0x3f) << 4;
	int const palette = ((w2 >> 22) & 0x7f) << 8;

	/* TODO: untangle this! */
	uint32_t const *v;
	if (address >= 0xff000)
		v = &m_objlist[(address - 0xff000) / 4];
	else if (address >= 0xfc000)
		v = &m_char_ram[(address - 0xfc000) / 4];
	else if (address >= 0xf8000)
		v = &m_tile_ram[(address - 0xf8000) / 4];
	else
		v = &m_vram[address / 4];
	auto const v8 = util::big_endian_cast<u8>(v);

	/* guess, but it's probably doable via a vreg ... */
	if ((width == 0 || height == 0) && bank_type == 2)
		width = height = 16;

	if (width == 0 || height == 0)
		return;

	int x1 = x;
	int x2 = x + width;
	int y1 = y;
	int y2 = y + height;

	// trivial rejection
	if (x1 > cliprect.max_x || x2 < cliprect.min_x || y1 > cliprect.max_y || y2 < cliprect.min_y)
	{
		return;
	}

//  osd_printf_debug("draw_object: %08X %08X, X: %d, Y: %d, W: %d, H: %d\n", w1, w2, x, y, width, height);

	int ix = 0;
	int iy = 0;

	// clip
	if (x1 < cliprect.min_x)
	{
		ix = abs(cliprect.min_x - x1);
		x1 = cliprect.min_x;
	}
	if (x2 > cliprect.max_x)
	{
		x2 = cliprect.max_x;
	}
	if (y1 < cliprect.min_y)
	{
		iy = abs(cliprect.min_y - y1);
		y1 = cliprect.min_y;
	}
	if (y2 > cliprect.max_y)
	{
		y2 = cliprect.max_y;
	}

	/* this bit seems to set up border at left/right of screen (reads at 0xffc00) */
	if (mask_screen)
	{
		if (address != 0xffc00)
		{
			popmessage("mask screen with %08x, contact MAMEdev",address);
			return;
		}

		for (int j = y1; j < y2; j++)
		{
			uint16_t *const d = &bitmap.pix(j);

			for (int i = x1; i < x2; i++)
			{
				d[i] = 0x78; //TODO: black

				//index++;
			}

			//iy++;
		}
	}
	else if (!color_depth) // Densha de Go 2/2X "credit text", 4bpp
	{
		for (int j = y1; j < y2; j++)
		{
			uint16_t *const d = &bitmap.pix(j);
			int index = (iy * (width / 2)) + ix;

			for (int i = x1; i < x2; i += 2)
			{
				uint8_t pen = (v8[index] & 0xf0) >> 4;
				if (pen != 0)
					d[i] = palette + pen;

				pen = v8[index] & 0x0f;
				if (pen != 0)
					d[i + 1] = palette + pen;

				index++;
			}

			iy++;
		}
	}
	else // 8bpp
	{
		{
			for (int j = y1; j < y2; j++)
			{
				uint16_t *const d = &bitmap.pix(j);
				int index = (iy * width) + ix;

				for (int i = x1; i < x2; i++)
				{
					uint8_t const pen = v8[index];
					if (pen != 0)
					{
						d[i] = palette + pen;
					}

					index++;
				}

				iy++;
			}
		}
	}
}

void taitojc_state::draw_object_bank(bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t bank_type, uint8_t pri)
{
	uint16_t const start_offs = ((bank_type + 1) * 0x400) / 4;
//  uint8_t const double_xy = BIT(m_objlist[(0xd1c + bank_type * 0x10) / 4], 29);

	/* probably a core bug in there (otherwise objects sticks on screen in Densha de Go) */
	if ((bank_type == 1) && BIT(~m_objlist[0xfc4/4], 13))
		return;

	for (int i = start_offs - 2; i >= (start_offs - 0x400 / 4); i -= 2)
	{
		uint32_t const w1 = m_objlist[i + 0];
		uint32_t const w2 = m_objlist[i + 1];

		if (BIT(w2, 21) == pri)
		{
			draw_object(bitmap, cliprect, w1, w2, bank_type);
		}
	}
}


void taitojc_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(taitojc_state::get_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);

	m_tilemap->set_transparent_pen(0);

	/* create the char set (gfx will then be updated dynamically from RAM) */
	m_gfxdecode->set_gfx(0, std::make_unique<gfx_element>(m_palette, char_layout, (uint8_t *)&m_char_ram[0], 0, m_palette->entries() / 16, 0));
}

uint32_t taitojc_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	// low priority objects
	draw_object_bank(bitmap, cliprect, 0, 0);
	draw_object_bank(bitmap, cliprect, 1, 0);
	draw_object_bank(bitmap, cliprect, 2, 0);

	// 3D layer
	m_tc0780fpa->draw(bitmap, cliprect);

	// high priority objects
	draw_object_bank(bitmap, cliprect, 0, 1);
	draw_object_bank(bitmap, cliprect, 1, 1);
	draw_object_bank(bitmap, cliprect, 2, 1);

	// text layer
	if (BIT(m_objlist[0xfc4/4], 16))
		m_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

uint32_t dendego_state::screen_update_dendego(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// update controller state in artwork
	static const uint8_t mascon_table[6] = { 0x76, 0x67, 0x75, 0x57, 0x73, 0x37 };
	static const uint8_t brake_table[11] = { 0x00, 0x05, 0x1d, 0x35, 0x4d, 0x65, 0x7d, 0x95, 0xad, 0xc5, 0xd4 };

	uint8_t btn = (m_io_buttons->read() & 0x77);
	int level;
	for (level = 5; level > 0; level--)
		if (btn == mascon_table[level]) break;

	if (level != m_counters[0])
		m_counters[0] = level;

	btn = m_analog_ports[0]->read() & 0xff;
	for (level = 10; level > 0; level--)
		if (btn >= brake_table[level]) break;

	if (level != m_counters[1])
		m_counters[1] = level;

	return screen_update(screen, bitmap, cliprect);
}
