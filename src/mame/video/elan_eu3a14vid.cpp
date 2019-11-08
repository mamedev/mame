// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "elan_eu3a14vid.h"

DEFINE_DEVICE_TYPE(ELAN_EU3A14_VID, elan_eu3a14vid_device, "elan_eu3a14vid", "Elan EU3A14 Video")

elan_eu3a14vid_device::elan_eu3a14vid_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: elan_eu3a05commonvid_device(mconfig, ELAN_EU3A14_VID, tag, owner, clock),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_bank(*this, finder_base::DUMMY_TAG),
	m_screen(*this, finder_base::DUMMY_TAG)
{
}

void elan_eu3a14vid_device::device_start()
{
	elan_eu3a05commonvid_device::device_start();
}

void elan_eu3a14vid_device::device_reset()
{
	elan_eu3a05commonvid_device::device_reset();

	m_spriteaddr = 0x14; // ?? rad_foot never writes, other games seem to use it to set sprite location
}



uint8_t elan_eu3a14vid_device::read_vram(int offset)
{
	address_space& cpuspace = m_cpu->space(AS_PROGRAM);
	int realoffset = offset + 0x200;
	if (realoffset < 0x4000)
	{
		return cpuspace.read_byte(realoffset);
	}
	else
		return 0x00;
}

uint32_t elan_eu3a14vid_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_spriterambase = (m_spriteaddr * 0x200) - 0x200;

	bitmap.fill(0, cliprect);
	m_prioritybitmap.fill(0, cliprect);

	draw_background(screen, bitmap, cliprect);

	draw_sprites(screen, bitmap, cliprect);
	return 0;
}

void elan_eu3a14vid_device::video_start()
{
	m_screen->register_screen_bitmap(m_prioritybitmap);
}


uint8_t elan_eu3a14vid_device::read_gfxdata(int offset, int x)
{
	address_space& fullbankspace = m_bank->space(AS_PROGRAM);
	return fullbankspace.read_byte((offset+x) & 0x3fffff);
}

