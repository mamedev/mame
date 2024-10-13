// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "xavix.h"

#include <cmath>

// #define VERBOSE 1
#include "logmacro.h"


inline void xavix_state::set_data_address(int address, int bit)
{
	m_tmp_dataaddress = address;
	m_tmp_databit = bit;
}

inline uint8_t xavix_state::get_next_bit()
{
	// going through memory is slow, try not to do it too often!
	if (m_tmp_databit == 0)
	{
		//m_bit = m_maincpu->read_full_data_sp(m_tmp_dataaddress);
		m_bit = read_full_data_sp_bypass(m_tmp_dataaddress);
	}

	uint8_t ret = m_bit >> m_tmp_databit;
	ret &= 1;

	m_tmp_databit++;

	if (m_tmp_databit == 8)
	{
		m_tmp_databit = 0;
		m_tmp_dataaddress++;
	}

	return ret;
}

inline uint8_t xavix_state::get_next_byte()
{
	uint8_t dat = 0;
	for (int i = 0; i < 8; i++)
	{
		dat |= (get_next_bit() << i);
	}
	return dat;
}

inline int xavix_state::get_current_address_byte()
{
	return m_tmp_dataaddress;
}


void xavix_state::video_start()
{
	m_screen->register_screen_bitmap(m_zbuffer);
}


void xavix_state::palram_sh_w(offs_t offset, uint8_t data)
{
	m_palram_sh[offset] = data;
	update_pen(offset, m_palram_sh[offset], m_palram_l[offset]);
}

void xavix_state::palram_l_w(offs_t offset, uint8_t data)
{
	m_palram_l[offset] = data;
	update_pen(offset, m_palram_sh[offset], m_palram_l[offset]);
}

void xavix_state::bmp_palram_sh_w(offs_t offset, uint8_t data)
{
	m_bmp_palram_sh[offset] = data;
	update_pen(offset+256, m_bmp_palram_sh[offset], m_bmp_palram_l[offset]);
}

void xavix_state::bmp_palram_l_w(offs_t offset, uint8_t data)
{
	m_bmp_palram_l[offset] = data;
	update_pen(offset+256, m_bmp_palram_sh[offset], m_bmp_palram_l[offset]);
}


void xavix_state::spriteram_w(offs_t offset, uint8_t data)
{
	if (offset < 0x100)
	{
		m_fragment_sprite[offset] = data;
		m_fragment_sprite[offset + 0x400] = data & 0x01;
	}
	else if (offset < 0x400)
	{
		m_fragment_sprite[offset] = data;
	}
	else if (offset < 0x500)
	{
		m_fragment_sprite[offset] = data & 1;
		m_fragment_sprite[offset - 0x400] = (m_fragment_sprite[offset - 0x400] & 0xfe) | (data & 0x01);
		m_sprite_xhigh_ignore_hack = false; // still doesn't help monster truck test mode case, which writes here, but still expects values to be ignored
	}
	else
	{
		m_fragment_sprite[offset] = data;
	}
}

