// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Angelo Salese

#include "emu.h"
#include "tzbx15_sprites.h"
#include "screen.h"

#include <algorithm>
#include <limits>
#include <utility>

// TZB215 on Apache 3
// TZB315 on Round Up 5, Big Fight, Cycle Warriors
// differences, if any, unknown
// (does not appear to be CLUT size, even if that would have made sense.
//  Round Up 5 uses the smaller CLUT like Apache 3 yet is confirmed to be a TZB315)

DEFINE_DEVICE_TYPE(TZB215_SPRITES, tzb215_device, "tzb215_sprites", "Tatsumi TZB215 Rotating Sprites")
DEFINE_DEVICE_TYPE(TZB315_SPRITES, tzb315_device, "tzb315_sprites", "Tatsumi TZB315 Rotating Sprites")

tzbx15_device::tzbx15_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_gfx_interface(mconfig, *this)
	, m_palette_clut(*this, "palette_clut")
	, m_palette_base(*this, finder_base::DUMMY_TAG)
	, m_spriteram(*this, finder_base::DUMMY_TAG)
	, m_sprites_l_rom(*this, "sprites_l")
	, m_sprites_h_rom(*this, "sprites_h")
{
}

tzbx15_device::tzbx15_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 clut_size)
	: tzbx15_device(mconfig, type, tag, owner, clock)
{
	m_rom_clut_size = clut_size;
}

tzb215_device::tzb215_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, u32 clut_size)
	: tzbx15_device(mconfig, TZB215_SPRITES, tag, owner, clock, clut_size)
{
}

tzb215_device::tzb215_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: tzbx15_device(mconfig, TZB215_SPRITES, tag, owner, clock, 0)
{
}

tzb315_device::tzb315_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, u32 clut_size)
	: tzbx15_device(mconfig, TZB315_SPRITES, tag, owner, clock, clut_size)
{
}

tzb315_device::tzb315_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: tzbx15_device(mconfig, TZB315_SPRITES, tag, owner, clock, 0)
{
}




static const gfx_layout spritelayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ 8,12,0,4, 24,28,16,20 },
	{ STEP8(0,4*8) },
	32*8
};

GFXDECODE_MEMBER( tzbx15_device::gfxinfo )
	GFXDECODE_DEVICE("sprites_l", 0, spritelayout, 0, 256)
	GFXDECODE_DEVICE("sprites_h", 0, spritelayout, 0, 256)
GFXDECODE_END

void tzbx15_device::device_start()
{
	m_rom_clut_offset = memregion("sprites_l")->bytes() - m_rom_clut_size;

	m_shadow_pen_array = make_unique_clear<uint8_t[]>(m_rom_clut_size * 2);

	decode_gfx(gfxinfo);
	gfx(0)->set_colors(m_rom_clut_size / 8);
	gfx(1)->set_colors(m_rom_clut_size / 8);
}

void tzbx15_device::device_reset()
{
}

void tzbx15_device::device_add_mconfig(machine_config &config)
{
	// 4096 or 8192 arranged as series of CLUTs
	PALETTE(config, m_palette_clut).set_format(palette_device::xRGB_555, m_rom_clut_size * 2);
}