void elan_eu3a14vid_device::draw_background_tile(bitmap_ind16& bitmap, const rectangle& cliprect, int bpp, int tileno, int palette, int priority, int flipx, int flipy, int xpos, int ypos, int transpen, int size, int base, int drawfromram)
{
	int baseaddr = base * 256;

	int xstride = 8;

	if (bpp == 8) // 8bpp selection
	{
		if (size == 8)
		{
			xstride = size / 1; baseaddr += tileno * 64; // 8x8 8bpp
		}
		else
		{
			xstride = size / 1; baseaddr += tileno * 256; // 16x16 8bpp
		}

		palette &= 0x100; // only top bit valid, as there are only 2 palettes?
	}
	else if (bpp == 4) // 4bpp selection
	{
		if (size == 8)
		{
			xstride = size / 2; baseaddr += tileno * 32; // 8x8 4bpp
		}
		else
		{
			xstride = size / 2; baseaddr += tileno * 128; // 16x16 4bpp
		}
	}
	else if (bpp == 2) // 2bpp?
	{
		xstride = size / 4; baseaddr += tileno * 64; // 16x16 2bpp
	}
	else
	{
		popmessage("draw_background_tile unsupported bpp %d\n", bpp);
		return;
	}


	int count = 0;
	for (int y = 0; y < size; y++)
	{
		int realy = ypos + y;
		int xposwithscroll = 0;

		if (!drawfromram)
		{
			xposwithscroll = xpos - get_xscroll_for_screenypos(realy);
		}
		else
		{
			xposwithscroll = xpos;
			// RAM tile layer has no scrolling? (or we've never seen it used / enabled)
		}

		uint16_t* dst = &bitmap.pix16(ypos + y);
		uint8_t* pridst = &m_prioritybitmap.pix8(ypos + y);

		for (int x = 0; x < xstride; x++)
		{
			int pix;

			if (drawfromram)
			{
				pix = read_vram((baseaddr + count) & 0x3fff);
			}
			else
			{
				address_space& fullbankspace = m_bank->space(AS_PROGRAM);
				pix =  fullbankspace.read_byte((baseaddr+count) & 0x3fffff);
			}

			if (realy >= cliprect.min_y && realy <= cliprect.max_y)
			{
				if (bpp == 8) // 8bpp
				{
					int realx = x + xposwithscroll;
					if (realx >= cliprect.min_x && realx <= cliprect.max_x)
					{
						if (pix)
						{
							if (pridst[realx] <= priority)
							{
								dst[realx] = pix | palette;
								pridst[realx] = priority;
							}

						}
					}
				}
				else if (bpp == 7)
				{
					popmessage("draw_background_tile bpp == 7");
				}
				else if (bpp == 6)
				{
					popmessage("draw_background_tile bpp == 6");
				}
				else if (bpp == 5)
				{
					popmessage("draw_background_tile bpp == 5");
				}
				else if (bpp == 4) // 4bpp
				{
					int realx = (x * 2) + xposwithscroll;
					if (realx >= cliprect.min_x && realx <= cliprect.max_x)
					{
						if (pix & 0xf0)
						{
							if (pridst[realx] <= priority)
							{
								dst[realx] = ((pix & 0xf0) >> 4) | palette;
								pridst[realx] = priority;
							}
						}
					}

					realx++;

					if (realx >= cliprect.min_x && realx <= cliprect.max_x)
					{
						if (pix & 0x0f)
						{
							if (pridst[realx] <= priority)
							{
								dst[realx] = (pix & 0x0f) | palette;
								pridst[realx] = priority;
							}
						}
					}
				}
				else if (bpp == 3)
				{
					popmessage("draw_background_tile bpp == 3");
				}
				else if (bpp == 2) // 2bpp (hnt3 ram text)
				{
					int realx = (x * 4) + xposwithscroll;
					if (realx >= cliprect.min_x && realx <= cliprect.max_x)
					{
						if (pix & 0xc0)
						{
							if (pridst[realx] <= priority)
							{
								dst[realx] = ((pix & 0xc0) >> 6) | palette;
								pridst[realx] = priority;
							}
						}
					}

					realx++;

					if (realx >= cliprect.min_x && realx <= cliprect.max_x)
					{
						if (pix & 0x30)
						{
							if (pridst[realx] <= priority)
							{
								dst[realx] = ((pix & 0x30) >> 4) | palette;
								pridst[realx] = priority;
							}
						}
					}

					realx++;

					if (realx >= cliprect.min_x && realx <= cliprect.max_x)
					{
						if (pix & 0x0c)
						{
							if (pridst[realx] <= priority)
							{
								dst[realx] = ((pix & 0x0c) >> 2) | palette;
								pridst[realx] = priority;
							}
						}
					}

					realx++;

					if (realx >= cliprect.min_x && realx <= cliprect.max_x)
					{
						if (pix & 0x03)
						{
							if (pridst[realx] <= priority)
							{
								dst[realx] = ((pix & 0x03) >> 0) | palette;
								pridst[realx] = priority;
							}
						}
					}
				}
				else if (bpp == 1)
				{
					popmessage("draw_background_tile bpp == 1");
				}
			}
			count++;
		}
	}
}

