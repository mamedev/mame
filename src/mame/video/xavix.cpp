// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "includes/xavix.h"

inline void xavix_state::set_data_address(int address, int bit)
{
	m_tmp_dataaddress = address;
	m_tmp_databit = bit;
}

inline uint8_t xavix_state::get_next_bit()
{
	uint8_t bit = m_rgn[m_tmp_dataaddress & (m_rgnlen - 1)];
	bit = bit >> m_tmp_databit;
	bit &= 1;

	m_tmp_databit++;

	if (m_tmp_databit == 8)
	{
		m_tmp_databit = 0;
		m_tmp_dataaddress++;
	}

	return bit;
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
}


double xavix_state::hue2rgb(double p, double q, double t)
{
	if (t < 0) t += 1;
	if (t > 1) t -= 1;
	if (t < 1 / 6.0f) return p + (q - p) * 6 * t;
	if (t < 1 / 2.0f) return q;
	if (t < 2 / 3.0f) return p + (q - p) * (2 / 3.0f - t) * 6;
	return p;
}


void xavix_state::handle_palette(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// not verified
	int offs = 0;
	for (int index = 0; index < 256; index++)
	{
		uint16_t dat = m_palram1[offs];
		dat |= m_palram2[offs]<<8;
		offs++;

		int l_raw = (dat & 0x1f00) >> 8;
		int sl_raw =(dat & 0x00e0) >> 5;
		int h_raw = (dat & 0x001f) >> 0;

		//if (h_raw > 24)
		//  logerror("hraw >24 (%02x)\n", h_raw);

		//if (l_raw > 17)
		//  logerror("lraw >17 (%02x)\n", l_raw);

		//if (sl_raw > 7)
		//  logerror("sl_raw >5 (%02x)\n", sl_raw);

		double l = (double)l_raw / 17.0f;
		double s = (double)sl_raw / 7.0f;
		double h = (double)h_raw / 24.0f;

		double r, g, b;

		if (s == 0) {
			r = g = b = l; // greyscale
		}
		else {
			double q = l < 0.5f ? l * (1 + s) : l + s - l * s;
			double p = 2 * l - q;
			r = hue2rgb(p, q, h + 1 / 3.0f);
			g = hue2rgb(p, q, h);
			b = hue2rgb(p, q, h - 1 / 3.0f);
		}

		int r_real = r * 255.0f;
		int g_real = g * 255.0f;
		int b_real = b * 255.0f;

		m_palette->set_pen_color(index, r_real, g_real, b_real);

	}
}