void xavix_state::update_pen(int pen, uint8_t shval, uint8_t lval)
{
	uint16_t dat;
	dat = shval;
	dat |= lval << 8;

	int y_raw = (dat & 0x1f00) >> 8;
	int c_raw = (dat & 0x00e0) >> 5;
	int h_raw = (dat & 0x001f) >> 0;

	// The dividers may be dynamic
	double y = y_raw / 20.0;
	double c = c_raw /  5.0;

	// These weights may be dynamic too.  They're standard NTSC values, would they change on PAL?
	const double wr = 0.299;
	const double wg = 0.587;
	const double wb = 0.114;

	// Table of hues
	// Values 24+ are transparent

	const double hues[32][3] = {
		{ 1.00, 0.00, 0.00 },
		{ 1.00, 0.25, 0.00 },
		{ 1.00, 0.50, 0.00 },
		{ 1.00, 0.75, 0.00 },
		{ 1.00, 1.00, 0.00 },
		{ 0.75, 1.00, 0.00 },
		{ 0.50, 1.00, 0.00 },
		{ 0.25, 1.00, 0.00 },
		{ 0.00, 1.00, 0.00 },
		{ 0.00, 1.00, 0.25 },
		{ 0.00, 1.00, 0.50 },
		{ 0.00, 1.00, 0.75 },
		{ 0.00, 1.00, 1.00 },
		{ 0.00, 0.75, 1.00 },
		{ 0.00, 0.50, 1.00 },
		{ 0.00, 0.25, 1.00 },
		{ 0.00, 0.00, 1.00 },
		{ 0.25, 0.00, 1.00 },
		{ 0.50, 0.00, 1.00 },
		{ 0.75, 0.00, 1.00 },
		{ 1.00, 0.00, 1.00 },
		{ 1.00, 0.00, 0.75 },
		{ 1.00, 0.00, 0.50 },
		{ 1.00, 0.00, 0.25 },

		{ 0   , 0   , 0    },
		{ 0   , 0   , 0    },
		{ 0   , 0   , 0    },
		{ 0   , 0   , 0    },
		{ 0   , 0   , 0    },
		{ 0   , 0   , 0    },
		{ 0   , 0   , 0    },
		{ 0   , 0   , 0    },
	};

	double r0 = hues[h_raw][0];
	double g0 = hues[h_raw][1];
	double b0 = hues[h_raw][2];

	double z = wr * r0 + wg * g0 + wb * b0;

	if(y < z)
		c *= y/z;
	else if(z < 1)
		c *= (1-y) / (1-z);

	double r1 = (r0 - z) * c + y;
	double g1 = (g0 - z) * c + y;
	double b1 = (b0 - z) * c + y;


	// lower overall brightness slightly, or some palette entries in tak_gin end up washed out / with identical colours, losing details
	// the darkest colours in certain flags still look wrong however, and appear nearly black
	// this might suggest the overall palette conversion needs work
	r1 = r1 * 0.92f;
	g1 = g1 * 0.92f;
	b1 = b1 * 0.92f;

	if(r1 < 0)
		r1 = 0;
	else if(r1 > 1)
		r1 = 1.0;

	if(g1 < 0)
		g1 = 0;
	else if(g1 > 1)
		g1 = 1.0;

	if(b1 < 0)
		b1 = 0;
	else if(b1 > 1)
		b1 = 1.0;

	int r_real = r1 * 255.0f;
	int g_real = g1 * 255.0f;
	int b_real = b1 * 255.0f;

	m_palette->set_pen_color(pen, r_real, g_real, b_real);

	m_screen->update_partial(m_screen->vpos());
}



void xavix_state::draw_tilemap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int which)
{
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		draw_tilemap_line(screen, bitmap, cliprect, which, y);
	}
}

void xavix_state::decode_inline_header(int &flipx, int &flipy, int &test, int &pal, int debug_packets)
{
	uint8_t byte1 = 0;
	int done = 0;

	flipx = 0;
	flipy = 0;
	test = 0;

	int first = 1;

	do
	{
		byte1 = get_next_byte();

		// only the first byte matters when it comes to setting palette / flips, the rest are just ignored until we reach a 0x6 command, after which there is the tile data
		if (first == 1)
		{
			pal = (byte1 & 0xf0) >> 4;
			int cmd = (byte1 & 0x0f);

			switch (cmd)
			{
			// these cases haven't been seen
			case 0x0:
			case 0x2:
			case 0x4:
			case 0x8:
			case 0xa:
			case 0xc:
			case 0xe:

			// this is just the end command, changes nothing, can be pointed at directly tho
			case 0x6:
				break;

			// flip cases
			// does bit 0x02 have a meaning here, we have 2 values for each case

			case 0x1:
			case 0x3:
				flipx = 0; flipy = 0;
				break;

			case 0x5:
			case 0x7:
				flipx = 1; flipy = 0;
				break;

			case 0x9:
			case 0xb:
				flipx = 0; flipy = 1;
				break;

			case 0xd:
			case 0xf:
				flipx = 1; flipy = 1;
				break;
			}

			first = 0;
		}

		if ((byte1 & 0x0f) == 0x06)
		{
			// tile data will follow after this, always?
			done = 1;
			//if (debug_packets) LOG(" (setting palette)");
		}
	} while (done == 0);
	//if (debug_packets) LOG("\n");
}