int elan_eu3a14vid_device::get_xscroll_for_screenypos(int ydraw)
{
	if ((ydraw < 0) || (ydraw >= 224))
		return 0;

	int xscroll = m_scrollregs[0] | (m_scrollregs[1] << 8);

	if (m_rowscrollcfg[1] == 0x01) // GUESS! could be anything, but this bit is set in Huntin'3
	{
		int split0 = m_rowscrollregs[0] | (m_rowscrollregs[1] << 8);
		int split1 = m_rowscrollregs[2] | (m_rowscrollregs[3] << 8);
		int split2 = m_rowscrollregs[4] | (m_rowscrollregs[5] << 8);
		int split3 = m_rowscrollregs[6] | (m_rowscrollregs[7] << 8);

		if (ydraw < m_rowscrollsplit[0])
		{
			return xscroll;
		}
		else if (ydraw < m_rowscrollsplit[1])
		{
			return split0;
		}
		else if (ydraw < m_rowscrollsplit[2])
		{
			return split1;
		}
		else if (ydraw < m_rowscrollsplit[3])
		{
			return split2;
		}
		else if (ydraw < m_rowscrollsplit[4])
		{
			return split3;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return xscroll;
	}
}


void elan_eu3a14vid_device::draw_background_page(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect, int ramstart, int ramend, int xbase, int ybase, int size, int bpp, int base, int pagewidth, int pageheight, int bytespertile, int palettepri, int drawfromram)
{

	int palette = ((palettepri & 0xf0) >> 4) | ((palettepri & 0x08) << 1);
	palette = palette << 4;
	int priority = palettepri & 0x07;

	int xdraw = xbase;
	int ydraw = ybase;
	int count = 0;

	for (int i = ramstart; i < ramend; i += bytespertile)
	{
		int tile = 0;
		int realpalette = palette;
		int realpriority = priority;
		int realbpp = bpp;

		if (bytespertile == 2)
		{
			tile = read_vram(i + 0) | (read_vram(i + 1) << 8);
		}
		else if (bytespertile == 4) // rad_foot hidden test mode, rad_hnt3 "Target Range" (not yet correct)
		{
			tile = read_vram(i + 0) | (read_vram(i + 1) << 8);// | (read_vram(i + 2) << 16) |  | (read_vram(i + 3) << 24);

			// read_vram(i + 3) & 0x04 is set in both seen cases, maybe per-tile bpp?
			// this would match up with this mode being inline replacements for m_tilecfg[1] (palettepri) and m_tilecfg[2] (bpp);

			int newpalette = ((read_vram(i + 2) & 0xf0) >> 4) | ((read_vram(i + 2) & 0x08) << 1);
			newpalette = newpalette << 4;
			realpalette = newpalette;
			realpriority = read_vram(i + 2) & 0x07;
			realbpp = read_vram(i + 3) & 0x07;
			if (realbpp == 0)
				realbpp = 8;

		}


		draw_background_tile(bitmap, cliprect, realbpp, tile, realpalette, realpriority, 0, 0, xdraw, ydraw, 0, size, base, drawfromram);

		xdraw += size;

		count++;
		if (((count % pagewidth) == 0))
		{
			xdraw -= size * pagewidth;
			ydraw += size;
		}
	}
}

void elan_eu3a14vid_device::draw_background_ramlayer(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect)
{
	// this register use is questionable
	if (m_ramtilecfg[0] & 0x80)
	{
		int rtm_size;;
		int rtm_pagewidth;
		int rtm_pageheight;
		int rtm_yscroll;
		int rtm_bpp;
		int rtm_bytespertile = 2;
		uint8_t palettepri = m_ramtilecfg[1];

		rtm_yscroll = 0;

		// this is the gfxbase in ram for all cases seen
		int rtm_base = (0x2000 - 0x200) / 256;

		// same as regular layer?
		if (m_ramtilecfg[0] & 0x10)
		{
			rtm_size = 8;
			rtm_pagewidth = 32;
			rtm_pageheight = 28;
		}
		else
		{
			rtm_size = 16;
			rtm_pagewidth = 32 / 2;
			rtm_pageheight = 28 / 2;
		}

		rtm_bpp = m_ramtilecfg[2] & 0x07;
		if (rtm_bpp == 0)
			rtm_bpp = 8;

		// this is in the same place even when the first tilemap is in 16x16 mode, probably a base register somewhere
		int ramstart = m_tilerambase + 0x700;
		int ramend = m_tilerambase + 0x700 + 0x700;

		// hack for "Target Range" mode
		if (m_ramtilecfg[5] == 0x06)
		{
			ramstart = 0x3980-0x200;
			ramend = 0x3980-0x200 + 0x700;
		}

		// normal
		draw_background_page(screen, bitmap, cliprect, ramstart, ramend, 0, 0 - rtm_yscroll, rtm_size, rtm_bpp, rtm_base, rtm_pagewidth, rtm_pageheight, rtm_bytespertile, palettepri, 1);
		// wrap x
		draw_background_page(screen, bitmap, cliprect, ramstart, ramend, (rtm_size * rtm_pagewidth), 0 - rtm_yscroll, rtm_size, rtm_bpp, rtm_base, rtm_pagewidth, rtm_pageheight, rtm_bytespertile, palettepri, 1);
		// wrap y
		draw_background_page(screen, bitmap, cliprect, ramstart, ramend, 0, (rtm_size * rtm_pageheight) + 0 - rtm_yscroll, rtm_size, rtm_bpp, rtm_base, rtm_pagewidth, rtm_pageheight, rtm_bytespertile, palettepri, 1);
		// wrap x+y
		draw_background_page(screen, bitmap, cliprect, ramstart, ramend, (rtm_size * rtm_pagewidth), (rtm_size * rtm_pageheight) + 0 - rtm_yscroll, rtm_size, rtm_bpp, rtm_base, rtm_pagewidth, rtm_pageheight, rtm_bytespertile, palettepri, 1);
	}
}


void elan_eu3a14vid_device::draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int yscroll = m_scrollregs[2] | (m_scrollregs[3] << 8);

	int base = (m_tilecfg[5] << 8) | m_tilecfg[4];
	uint8_t palettepri = m_tilecfg[1];

	int pagewidth = 1, pageheight = 1;
	int bytespertile = 2;
	int size;

	// m_tilecfg[0]   b-as ?-hh    b = bytes per tile  s = tilesize / page size?  a = always set when tilemaps are in use - check? h = related to page positions, when set uses 2x2 pages? ? = used
	// m_tilecfg[1]   pppp x--?    ? = used foot x = used, huntin3 summary (palette bank?) p = palette (used for different stages in huntin3 and the hidden test modes in others)
	// m_tilecfg[2]   ---- bbbb    b = bpp mode (0 = 8bpp)
	// m_tilecfg[3]   ---- ----     (unused or more gfxbase?)
	// m_tilecfg[4]   gggg gggg     gfxbase (lower bits)
	// m_tilecfg[5]   gggg gggg     gfxbase (upper bits)

	// ramtilecfg appears to be a similar format, except for the other layer with ram base tiles
	// however 'a' in m_tilecfg[0] is NOT set
	// also m_tilecfg[0] has 0x80 set, which would be 4 bytes per tile, but it isn't?
	// the layer seems to be disabled by setting m_tilecfg[0] to 0?

	if (m_tilecfg[0] & 0x10)
	{
		size = 8;
		pagewidth = 32;
		pageheight = 28;
	}
	else
	{
		size = 16;
		pagewidth = 16;
		pageheight = 14;
	}

	if (m_tilecfg[0] & 0x80)
	{
		bytespertile = 4;
	}
	else
	{
		bytespertile = 2;
	}

	int bpp = (m_tilecfg[2] & 0x07);
	if (bpp == 0)
		bpp = 8;

	int ramstart = 0;
	int ramend = 0;

	int pagesize = pagewidth * pageheight * 2;

	if (bytespertile == 4)
	{
		pagesize <<= 1; // shift because we need twice as much ram for this mode
	}

	if ((m_tilecfg[0] & 0x03) == 0x00) // tilemaps arranged as 2x2 pages?
	{
		ramstart = m_tilerambase + pagesize * 0;
		ramend = m_tilerambase + pagesize * 1;

		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, 0,                        0 - yscroll,                          size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // normal
		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, (size * pagewidth * 2),   0 - yscroll,                          size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // wrap x
		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, 0,                       (size * pageheight * 2) + 0 - yscroll, size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // wrap y
		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, (size * pagewidth * 2),  (size * pageheight * 2) + 0 - yscroll, size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // wrap x+y

		ramstart = m_tilerambase + pagesize * 1;
		ramend = m_tilerambase + pagesize * 2;

		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, (size * pagewidth),     0 - yscroll,                           size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // normal
		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, (size * pagewidth * 3), 0 - yscroll,                           size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // wrap x
		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, (size * pagewidth),     (size * pageheight * 2) + 0 - yscroll, size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // wrap y
		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, (size * pagewidth * 3), (size * pageheight * 2) + 0 - yscroll, size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // wrap x+y

		ramstart = m_tilerambase + pagesize * 2;
		ramend = m_tilerambase + pagesize * 3;

		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, 0,                      (size * pageheight) - yscroll,     size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // normal
		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, (size * pagewidth * 2), (size * pageheight) - yscroll,     size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // wrap x
		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, 0,                      (size * pageheight * 3) - yscroll, size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // wrap y
		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, (size * pagewidth * 2), (size * pageheight * 3) - yscroll, size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // wrap x+y

		ramstart = m_tilerambase + pagesize * 3;
		ramend = m_tilerambase + pagesize * 4;

		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, (size * pagewidth),     (size * pageheight) - yscroll,     size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // normal
		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, (size * pagewidth * 3), (size * pageheight) - yscroll,     size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // wrap x
		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, (size * pagewidth),     (size * pageheight * 3) - yscroll, size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0);// wrap y
		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, (size * pagewidth * 3), (size * pageheight * 3) - yscroll, size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // wrap x+y
	}
	else if ((m_tilecfg[0] & 0x03) == 0x03) // individual tilemaps? multiple layers?
	{
	//  popmessage("m_tilecfg[0] & 0x03 multiple layers config %04x", base);
		ramstart = m_tilerambase + pagesize * 0;
		ramend = m_tilerambase + pagesize * 1;

		// normal
		draw_background_page(screen, bitmap, cliprect, ramstart, ramend, 0, 0 - yscroll, size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0);
		// wrap x
		draw_background_page(screen, bitmap, cliprect, ramstart, ramend, (size * pagewidth), 0 - yscroll, size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0);
		// wrap y
		draw_background_page(screen, bitmap, cliprect, ramstart, ramend, 0, (size * pageheight) + 0 - yscroll, size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0);
		// wrap x+y
		draw_background_page(screen, bitmap, cliprect, ramstart, ramend, (size * pagewidth), (size * pageheight) + 0 - yscroll, size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0);

		// RAM based tile layer
		draw_background_ramlayer(screen, bitmap, cliprect);
	}
	else
	{
		popmessage("m_tilecfg[0] & 0x03 unknown config");
	}

}