void xavix_state::draw_tilemap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int which)
{
	int alt_tileaddressing = 0;
	int alt_tileaddressing2 = 0;

	int ydimension = 0;
	int xdimension = 0;
	int ytilesize = 0;
	int xtilesize = 0;
	int offset_multiplier = 0;
	int opaque = 0;

	uint8_t* tileregs;
	address_space& mainspace = m_maincpu->space(AS_PROGRAM);

	if (which == 0)
	{
		tileregs = m_tmap1_regs;
		opaque = 1;
	}
	else
	{
		tileregs = m_tmap2_regs;
		opaque = 0;
	}

	switch (tileregs[0x3] & 0x30)
	{
	case 0x00:
		ydimension = 32;
		xdimension = 32;
		ytilesize = 8;
		xtilesize = 8;
		break;

	case 0x10:
		ydimension = 32;
		xdimension = 16;
		ytilesize = 8;
		xtilesize = 16;
		break;

	case 0x20: // guess
		ydimension = 16;
		xdimension = 32;
		ytilesize = 16;
		xtilesize = 8;
		break;

	case 0x30:
		ydimension = 16;
		xdimension = 16;
		ytilesize = 16;
		xtilesize = 16;
		break;
	}

	offset_multiplier = (ytilesize * xtilesize)/8;

	if (tileregs[0x7] & 0x10)
		alt_tileaddressing = 1;
	else
		alt_tileaddressing = 0;

	if (tileregs[0x7] & 0x02)
		alt_tileaddressing2 = 1;

	static int hackx = 1;

	if (machine().input().code_pressed_once(KEYCODE_Q))
	{
		hackx--;
		logerror("%02x\n", hackx);
	}

	if (machine().input().code_pressed_once(KEYCODE_W))
	{
		hackx++;
		logerror("%02x\n", hackx);
	}

	if (tileregs[0x7] & 0x80)
	{
		//logerror("draw tilemap %d, regs base0 %02x base1 %02x base2 %02x tilesize,bpp %02x scrollx %02x scrolly %02x pal %02x mode %02x\n", which, tileregs[0x0], tileregs[0x1], tileregs[0x2], tileregs[0x3], tileregs[0x4], tileregs[0x5], tileregs[0x6], tileregs[0x7]);

		// there's a tilemap register to specify base in main ram, although in the monster truck test mode it points to an unmapped region
		// and expected a fixed layout, we handle that in the memory map at the moment
		int count;

		count = 0;// ;
		for (int y = 0; y < ydimension; y++)
		{
			for (int x = 0; x < xdimension; x++)
			{
				int bpp, pal, scrolly, scrollx;
				int tile = 0;
				int extraattr = 0;

				// the register being 0 probably isn't the condition here
				if (tileregs[0x0] != 0x00) tile |= mainspace.read_byte((tileregs[0x0] << 8) + count);
				if (tileregs[0x1] != 0x00) tile |= mainspace.read_byte((tileregs[0x1] << 8) + count) << 8;

				// at the very least boxing leaves unwanted bits set in ram after changing between mode 0x8a and 0x82, expecting them to not be used
				if (tileregs[0x7] & 0x08)
					extraattr = mainspace.read_byte((tileregs[0x2] << 8) + count);

				count++;

				bpp = (tileregs[0x3] & 0x0e) >> 1;
				bpp++;
				pal = (tileregs[0x6] & 0xf0) >> 4;
				scrolly = tileregs[0x5];
				scrollx = tileregs[0x4];

				int basereg;
				int flipx = 0;
				int flipy = 0;
				int gfxbase;

				// tile 0 is always skipped, doesn't even point to valid data packets in alt mode
				// this is consistent with the top layer too
				// should we draw as solid in solid layer?
				if (tile == 0)
					continue;

				const int debug_packets = 0;
				int test = 0;

				if (!alt_tileaddressing)
				{
					if (!alt_tileaddressing2)
					{
						basereg = 0;
						gfxbase = (m_spr_attra[(basereg * 2) + 1] << 16) | (m_spr_attra[(basereg * 2)] << 8);

						tile = tile * (offset_multiplier * bpp);
						tile += gfxbase;
					}
					else
					{
						tile = tile * 8;
						basereg = (tile & 0x70000) >> 16;
						tile &= 0xffff;
						gfxbase = (m_spr_attra[(basereg * 2) + 1] << 16) | (m_spr_attra[(basereg * 2)] << 8);
						tile += gfxbase;

						if (tileregs[0x7] & 0x08)
						{
							// make use of the extraattr stuff?
							pal = (extraattr & 0xf0)>>4;
							// low bits are priority?
						}
					}
				}
				else
				{
					if (debug_packets) logerror("for tile %04x (at %d %d): ", tile, (((x * 16) + scrollx) & 0xff), (((y * 16) + scrolly) & 0xff));


					basereg = (tile & 0xf000) >> 12;
					tile &= 0x0fff;
					gfxbase = (m_spr_attra[(basereg * 2) + 1] << 16) | (m_spr_attra[(basereg * 2)] << 8);

					tile += gfxbase;
					set_data_address(tile, 0);

					// there seems to be a packet stored before the tile?!
					// the offset used for flipped sprites seems to specifically be changed so that it picks up an extra byte which presumably triggers the flipping
					uint8_t byte1 = 0;
					int done = 0;
					int skip = 0;

					do
					{
						byte1 = get_next_byte();

						if (debug_packets) logerror(" %02x, ", byte1);

						if (skip == 1)
						{
							skip = 0;
							//test = 1;
						}
						else if ((byte1 & 0x0f) == 0x01)
						{
							// used
						}
						else if ((byte1 & 0x0f) == 0x03)
						{
							// causes next byte to be skipped??
							skip = 1;
						}
						else if ((byte1 & 0x0f) == 0x05)
						{
							// the upper bits are often 0x00, 0x10, 0x20, 0x30, why?
							flipx = 1;
						}
						else if ((byte1 & 0x0f) == 0x06) // there must be other finish conditions too because sometimes this fails..
						{
							// tile data will follow after this, always?
							pal = (byte1 & 0xf0) >> 4;
							done = 1;
						}
						else if ((byte1 & 0x0f) == 0x07)
						{
							// causes next byte to be skipped??
							skip = 1;
						}
						else if ((byte1 & 0x0f) == 0x09)
						{
							// used
						}
						else if ((byte1 & 0x0f) == 0x0a)
						{
							// not seen
						}
						else if ((byte1 & 0x0f) == 0x0b)
						{
							// used
						}
						else if ((byte1 & 0x0f) == 0x0c)
						{
							// not seen
						}
						else if ((byte1 & 0x0f) == 0x0d)
						{
							// used
						}
						else if ((byte1 & 0x0f) == 0x0e)
						{
							// not seen
						}
						else if ((byte1 & 0x0f) == 0x0f)
						{
							// used
						}

					} while (done == 0);
					if (debug_packets) logerror("\n");
					tile = get_current_address_byte();
				}

				if (test == 1) pal = machine().rand() & 0xf;


				draw_tile(screen, bitmap, cliprect, tile, bpp, (x * xtilesize) + scrollx, ((y * ytilesize) - 16) - scrolly, ytilesize, xtilesize, flipx, flipy, pal, opaque);
				draw_tile(screen, bitmap, cliprect, tile, bpp, (x * xtilesize) + scrollx, (((y * ytilesize) - 16) - scrolly) + 256, ytilesize, xtilesize, flipx, flipy, pal, opaque); // wrap-y
				draw_tile(screen, bitmap, cliprect, tile, bpp, ((x * xtilesize) + scrollx) - 256, ((y * ytilesize) - 16) - scrolly, ytilesize, xtilesize, flipx, flipy, pal, opaque); // wrap-x
				draw_tile(screen, bitmap, cliprect, tile, bpp, ((x * xtilesize) + scrollx) - 256, (((y * ytilesize) - 16) - scrolly) + 256, ytilesize, xtilesize, flipx, flipy, pal, opaque); // wrap-y and x
			}
		}
	}
}