void xavix_state::draw_tilemap_line(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int which, int line)
{
	uint8_t* tileregs;
	if (which == 0)
	{
		tileregs = m_tmap1_regs;
	}
	else
	{
		tileregs = m_tmap2_regs;
	}

	// bail if tilemap is disabled
	if (!(tileregs[0x7] & 0x80))
		return;

	int alt_tileaddressing = 0;
	int alt_tileaddressing2 = 0;

	int ydimension = 0;
	int xdimension = 0;
	int ytilesize = 0;
	int xtilesize = 0;
	int yshift = 0;

	switch (tileregs[0x3] & 0x30)
	{
	case 0x00:
		ydimension = 32;
		xdimension = 32;
		ytilesize = 8;
		xtilesize = 8;
		yshift = 3;
		break;

	case 0x10:
		ydimension = 32;
		xdimension = 16;
		ytilesize = 8;
		xtilesize = 16;
		yshift = 3;
		break;

	case 0x20: // guess
		ydimension = 16;
		xdimension = 32;
		ytilesize = 16;
		xtilesize = 8;
		yshift = 4;
		break;

	case 0x30:
		ydimension = 16;
		xdimension = 16;
		ytilesize = 16;
		xtilesize = 16;
		yshift = 4;
		break;
	}

	if (tileregs[0x7] & 0x10)
		alt_tileaddressing = 1;
	else
		alt_tileaddressing = 0;

	if (tileregs[0x7] & 0x02)
		alt_tileaddressing2 = 1;

	if ((tileregs[0x7] & 0x7f) == 0x04)
		alt_tileaddressing2 = 2;

	//LOG("draw tilemap %d, regs base0 %02x base1 %02x base2 %02x tilesize,bpp %02x scrollx %02x scrolly %02x pal %02x mode %02x\n", which, tileregs[0x0], tileregs[0x1], tileregs[0x2], tileregs[0x3], tileregs[0x4], tileregs[0x5], tileregs[0x6], tileregs[0x7]);

	// there's a tilemap register to specify base in main ram, although in the monster truck test mode it points to an unmapped region
	// and expected a fixed layout, we handle that in the memory map at the moment

	int drawline = line;
	int scrolly = tileregs[0x5];

	drawline += scrolly;

	drawline &= ((ydimension * ytilesize) - 1);

	int y = drawline >> yshift;
	int yyline = drawline & (ytilesize - 1);

	for (int x = 0; x < xdimension; x++)
	{
		int count = (y * xdimension) + x;

		int tile = 0;

		// the register being 0 probably isn't the condition here
		if (tileregs[0x0] != 0x00)
		{
			//tile |= m_maincpu->read_full_data_sp((tileregs[0x0] << 8) + count);
			tile |= read_full_data_sp_bypass((tileregs[0x0] << 8) + count);
		}

		// only read the next byte if we're not in an 8-bit mode
		if (((tileregs[0x7] & 0x7f) != 0x00) && ((tileregs[0x7] & 0x7f) != 0x08))
		{
			//tile |= m_maincpu->read_full_data_sp((tileregs[0x1] << 8) + count) << 8;
			tile |= read_full_data_sp_bypass((tileregs[0x1] << 8) + count) << 8;
		}

		// 24 bit modes can use reg 0x2, otherwise it gets used as extra attribute in other modes
		if (alt_tileaddressing2 == 2)
		{
			//tile |= m_maincpu->read_full_data_sp((tileregs[0x2] << 8) + count) << 16;
			tile |= read_full_data_sp_bypass((tileregs[0x2] << 8) + count) << 16;
		}


		int bpp = (tileregs[0x3] & 0x0e) >> 1;
		bpp++;
		int pal = (tileregs[0x6] & 0xf0) >> 4;
		int zval = (tileregs[0x6] & 0x0f) >> 0;
		int scrollx = tileregs[0x4];

		int basereg;
		int flipx = (tileregs[0x03]&0x40)>>6;
		int flipy = (tileregs[0x03]&0x80)>>7;
		int gfxbase;

		// tile 0 is always skipped, doesn't even point to valid data packets in alt mode
		// this is consistent with the top layer too
		// should we draw as solid in solid layer?
		if (tile == 0)
		{
			continue;
		}

		int debug_packets = 1;
		//if (line==128) debug_packets = 1;
		//else debug_packets = 0;

		int test = 0;

		if (!alt_tileaddressing)
		{
			if (alt_tileaddressing2 == 0)
			{
				// Tile Based Addressing takes into account Tile Sizes and bpp
				const int offset_multiplier = (ytilesize * xtilesize) / 8;

				basereg = 0;
				gfxbase = (m_segment_regs[(basereg * 2) + 1] << 16) | (m_segment_regs[(basereg * 2)] << 8);

				tile = tile * (offset_multiplier * bpp);
				tile += gfxbase;
			}
			else if (alt_tileaddressing2 == 1)
			{
				// 8-byte alignment Addressing Mode uses a fixed offset? (like sprites)
				tile = tile * 8;
				basereg = (tile & 0x70000) >> 16;
				tile &= 0xffff;
				gfxbase = (m_segment_regs[(basereg * 2) + 1] << 16) | (m_segment_regs[(basereg * 2)] << 8);
				tile += gfxbase;
			}

			// Tilemap specific mode extension with an 8-bit per tile attribute, works in all modes except 24-bit (no room for attribute) and header (not needed?)
			if (tileregs[0x7] & 0x08)
			{
				//uint8_t extraattr = m_maincpu->read_full_data_sp((tileregs[0x2] << 8) + count);
				uint8_t extraattr = read_full_data_sp_bypass((tileregs[0x2] << 8) + count);

				// make use of the extraattr stuff?
				pal = (extraattr & 0xf0) >> 4;
				zval = (extraattr & 0x0f) >> 0;
			}
		}
		else
		{
			// Addressing Mode 2 (plus Inline Header)

			//if (debug_packets) LOG("for tile %04x (at %d %d): ", tile, (((x * 16) + scrollx) & 0xff), (((y * 16) + scrolly) & 0xff));

			basereg = (tile & 0xf000) >> 12;
			tile &= 0x0fff;
			gfxbase = (m_segment_regs[(basereg * 2) + 1] << 16) | (m_segment_regs[(basereg * 2)] << 8);

			tile += gfxbase;
			set_data_address(tile, 0);

			decode_inline_header(flipx, flipy, test, pal, debug_packets);

			tile = get_current_address_byte();
		}

		if (test == 1)
		{
			pal = machine().rand() & 0xf;
		}

		draw_tile_line(screen, bitmap, cliprect, tile, bpp, (x * xtilesize) + scrollx, line, ytilesize, xtilesize, flipx, flipy, pal, zval, yyline);
		draw_tile_line(screen, bitmap, cliprect, tile, bpp, ((x * xtilesize) + scrollx) - 256, line, ytilesize, xtilesize, flipx, flipy, pal, zval, yyline); // wrap-x
	}
}