/*
    Sprite RAM itself uses an index into two ROM tables to actually draw the object.

    Sprite RAM format:

    Word 0: 0xf000 - ?
            0x0fff - Index into ROM sprite table
    Word 1: 0x8000 - X Flip
            0x4000 - Y Flip
            0x3000 - ?
            0x0ff8 - Color
            0x0007 - ?
    Word 2: 0xffff - X position
    Word 3: 0xffff - Y position
    Word 4: 0x01ff - Scale
    Word 5: 0x01ff - Rotation

    Sprite ROM table format, alternate lines come from each bank, with the
    very first line indicating control information:

    First bank:
    Byte 0: Y destination offset (in scanlines, unaffected by scale).
    Byte 1: Always 0?
    Byte 2: Number of source scanlines to render from (so unaffected by destination scale).
    Byte 3: Bit 0 is bit 8 of the scanline count (tall buildings in Apache 3 need more than 255 lines).
            Bit 7 repeats each line's tiles on towards the screen edge

    Other banks:
    Byte 0: Width of line in tiles (-1)
    Byte 1: X offset to start drawing line at (multipled by scale * 8)
    Bytes 2/3: Tile index to start fetching tiles from (increments per tile).

*/
template<class BitmapClass>
void tzbx15_device::draw_sprites_main(BitmapClass &bitmap, const rectangle &cliprect, int write_priority_only, int rambank)
{
	// Sprite data is double buffered
	for (int offs = rambank;offs < rambank + 0x800;offs += 6)
	{
		int y =         m_spriteram[offs+3];
		int x =         m_spriteram[offs+2];
		int scale =     m_spriteram[offs+4] & 0x1ff;
		int color =     m_spriteram[offs+1] >> 3 & 0x1ff;
		int flip_x =    m_spriteram[offs+1] & 0x8000;
		int flip_y =    m_spriteram[offs+1] & 0x4000;
		int rotate =    m_spriteram[offs+5] & 0x1ff;

		int index = m_spriteram[offs];

//      if (m_spriteram[offs+1]&0x7)
//          color=machine().rand()%0xff;

		/* End of sprite list marker */
		if (index == 0xffff || m_spriteram[offs + 4] == 0xffff) // todo
			return;

		if (index >= 0x4000)
			continue;

		draw_sprite(bitmap, cliprect, index, color, int16_t(x), int16_t(y),
				scale, rotate, flip_x, flip_y, write_priority_only);
	}
}

