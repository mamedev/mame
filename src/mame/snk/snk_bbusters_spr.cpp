// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

    Like NeoGeo sprite scale Y line selection is from an external rom.

    For 16 high sprites scale data starts at 0x3800 (9 scale levels)
    For 32 high sprites scale data starts at 0x7000 (17 scale levels)
    For 64 high sprites scale data starts at 0xa000 (33 scale levels)
    For 128 pixel high sprites scale data starts 0xc000 (65 scale levels)

    0xe000 and up - possibly X scale data?  unconfirmed

    Sprites are also double buffered, and this seems to be performed
    by having two complete sprite chips that are toggled per frame, rather
    than just ram.  Beast Busters has 4 sprite chips as it has two sprite
    banks.

***************************************************************************/

#include "emu.h"
#include "snk_bbusters_spr.h"

DEFINE_DEVICE_TYPE(SNK_BBUSTERS_SPR, snk_bbusters_spr_device, "snk_bbusters_spr", "SNK Beast Busters Sprites")

snk_bbusters_spr_device::snk_bbusters_spr_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SNK_BBUSTERS_SPR, tag, owner, clock)
	, device_gfx_interface(mconfig, *this, gfxinfo)
	, m_scale_table(*this, finder_base::DUMMY_TAG)
	, m_spriteram(*this, finder_base::DUMMY_TAG)
{
}

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,4) },
	{
		STEP4(0,1), STEP4(4*4,1),
		STEP4(4*4*2*8,1), STEP4(4*4+4*4*2*8,1)
	},
	{ STEP8(0,4*4*2), STEP8(16*32,4*8) },
	128*8
};

GFXDECODE_MEMBER( snk_bbusters_spr_device::gfxinfo )
	GFXDECODE_DEVICE( DEVICE_SELF, 0, spritelayout, 0, 16*4 )
GFXDECODE_END


void snk_bbusters_spr_device::device_start()
{
	save_item(NAME(m_scale_line_count));
}

template <int Size>
inline int snk_bbusters_spr_device::adjust_spritecode(int dx, int dy, int code)
{
	if (dy & (0x10 << Size)) code += 2 << (Size * 2);
	if (dx & (0x10 << Size)) code += 1 << (Size * 2);
	return code;
}

inline const uint8_t *snk_bbusters_spr_device::get_source_ptr(gfx_element *tilegfx, uint32_t sprite, int dx, int dy, int block)
{
	int code = 0;

	/* Get a tile index from the x,y position in the block */
	switch (block)
	{
	case 0: /* 16 x 16 sprite */
		break;

	case 1: /* 32 x 32 block
	            0 1
	            2 3
	        */
		code = adjust_spritecode<0>(dx, dy, code); // 4x4
		break;

	case 2: /* 64 by 64 block
	            0  1    4  5
	            2  3    6  7

	            8  9    12 13
	            10 11   14 15
	        */
		code = adjust_spritecode<0>(dx, dy, code); // 4x4
		code = adjust_spritecode<1>(dx, dy, code); // 8x8
		break;

	case 3: /* 128 by 128 block */
		code = adjust_spritecode<0>(dx, dy, code); // 4x4
		code = adjust_spritecode<1>(dx, dy, code); // 8x8
		code = adjust_spritecode<2>(dx, dy, code); // 16x16
		break;
	}

	return tilegfx->get_data((sprite + code) % tilegfx->elements()) + ((dy & 0xf) * tilegfx->rowbytes());
}