void xavix_state::draw_sprites(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		draw_sprites_line(screen, bitmap, cliprect, y);
	}
}

void xavix_state::draw_sprites_line(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int line)
{
	int alt_addressing = 0;

	if ((m_spritereg == 0x00) || (m_spritereg == 0x01))
	{
		// 8-bit addressing  (Tile Number)
		// 16-bit addressing (Tile Number) (rad_rh)
		alt_addressing = 1;
	}
	else if (m_spritereg == 0x02)
	{
		// 16-bit addressing (8-byte alignment Addressing Mode)
		alt_addressing = 2;
	}
	else if (m_spritereg == 0x04)
	{
		// 24-bit addressing (Addressing Mode 2)
		alt_addressing = 0;
	}
	else
	{
		popmessage("unknown sprite reg %02x", m_spritereg);
	}


	//LOG("frame\n");
	// priority doesn't seem to be based on list order, there are bad sprite-sprite priorities with either forward or reverse

	for (int i = 0xff; i >= 0; i--)
	{

		uint8_t* spr_attr0 = m_fragment_sprite + 0x000;
		uint8_t* spr_attr1 = m_fragment_sprite + 0x100;
		uint8_t* spr_ypos = m_fragment_sprite + 0x200;
		uint8_t* spr_xpos = m_fragment_sprite + 0x300;
		uint8_t* spr_xposh = m_fragment_sprite + 0x400;
		uint8_t* spr_addr_lo = m_fragment_sprite + 0x500;
		uint8_t* spr_addr_md = m_fragment_sprite + 0x600;
		uint8_t* spr_addr_hi = m_fragment_sprite + 0x700;


		/* attribute 0 bits
		   pppp bbb-    p = palette, b = bpp

		   attribute 1 bits
		   zzzz ssFf    s = size, F = flipy f = flipx
		*/

		int ypos = spr_ypos[i];
		int xpos = spr_xpos[i];
		int tile = 0;

		// high 8-bits only used in 24-bit mode
		if (((m_spritereg & 0x7f) == 0x04) || ((m_spritereg & 0x7f) == 0x15))
			tile |= (spr_addr_hi[i] << 16);

		// mid 8-bits used in everything except 8-bit mode
		if ((m_spritereg & 0x7f) != 0x00)
			tile |= (spr_addr_md[i] << 8);

		// low 8-bits always read
		tile |= spr_addr_lo[i];

		int attr0 = spr_attr0[i];
		int attr1 = spr_attr1[i];

		int pal = (attr0 & 0xf0) >> 4;

		int zval = (attr1 & 0xf0) >> 4;
		int flipx = (attr1 & 0x01);
		int flipy = (attr1 & 0x02);

		int drawheight = 16;
		int drawwidth = 16;

		int xpos_adjust = 0;
		int ypos_adjust = -16;

		// taito nost attr1 is 84 / 80 / 88 / 8c for the various elements of the xavix logo.  monster truck uses ec / fc / dc / 4c / 5c / 6c (final 6 sprites ingame are 00 00 f0 f0 f0 f0, radar?)

		drawheight = 8;
		drawwidth = 8;

		if (attr1 & 0x04) drawwidth = 16;
		if (attr1 & 0x08) drawheight = 16;

		xpos_adjust = -(drawwidth/2);
		ypos_adjust = -(drawheight/2);

		ypos ^= 0xff;

		if (ypos & 0x80)
		{
			ypos = -0x80 + (ypos & 0x7f);
		}
		else
		{
			ypos &= 0x7f;
		}

		ypos += 128 + 1;

		ypos += ypos_adjust;

		int spritelowy = ypos;
		int spritehighy = ypos + drawheight;

		if ((line >= spritelowy) && (line < spritehighy))
		{
			int drawline = line - spritelowy;


			/* coordinates are signed, based on screen position 0,0 being at the center of the screen
			   tile addressing likewise, point 0,0 is center of tile?
			   this makes the calculation a bit more annoying in terms of knowing when to apply offsets, when to wrap etc.
			   this is likely still incorrect

			   -- NOTE! HACK!

			   Use of additional x-bit is very confusing rad_snow, taitons1 (ingame) etc. clearly need to use it
			   but the taitons1 xavix logo doesn't even initialize the RAM for it and behavior conflicts with ingame?
			   maybe only works with certain tile sizes?

			   some code even suggests this should be bit 0 of attr0, but it never gets set there
			   (I'm mirroring the bits in the write handler at the moment)

			   there must be a register somewhere (or a side-effect of another mode) that enables / disables this
			   behavior, as we need to make use of xposh for the left side in cases that need it, but that
			   completely breaks the games that never set it at all (monster truck, xavix logo on taitons1)

			   monster truck hidden service mode ends up writing to the RAM, breaking the 'clock' display if
			   we use the values for anything.. again suggesting there must be a way to ignore it entirely?

			 */

			int xposh = spr_xposh[i] & 1;

			if (xpos & 0x80) // left side of center
			{
				xpos &= 0x7f;
				xpos = -0x80 + xpos;

				if (!m_sprite_xhigh_ignore_hack)
					if (!xposh)
						xpos -= 0x80;

			}
			else // right side of center
			{
				xpos &= 0x7f;

				if (!m_sprite_xhigh_ignore_hack)
					if (xposh)
						xpos += 0x80;
			}

			xpos += 128;

			xpos += xpos_adjust;

			// galplus phalanx beam (sprite wraparound)
			if (xpos<-0x80)
				xpos += 256+128;

			int bpp = 1;

			bpp = (attr0 & 0x0e) >> 1;
			bpp += 1;

			// Everything except directdirect addressing (Addressing Mode 2) goes through the segment registers?
			if (alt_addressing != 0)
			{
				int basereg = 0;

				// tile based addressing takes into account tile size (and bpp?)
				if (alt_addressing == 1)
				{
					tile = (tile * drawheight * drawwidth * bpp) / 8;
					basereg = 0; // always uses segment register 0 in tile addressing mode?
				}
				else
				{
					// 8-byte alignment Addressing Mode uses a fixed offset?
					if (alt_addressing == 2)
						tile = tile * 8;

					basereg = (tile & 0xf0000) >> 16;
					tile &= 0xffff;
				}

				int gfxbase = (m_segment_regs[(basereg * 2) + 1] << 16) | (m_segment_regs[(basereg * 2)] << 8);
				tile += gfxbase;
			}

			draw_tile_line(screen, bitmap, cliprect, tile, bpp, xpos , line, drawheight, drawwidth, flipx, flipy, pal, zval, drawline);

			/*
			if ((spr_ypos[i] != 0x81) && (spr_ypos[i] != 0x80) && (spr_ypos[i] != 0x00))
			{
			    LOG("sprite with enable? %02x attr0 %02x attr1 %02x attr3 %02x attr5 %02x attr6 %02x attr7 %02x\n", spr_ypos[i], spr_attr0[i], spr_attr1[i], spr_xpos[i], spr_addr_lo[i], spr_addr_md[i], spr_addr_hi[i] );
			}
			*/
		}
	}
}

