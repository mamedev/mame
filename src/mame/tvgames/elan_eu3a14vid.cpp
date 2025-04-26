// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "elan_eu3a14vid.h"

DEFINE_DEVICE_TYPE(ELAN_EU3A14_VID, elan_eu3a14vid_device, "elan_eu3a14vid", "Elan EU3A14 Video")

elan_eu3a14vid_device::elan_eu3a14vid_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: elan_eu3a05commonvid_device(mconfig, ELAN_EU3A14_VID, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_bank(*this, finder_base::DUMMY_TAG),
	m_screen(*this, finder_base::DUMMY_TAG),
	m_space_config("regs", ENDIANNESS_NATIVE, 8, 7, 0, address_map_constructor(FUNC(elan_eu3a14vid_device::map), this))
{
}

device_memory_interface::space_config_vector elan_eu3a14vid_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

/* Windowing mode note

 huntin'3 seems to use some registers for a windowing / highlight effect on the trophy room names and "Target Range" mode timer??
 5100 - 0x0f when effect is enabled, 0x00 otherwise?
 5101 - 0x0e in both modes
 5102 - 0x86 in both modes
 5103 - 0x0e in tropy room (left?)                                  / 0x2a in "Target Range" mode (left position?)
 5104 - trophy room window / highlight top, move with cursor        / 0xbf in "Target Range" mode (top?)
 5105 - 0x52 in trophy room (right?)                                / counts from 0xa1 to 0x2a in "Target Range" mode (right position?)
 5106 - trophy room window / highlight bottom, move with cursor     / 0xcb in "Target Range" mode (bottom?)
*/

void elan_eu3a14vid_device::map(address_map &map)
{
	map(0x00, 0x7f).rw(FUNC(elan_eu3a14vid_device::read_unmapped), FUNC(elan_eu3a14vid_device::write_unmapped));

	map(0x00, 0x06).ram(); // see Windowing note above
	map(0x07, 0x07).rw(FUNC(elan_eu3a14vid_device::reg5107_r), FUNC(elan_eu3a14vid_device::reg5107_w)); // writes on transitions, maybe layer disables?
	map(0x08, 0x08).rw(FUNC(elan_eu3a14vid_device::reg5108_r), FUNC(elan_eu3a14vid_device::reg5108_w)); // 0x04 in both modes // hnt3, frequently rewrites same values TODO
	map(0x09, 0x09).rw(FUNC(elan_eu3a14vid_device::reg5109_r), FUNC(elan_eu3a14vid_device::reg5109_w)); // related to above?
	// 0x0a
	// 0x0b
	// 0x0c
	// 0x0d
	// 0x0e
	// 0x0f
	map(0x10, 0x15).rw(FUNC(elan_eu3a14vid_device::tilecfg_r), FUNC(elan_eu3a14vid_device::tilecfg_w));
	map(0x16, 0x17).rw(FUNC(elan_eu3a14vid_device::rowscrollcfg_r), FUNC(elan_eu3a14vid_device::rowscrollcfg_w));
	// 0x18
	// 0x19
	map(0x1a, 0x1e).rw(FUNC(elan_eu3a14vid_device::rowscrollsplit_r), FUNC(elan_eu3a14vid_device::rowscrollsplit_w));
	// 0x1f
	// 0x20
	map(0x21, 0x24).rw(FUNC(elan_eu3a14vid_device::scrollregs_r), FUNC(elan_eu3a14vid_device::scrollregs_w));   // 0x21,0x22 = scroll reg 1,  0x23,0x24 = scroll reg 2
	map(0x25, 0x2c).rw(FUNC(elan_eu3a14vid_device::rowscrollregs_r), FUNC(elan_eu3a14vid_device::rowscrollregs_w)); // 0x25,0x26 = rowscroll reg 1, 0x27,0x28 = rowscroll reg 2, 0x29,0x2a = rowscroll reg 3, 0x2b,0x2c = rowscroll reg 3
	// 0x2d
	// 0x2e
	// 0x2f
	// 0x30
	// 0x31
	// 0x32
	// 0x33
	// 0x34
	// 0x35
	// 0x36
	// 0x37
	// 0x38
	// 0x39
	// 0x3a
	// 0x3b
	// 0x3c
	// 0x3d
	// 0x3e
	// 0x3f
	map(0x40, 0x45).rw(FUNC(elan_eu3a14vid_device::ramtilecfg_r), FUNC(elan_eu3a14vid_device::ramtilecfg_w));
	// 0x46
	// 0x47
	map(0x48, 0x4b).ram();  // hnt3 (always 0 tho?)
	// 0x4c
	// 0x4d
	// 0x4e
	// 0x4f
	map(0x50, 0x50).rw(FUNC(elan_eu3a14vid_device::spriteaddr_r), FUNC(elan_eu3a14vid_device::spriteaddr_w));
	map(0x51, 0x52).rw(FUNC(elan_eu3a14vid_device::spritebase_r), FUNC(elan_eu3a14vid_device::spritebase_w));
	map(0x53, 0x53).ram(); // startup

	// nothing else used?
}

void elan_eu3a14vid_device::device_start()
{
	elan_eu3a05commonvid_device::device_start();

	save_item(NAME(m_scrollregs));
	save_item(NAME(m_tilecfg));
	save_item(NAME(m_rowscrollregs));
	save_item(NAME(m_rowscrollsplit));
	save_item(NAME(m_rowscrollcfg));
	save_item(NAME(m_ramtilecfg));
	save_item(NAME(m_spritebase));

	save_item(NAME(m_5107));
	save_item(NAME(m_5108));
	save_item(NAME(m_5109));

	save_item(NAME(m_spriteaddr));
}

void elan_eu3a14vid_device::device_reset()
{
	elan_eu3a05commonvid_device::device_reset();

	for (int i = 0; i < 4; i++)
		m_scrollregs[i] = 0x00;

	for (int i = 0; i < 6; i++)
		m_tilecfg[i] = 0x00;

	for (int i = 0; i < 8; i++)
		m_rowscrollregs[i] = 0x00;

	for (int i = 0; i < 5; i++)
		m_rowscrollsplit[i] = 0x00;

	for (int i = 0; i < 2; i++)
		m_rowscrollcfg[i] = 0x00;

	for (int i = 0; i < 6; i++)
		m_ramtilecfg[i] = 0x00;

	for (int i = 0; i < 2; i++)
		m_spritebase[i] = 0x00;

	m_5107 = 0x00;
	m_5108 = 0x00;
	m_5109 = 0x00;

	// TODO: rad_foot and tsbuzz never write, other games seem to use address 0x5153 to set spriteram address
	m_spriteaddr = m_default_spriteramaddr;
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
	return fullbankspace.read_byte((offset+x) & 0x7fffff);
}

uint8_t elan_eu3a14vid_device::readpix(int baseaddr, int count, int drawfromram)
{
	int pix;

	if (drawfromram)
	{
		pix = read_vram((baseaddr + count) & 0x3fff);
	}
	else
	{
		address_space& fullbankspace = m_bank->space(AS_PROGRAM);
		pix =  fullbankspace.read_byte((baseaddr+count) & 0x7fffff);
	}
	return pix;
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

		uint16_t *const dst = &bitmap.pix(ypos + y);
		uint8_t *const pridst = &m_prioritybitmap.pix(ypos + y);

		for (int x = 0; x < xstride; x++)
		{
			if (realy >= cliprect.min_y && realy <= cliprect.max_y)
			{
				if (bpp == 8) // 8bpp
				{
					int realx = x + xposwithscroll;
					if (realx >= cliprect.min_x && realx <= cliprect.max_x)
					{
						uint8_t pix = readpix(baseaddr, count, drawfromram);

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

					if (((realx + 1) >= cliprect.min_x) || (realx <= cliprect.max_x))
					{
						uint8_t pix = readpix(baseaddr, count, drawfromram);

						int mask = 0xf0;
						int shift = 4;

						for (int i = 0; i < 4; i++)
						{
							if (realx >= cliprect.min_x && realx <= cliprect.max_x)
							{
								if (pix & mask)
								{
									if (pridst[realx] <= priority)
									{
										dst[realx] = ((pix & mask) >> shift) | palette;
										pridst[realx] = priority;
									}
								}
							}
							mask >>= 4;
							shift -= 4;
							realx++;
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

					if (((realx + 3) >= cliprect.min_x) || (realx <= cliprect.max_x))
					{
						uint8_t pix = readpix(baseaddr, count, drawfromram);

						int mask = 0xc0;
						int shift = 6;

						for (int i = 0; i < 4; i++)
						{
							if (realx >= cliprect.min_x && realx <= cliprect.max_x)
							{
								if (pix & 0xc0)
								{
									if (pridst[realx] <= priority)
									{
										dst[realx] = ((pix & mask) >> shift) | palette;
										pridst[realx] = priority;
									}
								}
							}

							realx++;
							mask >>= 2;
							shift -= 2;
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
		int rtm_size;
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

	if ((m_tilecfg[0] & 0x03) == 0x00) // 2 pages wide, 2 pages high
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
	else if ((m_tilecfg[0] & 0x03) == 0x01) // 2 pages wide, 1 page high
	{
		ramstart = m_tilerambase + pagesize * 0;
		ramend = m_tilerambase + pagesize * 1;

		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, 0,                        0 - yscroll,                          size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // normal
		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, (size * pagewidth * 2),   0 - yscroll,                          size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // wrap x
		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, 0,                       (size * pageheight) + 0 - yscroll,     size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // wrap y
		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, (size * pagewidth * 2),  (size * pageheight) + 0 - yscroll,     size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // wrap x+y

		ramstart = m_tilerambase + pagesize * 1;
		ramend = m_tilerambase + pagesize * 2;

		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, (size * pagewidth),     0 - yscroll,                           size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // normal
		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, (size * pagewidth * 3), 0 - yscroll,                           size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // wrap x
		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, (size * pagewidth),     (size * pageheight) + 0 - yscroll,     size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // wrap y
		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, (size * pagewidth * 3), (size * pageheight) + 0 - yscroll,     size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // wrap x+y
	}
	else if ((m_tilecfg[0] & 0x03) == 0x03) // 1 page wide, 1 page high
	{
	//  popmessage("m_tilecfg[0] & 0x03 multiple layers config %04x", base);
		ramstart = m_tilerambase + pagesize * 0;
		ramend = m_tilerambase + pagesize * 1;

		draw_background_page(screen, bitmap, cliprect, ramstart, ramend, 0,                  0 - yscroll,                       size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // normal
		draw_background_page(screen, bitmap, cliprect, ramstart, ramend, (size * pagewidth), 0 - yscroll,                       size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // wrap x
		draw_background_page(screen, bitmap, cliprect, ramstart, ramend, 0,                  (size * pageheight) + 0 - yscroll, size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // wrap y
		draw_background_page(screen, bitmap, cliprect, ramstart, ramend, (size * pagewidth), (size * pageheight) + 0 - yscroll, size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // wrap x+y

		// RAM based tile layer (probably has it's own enable?)
		draw_background_ramlayer(screen, bitmap, cliprect);
	}
	else // might be 1 page wide, 2 high, not seen yet
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
		uint16_t *const dst = &bitmap.pix(ypos);
		uint8_t *const pridst = &m_prioritybitmap.pix(ypos);

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
uint8_t elan_eu3a14vid_device::read_unmapped(offs_t offset)
{
	logerror("%s: elan_eu3a14vid_device::read_unmapped (offset %02x)\n", machine().describe_context(), offset);
	return 0x00;
}

void elan_eu3a14vid_device::write_unmapped(offs_t offset, uint8_t data)
{
	logerror("%s: elan_eu3a14vid_device::write_unmapped (offset %02x) (data %02x)\n", machine().describe_context(), offset, data);
}