void elan_eu3a14vid_device::draw_sprite_pix(const rectangle& cliprect, uint16_t* dst, uint8_t* pridst, int realx, int priority, uint8_t pix, uint8_t mask, uint8_t shift, int palette)
{
	if (realx >= cliprect.min_x && realx <= cliprect.max_x)
	{
		if (pridst[realx] <= priority)
		{
			if (pix & mask)
			{
				dst[realx] = ((pix & mask) >> shift) | palette;
				pridst[realx] = priority;
			}
		}
	}
}

void elan_eu3a14vid_device::draw_sprite_line(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int offset, int line, int palette, int flipx, int priority, int xpos, int ypos, int bpp)
{
	offset = offset * 2;

	int bppdiv = 0;

	switch (bpp)
	{
	default:
	case 0x8:
	case 0x7:
	case 0x6:
	case 0x5:
		offset += line * 8;
		bppdiv = 1;
		break;

	case 0x4:
	case 0x3:
		offset += line * 4;
		bppdiv = 2;
		break;

	case 0x2:
		offset += line * 2;
		bppdiv = 4;
		break;

	case 0x1:
		offset += line * 1;
		bppdiv = 8;
		break;
	}


	if (ypos >= cliprect.min_y && ypos <= cliprect.max_y)
	{
		uint16_t* dst = &bitmap.pix16(ypos);
		uint8_t* pridst = &m_prioritybitmap.pix8(ypos);

		int count = 0;
		for (int x = 0; x < 8/bppdiv;x++)
		{
			if (bpp == 8)
			{
				int pix,mask,shift;
				if (flipx) { pix = read_gfxdata(offset, 7 - count); } else { pix = read_gfxdata(offset, count); }
				int realx = xpos + x * 1;
				if (flipx) { mask = 0xff; shift = 0; } else { mask = 0xff; shift = 0; }
				draw_sprite_pix(cliprect, dst, pridst, realx, priority, pix, mask, shift, palette);
			}
			else if (bpp == 7)
			{
				int pix,mask,shift;
				if (flipx) { pix = read_gfxdata(offset, 7 - count); } else { pix = read_gfxdata(offset, count); }
				int realx = xpos + x * 1;
				// stride doesn't change, data isn't packed, just don't use top bit
				if (flipx) { mask = 0x7f; shift = 0; } else { mask = 0x7f; shift = 0; }
				draw_sprite_pix(cliprect, dst, pridst, realx, priority, pix, mask, shift, palette);
			}
			else if (bpp == 6)
			{
				popmessage("6bpp sprite\n");
			}
			else if (bpp == 5)
			{
				popmessage("5bpp sprite\n");
			}
			else if (bpp == 4)
			{
				int pix,mask,shift;
				if (flipx) { pix = read_gfxdata(offset, 3 - count); } else { pix = read_gfxdata(offset, count); }
				int realx = xpos + x * 2;
				if (flipx) { mask = 0x0f; shift = 0; } else { mask = 0xf0; shift = 4; }
				draw_sprite_pix(cliprect, dst, pridst, realx, priority, pix, mask, shift, palette);
				realx++;
				if (flipx) { mask = 0xf0; shift = 4; } else { mask = 0x0f; shift = 0; }
				draw_sprite_pix(cliprect, dst, pridst, realx, priority, pix, mask, shift, palette);
			}
			else if (bpp == 3)
			{
				popmessage("3bpp sprite\n");
			}
			else if (bpp == 2)
			{
				int pix,mask,shift;
				if (flipx) { pix = read_gfxdata(offset, 1 - count); } else { pix = read_gfxdata(offset, count); }
				int realx = xpos + x * 4;
				if (flipx) { mask = 0x03; shift = 0; } else { mask = 0xc0; shift = 6; }
				draw_sprite_pix(cliprect, dst, pridst, realx, priority, pix, mask, shift, palette);
				realx++;
				if (flipx) { mask = 0x0c; shift = 2; } else { mask = 0x30; shift = 4; }
				draw_sprite_pix(cliprect, dst, pridst, realx, priority, pix, mask, shift, palette);
				realx++;
				if (flipx) { mask = 0x30; shift = 4; } else { mask = 0x0c; shift = 2; }
				draw_sprite_pix(cliprect, dst, pridst, realx, priority, pix, mask, shift, palette);
				realx++;
				if (flipx) { mask = 0xc0; shift = 6; } else { mask = 0x03; shift = 0; }
				draw_sprite_pix(cliprect, dst, pridst, realx, priority, pix, mask, shift, palette);
			}
			else if (bpp == 1)
			{
				popmessage("1bpp sprite\n");
			}

			count++;
		}
	}

}