void xavix_state::draw_tile_line(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int tile, int bpp, int xpos, int ypos, int drawheight, int drawwidth, int flipx, int flipy, int pal, int zval, int line)
{
	const pen_t *paldata = m_palette->pens();
	if (ypos > cliprect.max_y || ypos < cliprect.min_y)
		return;

	if ((xpos > cliprect.max_x) || ((xpos + drawwidth) < cliprect.min_x))
		return;

	if ((ypos >= cliprect.min_y && ypos <= cliprect.max_y))
	{
		// if bpp>4 then ignore unaligned palette selects bits based on bpp
		// ttv_lotr uses 5bpp graphics (so 32 colour alignment) but sets palette 0xf (a 16 colour boundary) when it expects palette 0xe
		if (bpp>4)
			pal &= (0xf<<(bpp-4));

		int bits_per_tileline = drawwidth * bpp;

		// set the address here so we can increment in bits in the draw function
		set_data_address(tile, 0);

		if (flipy)
			line = (drawheight - 1) - line;

		m_tmp_dataaddress = m_tmp_dataaddress + ((line * bits_per_tileline) / 8);
		m_tmp_databit = (line * bits_per_tileline) % 8;

		for (int x = 0; x < drawwidth; x++)
		{
			int col;

			if (flipx)
			{
				col = xpos + (drawwidth - 1) - x;
			}
			else
			{
				col = xpos + x;
			}

			uint8_t dat = 0;

			for (int i = 0; i < bpp; i++)
			{
				dat |= (get_next_bit() << i);
			}

			if ((col >= cliprect.min_x && col <= cliprect.max_x))
			{
				uint16_t *const zyposptr = &m_zbuffer.pix(ypos);

				if (zval >= zyposptr[col])
				{
					int pen = (dat + (pal << 4)) & 0xff;

					if ((m_palram_sh[pen] & 0x1f) < 24) // hue values 24-31 are transparent
					{
						uint32_t *const yposptr = &bitmap.pix(ypos);
						yposptr[col] = paldata[pen];

						zyposptr[col] = zval;
					}
				}
			}
		}
	}
}