void snk_bbusters_spr_device::draw_block(bitmap_ind16 &dest, const rectangle &cliprect, int x, int y, int size, int flipx, int flipy, uint32_t sprite, int color, int block)
{
	// TODO: respect cliprect

	gfx_element *tilegfx = gfx(0);
	pen_t const pen_base = tilegfx->colorbase() + tilegfx->granularity() * (color % tilegfx->colors());
	uint32_t const xinc = (m_scale_line_count * 0x10000) / size;
	int dy = y;
	int const ex = m_scale_line_count;

	while (m_scale_line_count)
	{
		if (dy >= 16 && dy < 240)
		{
			uint16_t *const destline = &dest.pix(dy);
			uint8_t srcline = *m_scale_table_ptr;
			const uint8_t *srcptr = nullptr;

			if (!flipy)
				srcline = size - srcline - 1;

			int x_index;
			if (flipx)
				x_index = (ex - 1) * 0x10000;
			else
				x_index = 0;

			for (int sx = 0; sx < size; sx++)
			{
				if ((sx & 0xf) == 0)
					srcptr = get_source_ptr(tilegfx, sprite, sx, srcline, block);

				uint8_t const pixel = *srcptr++;
				if (pixel != 0xf)
					destline[(x + (x_index >> 16)) & 0x1ff] = pen_base + pixel;

				if (flipx)
					x_index -= xinc;
				else
					x_index += xinc;
			}
		}

		dy++;
		m_scale_table_ptr--;
		m_scale_line_count--;
	}
}

void snk_bbusters_spr_device::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const uint16_t *sprram = m_spriteram->buffer();

	for (int offs = 0; offs < 0x800; offs += 4)
	{
		int sprite = sprram[offs + 1];
		int colour = sprram[offs + 0];

		// TODO: get rid of this abominable hack without breaking anything
		if ((colour == 0xf7 || colour == 0xffff || colour == 0x43f9) && (sprite == 0x3fff || sprite == 0xffff || sprite == 0x0001))
			continue; // sprite 1, color 0x43f9 is the dead sprite in the top-right of the screen in Mechanized Attack's High Score table.

		int16_t y = sprram[offs + 3];
		int x = sprram[offs + 2];
		if (x & 0x200) x = -(0x100 - (x & 0xff));
		if (y > 320 || y < -256) y &= 0x1ff; // fix for bbusters ending & "Zing!" attract-mode fullscreen zombie & Helicopter on the 3rd rotation of the attractmode sequence

		/*
		    sprram[0]:
		        0xf000: Colour
		        0x0800: FX
		        0x0400: FY?
		        0x0300: Block control
		        0x0080: ?
		        0x007f: scale

		    Scale varies according to block size.
		    Block type 0: 0x70 = no scale, 0x7f == half size - 16 pixel sprite
		    Block type 1: 0x60 = no scale, 0x6f == half size - 32 pixel sprite
		    Block type 2: 0x40 = no scale, 0x5f == half size - 64 pixel sprite
		    Block type 3: 0x00 = no scale, 0x3f == half size - 128 pixel sprite

		*/
		int const block = (colour >> 8) & 0x3;
		bool const fy = BIT(colour, 10);
		bool const fx = BIT(colour, 11);
		colour >>= 12;
		sprite &= 0x3fff;

		int scale;
		switch (block)
		{
		case 0:
			scale = sprram[offs + 0] & 0x7;
			m_scale_table_ptr = m_scale_table + 0x387f + (0x80 * scale);
			m_scale_line_count = 0x10 - scale;
			draw_block(bitmap, cliprect, x, y, 16, fx, fy, sprite, colour, block);
			break;
		case 1: /* 2 x 2 */
			scale = sprram[offs + 0] & 0xf;
			m_scale_table_ptr = m_scale_table + 0x707f + (0x80 * scale);
			m_scale_line_count = 0x20 - scale;
			draw_block(bitmap, cliprect, x, y, 32, fx, fy, sprite, colour, block);
			break;
		case 2: /* 64 by 64 block (2 x 2) x 2 */
			scale = sprram[offs + 0] & 0x1f;
			m_scale_table_ptr = m_scale_table + 0xa07f + (0x80 * scale);
			m_scale_line_count = 0x40 - scale;
			draw_block(bitmap, cliprect, x, y, 64, fx, fy, sprite, colour, block);
			break;
		case 3: /* 2 x 2 x 2 x 2 */
			scale = sprram[offs + 0] & 0x3f;
			m_scale_table_ptr = m_scale_table + 0xc07f + (0x80 * scale);
			m_scale_line_count = 0x80 - scale;
			draw_block(bitmap, cliprect, x, y, 128, fx, fy, sprite, colour, block);
			break;
		}
	}
}