void xavix_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	//logerror("frame\n");
	// priority doesn't seem to be based on list order, there are bad sprite-sprite priorities with either forward or reverse

	for (int i = 0xff; i >= 0; i--)
	{
		/* attribute 0 bits
		   pppp bbb-    p = palette, b = bpp

		   attribute 1 bits
		   ---- ss-f    s = size, f = flipx
		*/

		int ypos = m_spr_ypos[i];
		int xpos = m_spr_xpos[i];
		int tile = (m_spr_addr_hi[i] << 16) | (m_spr_addr_md[i] << 8) | m_spr_addr_lo[i];
		int attr0 = m_spr_attr0[i];
		int attr1 = m_spr_attr1[i];

		int pal = (attr0 & 0xf0) >> 4;
		int flipx = (attr1 & 0x01);

		int drawheight = 16;
		int drawwidth = 16;

		tile &= (m_rgnlen - 1);

		// taito nostalgia 1 also seems to use a different addressing, is it selectable or is the chip different?
		// taito nost attr1 is 84 / 80 / 88 / 8c for the various elements of the xavix logo.  monster truck uses ec / fc / dc / 4c / 5c / 6c (final 6 sprites ingame are 00 00 f0 f0 f0 f0, radar?)

		if ((attr1 & 0x0c) == 0x0c)
		{
			drawheight = 16;
			drawwidth = 16;
			if (m_alt_addressing == 1)
				tile = tile * 128;
			else if (m_alt_addressing == 2)
				tile = tile * 8;
		}
		else if ((attr1 & 0x0c) == 0x08)
		{
			drawheight = 16;
			drawwidth = 8;
			xpos += 4;
			if (m_alt_addressing == 1)
				tile = tile * 64;
			else if (m_alt_addressing == 2)
				tile = tile * 8;
		}
		else if ((attr1 & 0x0c) == 0x04)
		{
			drawheight = 8;
			drawwidth = 16;
			ypos -= 4;
			if (m_alt_addressing == 1)
				tile = tile * 64;
			else if (m_alt_addressing == 2)
				tile = tile * 8;
		}
		else if ((attr1 & 0x0c) == 0x00)
		{
			drawheight = 8;
			drawwidth = 8;
			xpos += 4;
			ypos -= 4;
			if (m_alt_addressing == 1)
				tile = tile * 32;
			else if (m_alt_addressing == 2)
				tile = tile * 8;
		}

		if (m_alt_addressing != 0)
		{
			int basereg = (tile & 0xf0000) >> 16;
			tile &= 0xffff;
			int gfxbase = (m_spr_attra[(basereg * 2) + 1] << 16) | (m_spr_attra[(basereg * 2)] << 8);
			tile+= gfxbase;
		}

		ypos = 0x100 - ypos;

		xpos -= 136;
		ypos -= 152;

		xpos &= 0xff;
		ypos &= 0xff;

		if (ypos >= 192)
			ypos -= 256;

		int bpp = 1;

		bpp = (attr0 & 0x0e) >> 1;
		bpp += 1;

		draw_tile(screen, bitmap, cliprect, tile, bpp, xpos, ypos, drawheight, drawwidth, flipx, 0, pal, 0);
		draw_tile(screen, bitmap, cliprect, tile, bpp, xpos - 256, ypos, drawheight, drawwidth, flipx, 0, pal, 0); // wrap-x
		draw_tile(screen, bitmap, cliprect, tile, bpp, xpos, ypos - 256, drawheight, drawwidth, flipx, 0, pal, 0); // wrap-y
		draw_tile(screen, bitmap, cliprect, tile, bpp, xpos - 256, ypos - 256, drawheight, drawwidth, flipx, 0, pal, 0); // wrap-x,y

		/*
		if ((m_spr_ypos[i] != 0x81) && (m_spr_ypos[i] != 0x80) && (m_spr_ypos[i] != 0x00))
		{
		    logerror("sprite with enable? %02x attr0 %02x attr1 %02x attr3 %02x attr5 %02x attr6 %02x attr7 %02x\n", m_spr_ypos[i], m_spr_attr0[i], m_spr_attr1[i], m_spr_xpos[i], m_spr_addr_lo[i], m_spr_addr_md[i], m_spr_addr_hi[i] );
		}
		*/
	}
}