uint32_t xavix_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *paldata = m_palette->pens();
	// not sure what you end up with if you fall through all layers as transparent, so far no issues noticed
	bitmap.fill(m_palette->black_pen(), cliprect);
	m_zbuffer.fill(0, cliprect);

	rectangle clip = cliprect;

	clip.min_y = cliprect.min_y;
	clip.max_y = cliprect.max_y;
	clip.min_x = cliprect.min_x;
	clip.max_x = cliprect.max_x;

	if (m_arena_control & 0x01)
	{
		/* Controls the clipping area (for all layers?) used for effect at start of Slap Fight and to add black borders in other cases
		   based on Slap Fight Tiger lives display (and reference videos) this is slightly offset as all status bar gfx must display
		   Monster Truck black bars are wider on the right hand side, but this matches with the area in which the tilemap is incorrectly rendered so seems to be intentional
		   Snowboard hides garbage sprites on the right hand side with this, confirming the right hand side offset
		   Taito Nostalgia 1 'Gladiator' portraits in demo mode are cut slightly due to the area specified, again the cut-off points for left and right are confirmed as correct on hardware

		   some games enable it with both regs as 00, which causes a problem, likewise ping pong sets both to 0xff
		   but Slap Fight Tiger has both set to 0x82 at a time when everything should be clipped
		*/
		if (((m_arena_start != 0x00) && (m_arena_end != 0x00)) && ((m_arena_start != 0xff) && (m_arena_end != 0xff)))
		{
			clip.max_x = m_arena_start - 3; // must be -3 to hide garbage on the right hand side of snowboarder
			clip.min_x = m_arena_end - 2; // must be -2 to render a single pixel line of the left border on Mappy remix (verified to render), although this creates a single pixel gap on the left of snowboarder status bar (need to verify)

			if (clip.min_x < cliprect.min_x)
				clip.min_x = cliprect.min_x;

			if (clip.max_x > cliprect.max_x)
				clip.max_x = cliprect.max_x;
		}
	}
	bitmap.fill(paldata[0], clip);

	draw_tilemap(screen, bitmap, clip, 1);
	draw_tilemap(screen, bitmap, clip, 0);
	draw_sprites(screen, bitmap, clip);

	//popmessage("%02x %02x %02x %02x   %02x %02x %02x %02x   %02x %02x %02x %02x   %02x %02x %02x %02x", m_soundregs[0],m_soundregs[1],m_soundregs[2],m_soundregs[3],m_soundregs[4],m_soundregs[5],m_soundregs[6],m_soundregs[7],m_soundregs[8],m_soundregs[9],m_soundregs[10],m_soundregs[11],m_soundregs[12],m_soundregs[13],m_soundregs[14],m_soundregs[15]);

	// temp, needs priority, transparency etc. also it's far bigger than the screen, I guess it must get scaled?!
	if (m_bmp_base)
	{
		// looks like it can zoom the bitmap using these?
		uint16_t top = ((m_bmp_base[0x01] << 8) | m_bmp_base[0x00]);
		uint16_t bot = ((m_bmp_base[0x03] << 8) | m_bmp_base[0x02]);
		uint16_t lft = ((m_bmp_base[0x05] << 8) | m_bmp_base[0x04]);
		uint16_t rgt = ((m_bmp_base[0x07] << 8) | m_bmp_base[0x06]);

		// and can specify base address relative start / end positions with these for data reading to be cut off?
		uint16_t topadr = ((m_bmp_base[0x09] << 8) | m_bmp_base[0x08]);
		uint16_t botadr = ((m_bmp_base[0x0b] << 8) | m_bmp_base[0x0a]);
		uint16_t lftadr = ((m_bmp_base[0x0d] << 8) | m_bmp_base[0x0c]);
		uint16_t rgtadr = ((m_bmp_base[0x0f] << 8) | m_bmp_base[0x0e]);

		uint16_t start = ((m_bmp_base[0x11] << 8) | m_bmp_base[0x10]);
		uint8_t end = m_bmp_base[0x12]; // ?? related to width?
		uint8_t size = m_bmp_base[0x13]; // some kind of additional scaling?
		uint8_t mode = m_bmp_base[0x14]; // enable,bpp, zval etc.

		uint32_t unused = ((m_bmp_base[0x15] << 16) | (m_bmp_base[0x16] << 8) | (m_bmp_base[0x17] << 0));

		if (mode & 0x01)
		{
			popmessage("bitmap t:%04x b:%04x l:%04x r:%04x  -- -- ba:%04x la:%04x ra:%04x   -- -- end:%02x - size:%02x unused:%08x",
				top, bot, lft, rgt,
				/*topadr*/ botadr, lftadr, rgtadr,
				/*start*/ end, size, unused);

			int base = start * 0x800;
			int base2 = topadr * 0x8;

			int bpp = ((mode & 0x0e) >> 1) + 1;
			int zval = ((mode & 0xf0) >> 4);

			int width = (rgtadr * 8) / bpp;

			//int count = 0;

			for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
			{
				int line = y - top;

				if ((line > 0) && (y < bot))
				{
					set_data_address(base + base2 + ((line * width * bpp) / 8), 0);

					for (int x = 0; x < width; x++)
					{
						uint32_t *const yposptr = &bitmap.pix(y);
						uint16_t *const zyposptr = &m_zbuffer.pix(y);

						uint8_t dat = 0;
						for (int i = 0; i < bpp; i++)
						{
							dat |= (get_next_bit() << i);
						}

						if (((x <= cliprect.max_x) && (x >= cliprect.min_x)) && ((y <= cliprect.max_y) && (y >= cliprect.min_y)))
						{
							if ((m_bmp_palram_sh[dat] & 0x1f) < 24) // same transparency logic as everything else? (baseball title)
							{
								if (zval >= zyposptr[x])
								{
									yposptr[x] = paldata[dat + 0x100];
									zyposptr[x] = zval;
								}
							}
						}
					}
				}
			}

		}
	}

	return 0;
}