void elan_eu3a14vid_device::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// first 4 sprite entries seem to be garbage sprites, so we start at 0x20
	// likely we're just interpreting them wrong and they're used for blanking things or clipping?
	for (int i = m_spriterambase; i < m_spriterambase + 0x800; i += 8)
	{
		/*
		+0  e-ff hhww  flip yx, enable, height, width
		+1  yyyy yyyy  ypos
		+2  xxxx xxxx  xpos
		+3  pppp Pzzz  p = palette, P = upper palette bank, z = priority
		+4  tttt tttt  tile bits
		+5  tttt tttt
		+6  --TT TPPP  TTT = tile bank PPP = bpp select (+more?)
		+7  ---- ----

		*/

		int attr = read_vram(i + 0);
		int y = read_vram(i + 1);
		int x = read_vram(i + 2);
		int palettepri = read_vram(i + 3);

		int h = attr & 0x0c;
		int w = attr & 0x03;
		int flipx = (attr & 0x10) >> 4;
		int flipy = (attr & 0x20) >> 5;

		int height = 0;
		int width = 0;

		int pri = palettepri & 0x07;

		int palette = ((palettepri & 0xf0) >> 4) | ((palettepri & 0x08) << 1);
		palette = palette << 4;

		switch (h)
		{
		case 0x0:height = 2; break;
		case 0x4:height = 4; break;
		case 0x8:height = 8; break;
		case 0xc:height = 16; break;
		}

		switch (w)
		{
		case 0x0:width = 1; break;
		case 0x1:width = 2; break;
		case 0x2:width = 4; break;
		case 0x3:width = 8; break;
		}

		y -= ((height * 2) - 4);

		x -= ((width * 4) - 4);

		height *= 4;

		x -= 6;
		y -= 4;

		int offset = ((read_vram(i + 5) << 8) + (read_vram(i + 4) << 0));
		int extra = read_vram(i + 6);

		int spritebase = (m_spritebase[1] << 8) | m_spritebase[0];

		offset += (extra & 0xf8) << 13;
		offset += spritebase << 7;

		int bpp = extra & 0x07;
		if (bpp == 0)
			bpp = 8;

		if (attr & 0x80)
		{
			int count = 0;
			for (int yy = 0; yy < height; yy++)
			{
				int yoff = flipy ? height-1-yy : yy;

				for (int xx = 0; xx < width; xx++)
				{
					int xoff = flipx ? (((width - 1) * 8) - (xx * 8)) : (xx * 8);

					draw_sprite_line(screen, bitmap, cliprect, offset, count, palette, flipx, pri, x + xoff, y + yoff, bpp);
					count++;
				}
			}
		}
	}
}