/*
    The rotation value is a signed 9-bit slope t (0x100 = 1.0). The chip maps the drawn (flipped
    and scaled) sprite through [1 -t; t 1] about its origin, which lands on screen position
    (x, y). That turns the sprite by atan(t) and enlarges it by sqrt(1 + t^2); flipping X
    negates the turn. Games compensate for the enlargement by reducing the scale.

    Sprites are drawn a scanline at a time: each screen pixel is traced back through the
    rotation to a pixel of the scaled sprite, then through the scaling to a pixel of the sprite
    ROM tiles. The rotation step advances by a fixed amount per pixel, so it is stepped in
    fixed point.
*/
template<class BitmapClass>
void tzbx15_device::draw_sprite(BitmapClass &bitmap, const rectangle &cliprect,
		int index, int color, int x, int y, int scale, int rotate, bool flip_x, bool flip_y,
		int write_priority_only)
{
	uint8_t const *const src1 = m_sprites_l_rom + (index * 4);
	uint8_t const *const src2 = m_sprites_h_rom + (index * 4);
	bool const repeat = BIT(src1[3], 7);

	int const top = src1[0] & 0xf8;
	int const lines = src1[2] | ((src1[3] & 0x01) << 8);
	int const rows = (lines - top + 7) / 8;
	if (!scale || rows <= 0)
		return;
	int const bottom = top + rows * 8;

	// One run of tiles per row of 8 lines, taken from alternate banks
	int run_left[64], run_right[64], run_base[64];
	int width = 0;
	for (int h = 0; h < rows; h++)
	{
		uint8_t const *const entry = (h & 1) ? (src1 + 4 + (h / 2) * 4) : (src2 + (h / 2) * 4);
		run_left[h] = entry[1] * 8;
		run_right[h] = run_left[h] + (entry[0] + 1) * 8;
		run_base[h] = (entry[2] | (entry[3] << 8)) * 2;
		width = std::max(width, run_right[h]);
	}

	// Slope t in 8 fractional bits, and 1 / (1 + t^2) in 24 fractional bits
	int const slope = util::sext(rotate, 9) * (flip_x ? -1 : 1);
	int64_t const shrink = (int64_t(1) << 40) / (65536 + slope * slope);

	// Screen position relative to the origin (16 fractional bits) -> position in the scaled
	// sprite (40 fractional bits), before flipping
	auto const unrotate = [slope, shrink] (int64_t px, int64_t py)
	{
		return std::make_pair((px + ((slope * py) >> 8)) * shrink, (py - ((slope * px) >> 8)) * shrink);
	};

	// Size of the scaled sprite. Scale 0x80 is 1:1
	int const scaled_top = (top * scale) >> 7;
	int const scaled_bottom = (bottom * scale + 127) >> 7;
	int const scaled_left = 0;
	int scaled_right = (width * scale + 127) >> 7;
	if (repeat)
	{
		// Repeated lines run on to the edge of the screen, so find how far that is in the sprite
		for (int cx : { cliprect.left(), cliprect.right() + 1 })
			for (int cy : { cliprect.top(), cliprect.bottom() + 1 })
			{
				int64_t const zx = unrotate(int64_t(cx - x) << 16, int64_t(cy - y) << 16).first;
				scaled_right = std::max(scaled_right, int(-((flip_x ? zx : -zx) >> 40)) + 1);
			}
	}

	// Screen bounds: rotate the corners of the scaled sprite (8 fractional bits)
	int64_t minx = std::numeric_limits<int64_t>::max(), maxx = std::numeric_limits<int64_t>::min();
	int64_t miny = std::numeric_limits<int64_t>::max(), maxy = std::numeric_limits<int64_t>::min();
	for (int corner = 0; corner < 4; corner++)
	{
		int const cx = (corner & 1) ? scaled_right : scaled_left;
		int const cy = (corner & 2) ? scaled_bottom : scaled_top;
		int const lx = flip_x ? -cx : cx;
		int const ly = flip_y ? -cy : cy;
		int64_t const rx = (int64_t(lx) << 8) - int64_t(slope) * ly;
		int64_t const ry = int64_t(slope) * lx + (int64_t(ly) << 8);
		minx = std::min(minx, rx); maxx = std::max(maxx, rx);
		miny = std::min(miny, ry); maxy = std::max(maxy, ry);
	}
	rectangle bounds(x + int(minx >> 8) - 1, x + int(-((-maxx) >> 8)) + 1,
			y + int(miny >> 8) - 1, y + int(-((-maxy) >> 8)) + 1);
	bounds &= cliprect;
	bounds &= bitmap.cliprect();
	if (bounds.empty())
		return;

	unsigned const palette = 16 * (color % gfx(0)->colors());
	pen_t const *const pens = &m_palette_clut->pen(palette);
	uint8_t const *const shadows = m_shadow_pen_array.get() + palette;

	// Moving one pixel along a scanline moves the same amount through the scaled sprite each time
	int64_t const step_x = int64_t(shrink) << 16;
	int64_t const step_y = -(int64_t(slope) << 8) * shrink;

	gfx_element *const tile_gfx[2] = { gfx(0), gfx(1) };

	for (int py = bounds.top(); py <= bounds.bottom(); py++)
	{
		// Pixel centres
		auto const [start_x, start_y] = unrotate(
				(int64_t(bounds.left() - x) << 16) + 0x8000,
				(int64_t(py - y) << 16) + 0x8000);
		int64_t sprite_x = start_x;
		int64_t sprite_y = start_y;
		auto *const dest = &bitmap.pix(py);
		int last_dist_y = -1, line = 0, h = 0;
		bool line_ok = false;

		for (int px = bounds.left(); px <= bounds.right(); px++, sprite_x += step_x, sprite_y += step_y)
		{
			// Distance from the origin in the scaled sprite, which grows away from it whichever way it is flipped
			int const dist_x = flip_x ? int(-(sprite_x >> 40) - 1) : int(sprite_x >> 40);
			int const dist_y = flip_y ? int(-(sprite_y >> 40) - 1) : int(sprite_y >> 40);
			if (dist_x < 0 || dist_y < 0)
				continue;

			// Back through the scaling to the tile pixel
			if (dist_y != last_dist_y)
			{
				last_dist_y = dist_y;
				line = (dist_y << 7) / scale;
				line_ok = line >= top && line < bottom;
				h = (line - top) >> 3;
			}
			if (!line_ok)
				continue;
			int col = (dist_x << 7) / scale;
			if (col < run_left[h])
				continue;
			if (col >= run_right[h])
			{
				if (!repeat)
					continue;
				col = run_left[h] + (col - run_left[h]) % (run_right[h] - run_left[h]);
			}

			int const tile = run_base[h] + ((col - run_left[h]) >> 3);
			gfx_element *const tiles = tile_gfx[tile & 1];
			uint8_t const pixel = tiles->get_data((tile >> 1) % tiles->elements())[(line & 7) * tiles->rowbytes() + (col & 7)];
			if (pixel)
			{
				// Only draw shadow pens if writing priority buffer
				if (write_priority_only)
					dest[px] = shadows[pixel];
				else if (!shadows[pixel])
					dest[px] = pens[pixel];
			}
		}
	}
}