void xavix_state::spritefragment_dma_params_1_w(offs_t offset, uint8_t data)
{
	m_spritefragment_dmaparam1[offset] = data;
}

void xavix_state::spritefragment_dma_params_2_w(offs_t offset, uint8_t data)
{
	m_spritefragment_dmaparam2[offset] = data;
}

void xavix_state::spritefragment_dma_trg_w(uint8_t data)
{
	uint16_t len = data & 0x07;
	uint16_t src = (m_spritefragment_dmaparam1[1] << 8) | m_spritefragment_dmaparam1[0];
	uint16_t dst = (m_spritefragment_dmaparam2[0] << 8);

	uint8_t unk = m_spritefragment_dmaparam2[1];

	LOG("%s: spritefragment_dma_trg_w with trg %02x size %04x src %04x dest %04x unk (%02x)\n", machine().describe_context(), data & 0xf8, len, src, dst, unk);

	if (unk)
	{
		fatalerror("m_spritefragment_dmaparam2[1] != 0x00 (is %02x)\n", m_spritefragment_dmaparam2[1]);
	}

	if (len == 0x00)
	{
		len = 0x08;
		LOG(" (length was 0x0, assuming 0x8)\n");
	}

	len = len << 8;

	if (data & 0x40)
	{
		for (int i = 0; i < len; i++)
		{
			//uint8_t dat = m_maincpu->read_full_data_sp(src + i);
			uint8_t dat = read_full_data_sp_bypass(src + i);
			//m_fragment_sprite[(dst + i) & 0x7ff] = dat;
			spriteram_w((dst + i) & 0x7ff, dat);
		}
	}
}

uint8_t xavix_state::spritefragment_dma_status_r()
{
	// expects bit 0x40 to clear in most cases
	return 0x00;
}