void xavix_state::draw_tile(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int tile, int bpp, int xpos, int ypos, int drawheight, int drawwidth, int flipx, int flipy, int pal, int opaque)
{
	// set the address here so we can increment in bits in the draw function
	set_data_address(tile, 0);

	for (int y = 0; y < drawheight; y++)
	{
		int row;
		if (flipy)
		{
			row = ypos + (drawheight-1) - y;
		}
		else
		{
			row = ypos + y;
		}

		for (int x = 0; x < drawwidth; x++)
		{

			int col;

			if (flipx)
			{
				col = xpos + (drawwidth-1) - x;
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

			if ((row >= cliprect.min_y && row <= cliprect.max_y) && (col >= cliprect.min_x && col <= cliprect.max_x))
			{
				uint16_t* rowptr;

				rowptr = &bitmap.pix16(row);

				if (opaque)
				{
					rowptr[col] = (dat + (pal << 4)) & 0xff;
				}
				else
				{
					if (dat)
						rowptr[col] = (dat + (pal << 4)) & 0xff;
				}
			}
		}
	}
}

uint32_t xavix_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	handle_palette(screen, bitmap, cliprect);

	bitmap.fill(0, cliprect);

	draw_tilemap(screen,bitmap,cliprect,0);
	draw_sprites(screen,bitmap,cliprect);
	draw_tilemap(screen,bitmap,cliprect,1);

	//popmessage("%02x %02x %02x %02x   %02x %02x %02x %02x   %02x %02x %02x %02x   %02x %02x %02x %02x", m_soundregs[0],m_soundregs[1],m_soundregs[2],m_soundregs[3],m_soundregs[4],m_soundregs[5],m_soundregs[6],m_soundregs[7],m_soundregs[8],m_soundregs[9],m_soundregs[10],m_soundregs[11],m_soundregs[12],m_soundregs[13],m_soundregs[14],m_soundregs[15]);

	return 0;
}


WRITE8_MEMBER(xavix_state::vid_dma_params_1_w)
{
	m_vid_dma_param1[offset] = data;
}

WRITE8_MEMBER(xavix_state::vid_dma_params_2_w)
{
	m_vid_dma_param2[offset] = data;
}