void tzbx15_device::draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, int write_priority_only, int rambank)
{
	draw_sprites_main(bitmap, cliprect, write_priority_only, rambank);
}

void tzbx15_device::draw_sprites(bitmap_ind8& bitmap, const rectangle &cliprect, int write_priority_only, int rambank)
{
	draw_sprites_main(bitmap, cliprect, write_priority_only, rambank);
}

/*
 * Object palettes are build from a series of cluts stored in the object roms.
 *
 *  We update 'Mame palettes' from the clut here in order to simplify the
 *  draw routines.  We also note down any uses of the 'shadow' pen (index 255).
 */
void tzbx15_device::update_cluts()
{
	const int length = m_rom_clut_size * 2;
	const uint8_t* bank1 = m_sprites_l_rom + m_rom_clut_offset;
	const uint8_t* bank2 = m_sprites_h_rom + m_rom_clut_offset;

	for (int i = 0; i < length; i+=8)
	{
		m_palette_clut->set_pen_color(i + 0, m_palette_base->pen_color(bank1[1] + m_sprite_palette_base));
		m_shadow_pen_array[i+0]=(bank1[1] == 255);
		m_palette_clut->set_pen_color(i + 1, m_palette_base->pen_color(bank1[0] + m_sprite_palette_base));
		m_shadow_pen_array[i+1]=(bank1[0] == 255);
		m_palette_clut->set_pen_color(i + 2, m_palette_base->pen_color(bank1[3] + m_sprite_palette_base));
		m_shadow_pen_array[i+2]=(bank1[3] == 255);
		m_palette_clut->set_pen_color(i + 3, m_palette_base->pen_color(bank1[2] + m_sprite_palette_base));
		m_shadow_pen_array[i+3]=(bank1[2] == 255);

		m_palette_clut->set_pen_color(i + 4, m_palette_base->pen_color(bank2[1] + m_sprite_palette_base));
		m_shadow_pen_array[i+4]=(bank2[1] == 255);
		m_palette_clut->set_pen_color(i + 5, m_palette_base->pen_color(bank2[0] + m_sprite_palette_base));
		m_shadow_pen_array[i+5]=(bank2[0] == 255);
		m_palette_clut->set_pen_color(i + 6, m_palette_base->pen_color(bank2[3] + m_sprite_palette_base));
		m_shadow_pen_array[i+6]=(bank2[3] == 255);
		m_palette_clut->set_pen_color(i + 7, m_palette_base->pen_color(bank2[2] + m_sprite_palette_base));
		m_shadow_pen_array[i+7]=(bank2[2] == 255);

		bank1+=4;
		bank2+=4;
	}
}