uint8_t xavix_state::pal_ntsc_r()
{
	// only seen 0x10 checked in code
	// in monster truck the tile base address gets set based on this, there are 2 copies of the test screen in rom, one for pal, one for ntsc, see 1854c
	// likewise card night has entirely different tilesets for each region title
	return m_region->read();
}


void xavix_state::tmap1_regs_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	/*
	   0x0 pointer to low tile bits
	   0x1 pointer to middle tile bits
	   0x2 pointer to upper tile bits

	   0x3 Fftt bbb-  Ff = flip Y,X
	                  tt = tile/tilemap size
	                  b = bpp
	                  - = unused

	   0x4 scroll
	   0x5 scroll

	   0x6 pppp zzzz  p = palette
	                  z = priority

	   0x7 e--m mmmm  e = enable
	                  m = mode

	   modes are
	    ---0 0000 (00) 8-bit addressing  (Tile Number)
	    ---0 0001 (01) 16-bit addressing (Tile Number) (monster truck, ekara)
	    ---0 0010 (02) 16-bit addressing (8-byte alignment Addressing Mode) (boxing)
	    ---0 0011 (03) 16-bit addressing (Addressing Mode 2)
	    ---0 0100 (04) 24-bit addressing (Addressing Mode 2) (epo_efdx)

	    ---0 1000 (08) 8-bit+8 addressing  (Tile Number + 8-bit Attribute)
	    ---0 1001 (09) 16-bit+8 addressing (Tile Number + 8-bit Attribute) (Taito Nostalgia 2)
	    ---0 1010 (0a) 16-bit+8 addressing (8-byte alignment Addressing Mode + 8-bit Attribute) (boxing, Snowboard)
	    ---0 1011 (0b) 16-bit+8 addressing (Addressing Mode 2 + 8-bit Attribute)

	    ---1 0011 (13) 16-bit addressing (Addressing Mode 2 + Inline Header)  (monster truck)
	    ---1 0100 (14) 24-bit addressing (Addressing Mode 2 + Inline Header)

	*/

	if ((offset != 0x4) && (offset != 0x5))
	{
		LOG("%s: tmap1_regs_w offset %02x data %02x\n", machine().describe_context(), offset, data);
	}

	COMBINE_DATA(&m_tmap1_regs[offset]);
}

void xavix_state::tmap2_regs_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	// same as above but for 2nd tilemap
	if ((offset != 0x4) && (offset != 0x5))
	{
		LOG("%s: tmap2_regs_w offset %02x data %02x\n", machine().describe_context(), offset, data);
	}

	COMBINE_DATA(&m_tmap2_regs[offset]);
}


void xavix_state::spriteregs_w(uint8_t data)
{
	LOG("%s: spriteregs_w data %02x\n", machine().describe_context(), data);
	/*
	    This is similar to Tilemap reg 7 and is used to set the addressing mode for sprite data

	    ---0 -000 (00) 8-bit addressing  (Tile Number)
	    ---0 -001 (01) 16-bit addressing (Tile Number)
	    ---0 -010 (02) 16-bit addressing (8-byte alignment Addressing Mode)
	    ---0 -011 (03) 16-bit addressing (Addressing Mode 2)
	    ---0 -100 (04) 24-bit addressing (Addressing Mode 2)

	    ---1 -011 (13) 16-bit addressing (Addressing Mode 2 + Inline Header)
	    ---1 -100 (14) 24-bit addressing (Addressing Mode 2 + Inline Header)
	*/
	m_spritereg = data;
}

uint8_t xavix_state::tmap1_regs_r(offs_t offset)
{
	LOG("%s: tmap1_regs_r offset %02x\n", offset, machine().describe_context());
	return m_tmap1_regs[offset];
}

uint8_t xavix_state::tmap2_regs_r(offs_t offset)
{
	LOG("%s: tmap2_regs_r offset %02x\n", offset, machine().describe_context());
	return m_tmap2_regs[offset];
}

// The Text Array / Memory Emulator acts as a memory area that you can point the tilemap sources at to get a fixed pattern of data
void xavix_state::xavix_memoryemu_txarray_w(offs_t offset, uint8_t data)
{
	if (offset < 0x400)
	{
		// nothing
	}
	else if (offset < 0x800)
	{
		m_txarray[0] = data;
	}
	else if (offset < 0xc00)
	{
		m_txarray[1] = data;
	}
	else if (offset < 0x1000)
	{
		m_txarray[2] = data;
	}
}

uint8_t xavix_state::xavix_memoryemu_txarray_r(offs_t offset)
{
	return txarray_r(offset);
}