WRITE8_MEMBER(xavix_state::vid_rom_dmatrg_w)
{
	uint16_t len = data & 0x07;
	uint16_t src = (m_vid_dma_param1[1]<<8) | m_vid_dma_param1[0];
	uint16_t dst = (m_vid_dma_param2[0]<<8);
	dst += 0x6000;

	uint8_t unk = m_vid_dma_param2[1];

	logerror("%s: vid_rom_dmatrg_w with trg %02x size %04x src %04x dest %04x unk (%02x)\n", machine().describe_context(), data & 0xf8, len,src,dst,unk);

	if (unk)
	{
		fatalerror("m_vid_dma_param2[1] != 0x00 (is %02x)\n", m_vid_dma_param2[1]);
	}

	if (len == 0x00)
	{
		len = 0x08;
		logerror(" (length was 0x0, assuming 0x8)\n");
	}

	len = len << 8;

	address_space& dmaspace = m_maincpu->space(AS_PROGRAM);

	if (data & 0x40)
	{
		for (int i = 0; i < len; i++)
		{
			uint8_t dat = dmaspace.read_byte(src + i);
			dmaspace.write_byte(dst + i, dat);
		}
	}
}

READ8_MEMBER(xavix_state::vid_rom_dmatrg_r)
{
	// expects bit 0x40 to clear in most cases
	return 0x00;
}

READ8_MEMBER(xavix_state::pal_ntsc_r)
{
	// only seen 0x10 checked in code
	// in monster truck the tile base address gets set based on this, there are 2 copies of the test screen in rom, one for pal, one for ntsc, see 1854c
	// likewise card night has entirely different tilesets for each region title
	return m_region->read();
}

/*
WRITE8_MEMBER(xavix_state::xavix_6fc0_w) // also related to tilemap 1?
{
	logerror("%s: xavix_6fc0_w data %02x\n", machine().describe_context(), data);
}
*/

WRITE8_MEMBER(xavix_state::tmap1_regs_w)
{
	/*
	   0x0 pointer to address where tile data is
	       it gets set to 0x40 in monster truck test mode, which is outside of ram but test mode requires a fixed 'column scan' layout
	       so that might be special

	   0x1 pointer to middle tile bits (if needed, depends on mode) (usually straight after the ram needed for above)

	   0x2 pointer to tile highest tile bits (if needed, depends on mode) (usually straight after the ram needed for above)

	   0x3 --tt bbb-     - = ?  tt = tile/tilemap size b = bpp    (0x36 xavix logo, 0x3c title screen, 0x36 course select)

	   0x4 and 0x5 are scroll

	   0x6 pppp ----     p = palette  - = ?   (0x02 xavix logo, 0x01 course select)

	   0x7 could be mode (16x16, 8x8 etc.)
	        0x00 is disabled?
	        0x80 means 16x16 tiles
	        0x81 might be 8x8 tiles
	        0x93 course / mode select bg / ingame (weird addressing?)


	 */

	/*
	    6aff base registers
	    -- ingame

	    ae 80
	    02 80
	    02 90
	    02 a0
	    02 b0
	    02 c0
	    02 d0
	    02 e0

	    02 00
	    04 80
	    04 90
	    04 a0
	    04 b0
	    04 c0
	    04 d0
	    04 e0

	    -- menu
	    af 80
	    27 80
	    27 90
	    27 a0
	    27 b0
	    27 c0
	    27 d0
	    27 e0

	    27 00
	    00 80
	    00 90
	    00 a0
	    00 b0
	    00 c0
	    00 d0
	    00 e0
	*/


	if ((offset != 0x4) && (offset != 0x5))
	{
		logerror("%s: tmap1_regs_w offset %02x data %02x\n", machine().describe_context(), offset, data);
	}

	COMBINE_DATA(&m_tmap1_regs[offset]);
}

WRITE8_MEMBER(xavix_state::spriteregs_w) // also related to tilemap 2?
{
	// possibly just a mirror of tmap2_regs_w, at least it writes 0x04 here which would be the correct
	// base address to otherwise write at tmap2_regs_w offset 0
//  tmap2_regs_w(space,offset,data);

	logerror("%s: spriteregs_w data %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::tmap2_regs_w)
{
	// same as above but for 2nd tilemap
	if ((offset != 0x4) && (offset != 0x5))
	{
		//logerror("%s: tmap2_regs_w offset %02x data %02x\n", machine().describe_context(), offset, data);
	}

	COMBINE_DATA(&m_tmap2_regs[offset]);
}

READ8_MEMBER(xavix_state::tmap2_regs_r)
{
	// does this return the same data or a status?

	logerror("%s: tmap2_regs_r offset %02x\n", offset, machine().describe_context());
	return m_tmap2_regs[offset];
}

READ8_MEMBER(xavix_state::xavix_4000_r)
{
	if (offset < 0x100)
	{
		return ((offset>>4) | (offset<<4));
	}
	else
	{
		return 0x00;
	}
}

