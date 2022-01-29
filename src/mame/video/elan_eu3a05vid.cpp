// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "elan_eu3a05vid.h"

DEFINE_DEVICE_TYPE(ELAN_EU3A05_VID, elan_eu3a05vid_device, "elan_eu3a05vid", "Elan EU3A05 Video")

// tilemaps start at 0x0600 in mainram, sprites at 0x3e00, unlike eu3a14 these could be fixed addresses

elan_eu3a05vid_device::elan_eu3a05vid_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	elan_eu3a05commonvid_device(mconfig, ELAN_EU3A05_VID, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_bank(*this, finder_base::DUMMY_TAG),
	m_space_config("regs", ENDIANNESS_NATIVE, 8, 5, 0, address_map_constructor(FUNC(elan_eu3a05vid_device::map), this)),
	m_bytes_per_tile_entry(4),
	m_vrambase(0x600),
	m_spritebase(0x3e00),
	m_use_spritepages(false),
	m_force_transpen_ff(false),
	m_force_basic_scroll(false)
{
}

device_memory_interface::space_config_vector elan_eu3a05vid_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

void elan_eu3a05vid_device::map(address_map &map)
{
	map(0x00, 0x1f).rw(FUNC(elan_eu3a05vid_device::read_unmapped), FUNC(elan_eu3a05vid_device::write_unmapped));

	map(0x00, 0x06).ram(); // unknown, space invaders sets these to fixed values, tetris has them as 00
	map(0x07, 0x07).rw(FUNC(elan_eu3a05vid_device::elan_eu3a05_vidctrl_r), FUNC(elan_eu3a05vid_device::elan_eu3a05_vidctrl_w));
	map(0x08, 0x08).ram(); // unknown
	map(0x09, 0x09).rw(FUNC(elan_eu3a05vid_device::tile_gfxbase_lo_r), FUNC(elan_eu3a05vid_device::tile_gfxbase_lo_w));
	map(0x0a, 0x0a).rw(FUNC(elan_eu3a05vid_device::tile_gfxbase_hi_r), FUNC(elan_eu3a05vid_device::tile_gfxbase_hi_w));
	map(0x0b, 0x0b).rw(FUNC(elan_eu3a05vid_device::sprite_gfxbase_lo_r), FUNC(elan_eu3a05vid_device::sprite_gfxbase_lo_w));
	map(0x0c, 0x0c).rw(FUNC(elan_eu3a05vid_device::sprite_gfxbase_hi_r), FUNC(elan_eu3a05vid_device::sprite_gfxbase_hi_w));
	map(0x0d, 0x0e).rw(FUNC(elan_eu3a05vid_device::splitpos_r), FUNC(elan_eu3a05vid_device::splitpos_w));
	map(0x0f, 0x16).rw(FUNC(elan_eu3a05vid_device::tile_scroll_r), FUNC(elan_eu3a05vid_device::tile_scroll_w));
	map(0x17, 0x17).ram(); // unknown
	map(0x18, 0x18).ram(); // unknown
	// no other writes seen
}

void elan_eu3a05vid_device::device_start()
{
	elan_eu3a05commonvid_device::device_start();

	save_item(NAME(m_vidctrl));
	save_item(NAME(m_tile_gfxbase_lo_data));
	save_item(NAME(m_tile_gfxbase_hi_data));
	save_item(NAME(m_sprite_gfxbase_lo_data));
	save_item(NAME(m_sprite_gfxbase_hi_data));
	save_item(NAME(m_tile_scroll));
	save_item(NAME(m_splitpos));
}

void elan_eu3a05vid_device::device_reset()
{
	elan_eu3a05commonvid_device::device_reset();

	m_vidctrl = 0x00; // need to default to an 8x8 mode for Space Invaders test mode at least

	for (int i=0;i<4*2;i++)
		m_tile_scroll[i] = 0x00;

	m_tile_gfxbase_lo_data = 0x00;
	m_tile_gfxbase_hi_data = 0x00;

	m_sprite_gfxbase_lo_data = 0x00;
	m_sprite_gfxbase_hi_data = 0x00;

	for (int i=0;i<2;i++)
		m_splitpos[i] = 0x00;

}

void elan_eu3a05vid_device::set_is_sudoku()
{
	m_bytes_per_tile_entry = 2;
	m_vrambase = 0x200;
	m_spritebase = 0x1000;
}

void elan_eu3a05vid_device::set_is_pvmilfin()
{
	m_bytes_per_tile_entry = 4;
	m_vrambase = 0x200;
	m_spritebase = 0x1000; // not verified
}

uint8_t elan_eu3a05vid_device::read_spriteram(int offset)
{
	address_space& cpuspace = m_cpu->space(AS_PROGRAM);
	int realoffset = offset+m_spritebase;
	if (realoffset < 0x4000)
	{
		return cpuspace.read_byte(realoffset);
	}
	else
		return 0x00;
}


uint8_t elan_eu3a05vid_device::read_vram(int offset)
{
	address_space& cpuspace = m_cpu->space(AS_PROGRAM);
	int realoffset = offset+m_vrambase;
	if (realoffset < 0x4000)
	{
		return cpuspace.read_byte(realoffset);
	}
	else
		return 0x00;
}



/* (m_tile_gfxbase_lo_data | (m_tile_gfxbase_hi_data << 8)) * 0x100
   gives you the actual rom address, everything references the 3MByte - 4MByte region, like the banking so
   the system can probably have up to a 4MByte rom, all games we have so far just use the upper 1MByte of
   that space (Tetris seems to rely on mirroring? as it sets all addresses up for the lower 1MB instead)
*/

void elan_eu3a05vid_device::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, bitmap_ind8 &priority_bitmap, const rectangle &cliprect)
{
	address_space& fullbankspace = m_bank->space(AS_PROGRAM);

	/*
	    Sprites
	    FF yy xx AA XX YY aa bb

	    yy = y position
	    xx = x position
	    XX = texture x start
	    YY = texture y start
	    bb = sometimes set in invaders
	    AA = same as attr on tiles (cccc pppp) (c = colour / p = priority?)


	    aa = same as unk2 on tiles? ( --pp ---- )
	    p = page (some hardware types only? or selectable meaning)

	    FF = flags  ( e-dD fFsS )
	    e = enable
	    D = ZoomX to double size (boss explosions on Air Blaster)
	    d = ZoomY to double size (boss explosions on Air Blaster)
	    S = SizeX
	    s = SizeY
	    F = FlipX
	    f = FlipY (assumed, not seen)

	*/

//  later in list with AA == 0f   aa == 07 takes priority over earlier in the list with AA == 0f and aa ==  07
//  later in the list with AA == 0e and aa == 17 is under AA == 0f aa == and 07

	for (int i = 512-8; i >=0; i -= 8)
	{
		uint8_t x = read_spriteram(i + 2);
		uint8_t y = read_spriteram(i + 1);

		/*
		   Space Invaders draws the player base with this specific y value before the start of each life
		   and expects it to NOT wrap around.  There are no high priority tiles or anything else to hide
		   and it doesn't appear on real hardware.

		   it's possible sprites don't wrap around at all (but then you couldn't have smooth entry at the
		   top of the screen - there are no extra y co-ordinate bits.  However there would have been easier
		   ways to hide this tho as there are a bunch of unseen lines at the bottom of the screen anyway!

		   Air Blaster Joystick seems to indicate there is no sprite wrapping - sprites abruptly enter
		   the screen in pieces on real hardware.

		   needs further investigation.
		*/
		if (y==255)
			continue;

		uint8_t tex_x = read_spriteram(i + 4);
		uint8_t tex_y = read_spriteram(i + 5);

		uint8_t flags = read_spriteram(i + 0);
		uint8_t attr = read_spriteram(i + 3);
		uint8_t unk2 = read_spriteram(i + 6);

		const int doubleX = (flags & 0x10)>>4;
		const int doubleY = (flags & 0x20)>>5;

		int priority = (attr & 0x0f)^0xf;

		int colour = attr & 0xf0;

		// ? game select has this set to 0xff, but clearly doesn't want the palette to change!
		// while in Space Invaders this has to be palette for the UFO to be red.
		if (colour & 0x80)
			colour = 0;

		int transpen = 0;

		 /* HACK - how is this calculated
		   phoenix and the game select need it like this
		   it isn't a simple case of unk2 being transpen either because Qix has some elements set to 0x07
		   where the transpen needs to be 0x00 and Space Invaders has it set to 0x04
		   it could be a global register rather than something in the spritelist?
		*/
		if (((attr == 0xff) && (unk2 == 0xff)) || m_force_transpen_ff)
			transpen = 0xff;


		if (!(flags & 0x80))
			continue;

		int sizex = 8;
		int sizey = 8;

		if (flags & 0x01)
		{
			sizex = 16;
		}

		if (flags & 0x02)
		{
			sizey = 16;
		}

		int base = (m_sprite_gfxbase_lo_data | (m_sprite_gfxbase_hi_data << 8)) * 0x100;
		int page = (unk2 & 0x30) >> 4;

		// rad_sinv menu screen and phoenix don't agree with this, but carlecfg needs it
		if (m_use_spritepages)
		{
			base += 0x10000 * page;
		}

		if (doubleX)
			sizex = sizex * 2;

		if (doubleY)
			sizey = sizey * 2;

		for (int yy = 0; yy < sizey; yy++)
		{
			uint16_t* row;
			uint8_t* rowpri;

			if (flags & 0x08) // guess flipy
			{
				int drawypos = (y + (sizey - 1 - yy)) & 0xff;
				row = &bitmap.pix(drawypos);
				rowpri = &priority_bitmap.pix(drawypos);
			}
			else
			{
				int drawypos = (y + yy) & 0xff;
				row = &bitmap.pix(drawypos);
				rowpri = &priority_bitmap.pix(drawypos);
			}

			for (int xx = 0; xx < sizex; xx++)
			{
				int realaddr;

				if (!doubleX)
					realaddr = base + ((tex_x + xx) & 0xff);
				else
					realaddr = base + ((tex_x + (xx>>1)) & 0xff);

				if (!doubleY)
					realaddr += ((tex_y + yy) & 0xff) * 256;
				else
					realaddr += ((tex_y + (yy>>1)) & 0xff) * 256;

				uint8_t pix = fullbankspace.read_byte(realaddr);

				if (pix != transpen)
				{
					if (flags & 0x04) // flipx
					{
						int xdrawpos = (x + (sizex - 1 - xx)) & 0xff;

						if (rowpri[xdrawpos] > priority)
						{
							rowpri[xdrawpos] = priority;
							row[xdrawpos] = (pix + ((colour & 0x70) << 1)) & 0xff;
						}
					}
					else
					{
						int xdrawpos = (x + xx) & 0xff;

						if (rowpri[xdrawpos] > priority)
						{
							rowpri[xdrawpos] = priority;
							row[xdrawpos] = (pix + ((colour & 0x70) << 1)) & 0xff;
						}
					}
				}
			}
		}
	}
}


// a hacky mess for now
bool elan_eu3a05vid_device::get_tile_data(int base, int drawpri, int& tile, int &attr, int &unk2)
{
	tile = read_vram(base * m_bytes_per_tile_entry) + (read_vram((base * m_bytes_per_tile_entry) + 1) << 8);

	// these seem to be the basically the same as attr/unk2 in the sprites, which also make
	// very little sense.
	if (m_bytes_per_tile_entry == 4)
	{
		attr = read_vram((base * m_bytes_per_tile_entry) + 2);
		unk2 = read_vram((base * m_bytes_per_tile_entry) + 3);
	}
	else
	{
		attr = 0;
		unk2 = 0;
	}

	/* hack for phoenix title screens.. the have attr set to 0x3f which change the colour bank in ways we don't want
	   and also by our interpretation of 0x0f bits sets the tiles to priority over sprites (although in this case
	   that might tbe ok, because it looks like the sprites also have that set */
	if (unk2 == 0x07)
		attr = 0;

	int priority = attr & 0x0f;

	// likely wrong
	if ((drawpri == 0 && priority == 0x0f) || (drawpri == 1 && priority != 0x0f))
		return false;

	return true;
}


void elan_eu3a05vid_device::draw_tilemaps_tileline(int drawpri, int tile, int attr, int unk2, int tilexsize, int i, int xpos, uint16_t* row)
{
	address_space& fullbankspace = m_bank->space(AS_PROGRAM);
	int colour = attr & 0xf0;

	/* 'tiles' are organized / extracted from 'texture' lines that form a 'page' the length of the rom
	    each texture line in 8bpp mode is 256 bytes
	    each texture line in 4bpp mode is 128 bytes
	    in 8x8 mode these pages are 32 tiles wide
	    in 16x16 mode these pages are 16 tiles wide
	    tiles can start on any line

	    it is unclear what the behavior is if the tile starts at the far edge of a line (wrap around on line?)

	    this is eu3a05 specific, eu3a14 uses a more traditional approach
	*/

	const int tilespersrcline = 256 / tilexsize;
	const int tilespersrcline_mask = tilespersrcline - 1;

	tile = (tile & tilespersrcline_mask) + ((tile & ~tilespersrcline_mask) * tilexsize);

	if (!(m_vidctrl & 0x20)) // 8bpp
		tile <<= 1;

	if (!(m_vidctrl & 0x40)) // 8*8 tiles
		tile >>= 1;

	tile += ((m_tile_gfxbase_lo_data | m_tile_gfxbase_hi_data << 8) << 5);


	if (m_vidctrl & 0x20) // 4bpp
	{
		for (int xx = 0; xx < tilexsize; xx += 2)
		{
			int realaddr = ((tile + i * 16) << 3) + (xx >> 1);
			uint8_t pix = fullbankspace.read_byte(realaddr);

			int drawxpos;

			drawxpos = xpos + xx + 0;
			if ((drawxpos >= 0) && (drawxpos < 256))
				row[drawxpos] = ((pix & 0xf0) >> 4) + colour;

			drawxpos = xpos + xx + 1;
			if ((drawxpos >= 0) && (drawxpos < 256))
				row[drawxpos] = ((pix & 0x0f) >> 0) + colour;
		}
	}
	else // 8bpp
	{
		for (int xx = 0; xx < tilexsize; xx++)
		{
			int realaddr = ((tile + i * 32) << 3) + xx;
			uint8_t pix = fullbankspace.read_byte(realaddr);

			int drawxpos = xpos + xx;
			if ((drawxpos >= 0) && (drawxpos < 256))
				row[drawxpos] = (pix + ((colour & 0x70) << 1)) & 0xff;
		}
	}
}

uint16_t elan_eu3a05vid_device::get_tilemapindex_from_xy(uint16_t x, uint16_t y)
{
	// for mousetrap and candyland the pages in RAM (16x16 tile mode, 4 pages) are
	// top left
	// top right
	// bottom left
	// bottom right

	// for airblaster joystick 3d stagess the pages in RAM (8x8 tile mode, 2 pages) are
	// left
	// right

	// for airblaster joystick scrolling stages (8x8 tile mode, 2 pages)
	// top
	// bottom

	uint16_t tilemapsizey = 0;
	uint16_t tilemapsizex = 0;
	uint16_t pagesizey, pagesizex;

	pagesizey = 14; pagesizex = 16;

	switch (m_vidctrl & 0x03)
	{
	case 0x00: tilemapsizey = 14 * 2; tilemapsizex = 16 * 2; break; // double height & double width
	case 0x02: tilemapsizey = 14 * 2; tilemapsizex = 16; break; // double height
	case 0x01: tilemapsizey = 14; tilemapsizex = 16 * 2; break; // double width
	case 0x03: tilemapsizey = 14; tilemapsizex = 16;  break; // normal
	}

	if (!(m_vidctrl & 0x40)) // 16x16 tiles
	{
		pagesizey <<= 1;
		pagesizex <<= 1;
		tilemapsizey <<= 1;
		tilemapsizex <<= 1;
	}

	while (y >= tilemapsizey)
		y -= tilemapsizey;

	while (x >= tilemapsizex)
		x -= tilemapsizex;

	int index = 0;
	int page = 0;

	switch (m_vidctrl & 0x03)
	{
	case 0x00:  // double height & double width
		if (y < pagesizey)
		{
			if (x < pagesizex)
			{
				page = 0;
			}
			else
			{
				page = 1;
			}
		}
		else
		{
			if (x < pagesizex)
			{
				page = 2;
			}
			else
			{
				page = 3;
			}
		}
		break;

	case 0x02: // double height
		if (y < pagesizey)
		{
			page = 0;
		}
		else
		{
			page = 1;
		}
		break;

	case 0x01: // double width
		if (x < pagesizex)
		{
			page = 0;
		}
		else
		{
			page = 1;
		}
		break;

	case 0x03:
		page = 0;
		break;
	}

	while (y >= pagesizey)
		y -= pagesizey;

	while (x >= pagesizex)
		x -= pagesizex;

	index = x + y * pagesizex;
	index += page * pagesizey * pagesizex;

	return index;
}

void elan_eu3a05vid_device::draw_tilemaps(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect, int drawpri)
{
	/*
	    this doesn't handle 8x8 4bpp (not used by anything yet)
	*/
	//popmessage("%02x: %04x %04x %04x %04x", m_vidctrl, get_scroll(0), get_scroll(1), get_scroll(2), get_scroll(3));

	int scrolly = get_scroll(1);

	for (int screenline = 0; screenline < 224; screenline++)
	{
		int scrollx;
		int coursescrollx;
		int finescrollx;
		int realline = screenline + scrolly;

		// split can be probably configured in more ways than this
		// exact enable conditions unclear
		// this logic is needed for Air Blaster Joystick
		if (screenline < m_splitpos[0])
		{
			scrollx = get_scroll(0);
		}
		else if (screenline < m_splitpos[1])
		{
			scrollx = get_scroll(2);
		}
		else
		{
			scrollx = get_scroll(3);
		}

		// Candy Land and Mouse Trap in the TV Board Games units don't like the above logic, so force them to just use the
		// first scroll register for now, there must be more complex enable conditions for the above
		if (m_force_basic_scroll)
			scrollx = get_scroll(0);

		uint16_t* row = &bitmap.pix(screenline);

		int xtiles;
		int xtilesize;
		if (m_vidctrl & 0x40) // 16x16 tiles
		{
			xtiles = 16; // number of tilemap entries per row
			xtilesize = 16; // width of tile
			coursescrollx = scrollx >> 4;
			finescrollx = scrollx & 0xf;
		}
		else
		{
			xtiles = 32;
			xtilesize = 8;
			coursescrollx = scrollx >> 3;
			finescrollx = scrollx & 0x7;
		}

		for (int xtile = 0; xtile <= xtiles; xtile++)
		{

			int realxtile = xtile + coursescrollx;

			int tilemaprow;
			int tileline;

			if (m_vidctrl & 0x40) // 16x16 tiles
			{
				tilemaprow = realline >> 4;
				tileline = realline & 0xf;
			}
			else
			{
				tilemaprow = realline >> 3;
				tileline = realline & 0x7;
			}

			int tilemap_entry_index = get_tilemapindex_from_xy(realxtile, tilemaprow);

			int tile, attr, unk2;

			if (!get_tile_data(tilemap_entry_index, drawpri, tile, attr, unk2))
				continue;

			int xpos = xtile * xtilesize - finescrollx;
			draw_tilemaps_tileline(drawpri, tile, attr, unk2, xtilesize, tileline, xpos, row);

		}
	}
}

uint32_t elan_eu3a05vid_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	screen.priority().fill(0xff, cliprect);

	draw_tilemaps(screen,bitmap,cliprect,0); // 'low priority'
	draw_sprites(screen,bitmap,screen.priority(),cliprect);
	draw_tilemaps(screen,bitmap,cliprect,1); // 'high priority'

	return 0;
}

// Tile bases

void elan_eu3a05vid_device::tile_gfxbase_lo_w(uint8_t data)
{
	//logerror("%s: tile_gfxbase_lo_w (select GFX base lower) %02x\n", machine().describe_context(), data);
	m_tile_gfxbase_lo_data = data;
}

void elan_eu3a05vid_device::tile_gfxbase_hi_w(uint8_t data)
{
	//logerror("%s: tile_gfxbase_hi_w (select GFX base upper) %02x\n", machine().describe_context(), data);
	m_tile_gfxbase_hi_data = data;
}

uint8_t elan_eu3a05vid_device::tile_gfxbase_lo_r()
{
	//logerror("%s: tile_gfxbase_lo_r (GFX base lower)\n", machine().describe_context());
	return m_tile_gfxbase_lo_data;
}

uint8_t elan_eu3a05vid_device::tile_gfxbase_hi_r()
{
	//logerror("%s: tile_gfxbase_hi_r (GFX base upper)\n", machine().describe_context());
	return m_tile_gfxbase_hi_data;
}



// Sprite Tile bases

void elan_eu3a05vid_device::sprite_gfxbase_lo_w(uint8_t data)
{
	//logerror("%s: sprite_gfxbase_lo_w (select Sprite GFX base lower) %02x\n", machine().describe_context(), data);
	m_sprite_gfxbase_lo_data = data;
}

void elan_eu3a05vid_device::sprite_gfxbase_hi_w(uint8_t data)
{
	//logerror("%s: sprite_gfxbase_hi_w (select Sprite GFX base upper) %02x\n", machine().describe_context(), data);
	m_sprite_gfxbase_hi_data = data;
}

uint8_t elan_eu3a05vid_device::sprite_gfxbase_lo_r()
{
	//logerror("%s: sprite_gfxbase_lo_r (Sprite GFX base lower)\n", machine().describe_context());
	return m_sprite_gfxbase_lo_data;
}

uint8_t elan_eu3a05vid_device::sprite_gfxbase_hi_r()
{
	//logerror("%s: sprite_gfxbase_hi_r (Sprite GFX base upper)\n", machine().describe_context());
	return m_sprite_gfxbase_hi_data;
}



uint8_t elan_eu3a05vid_device::tile_scroll_r(offs_t offset)
{
	return m_tile_scroll[offset];
}

void elan_eu3a05vid_device::tile_scroll_w(offs_t offset, uint8_t data)
{
	//logerror("tile_scroll_w %02x %02x\n", offset, data);
	m_tile_scroll[offset] = data;
}

uint8_t elan_eu3a05vid_device::splitpos_r(offs_t offset)
{
	return m_splitpos[offset];
}

void elan_eu3a05vid_device::splitpos_w(offs_t offset, uint8_t data)
{
	m_splitpos[offset] = data;
}

uint16_t elan_eu3a05vid_device::get_scroll(int which)
{
	switch (which)
	{
	case 0x0: return (m_tile_scroll[1] << 8) | (m_tile_scroll[0]); // xscroll
	case 0x1: return (m_tile_scroll[3] << 8) | (m_tile_scroll[2]); // yscroll
	case 0x2: return (m_tile_scroll[5] << 8) | (m_tile_scroll[4]); // xsplit 1 scroll
	case 0x3: return (m_tile_scroll[7] << 8) | (m_tile_scroll[6]); // scplit 2 scroll
	}

	return 0x0000;
}

uint8_t elan_eu3a05vid_device::elan_eu3a05_vidctrl_r()
{
	return m_vidctrl;
}

void elan_eu3a05vid_device::elan_eu3a05_vidctrl_w(uint8_t data)
{
	logerror("%s: elan_eu3a05_vidctrl_w %02x (video control?)\n", machine().describe_context(), data);
	/*
	    c3  8bpp 16x16         1100 0011  abl logo
	    e3  4bpp 16x16         1110 0011
	    83  8bpp 8x8           1000 0011  air blaster logo
	    02  8bpp 8x8 (phoenix) 0000 0010  air blaster 2d normal
	    03  8bpp 8x8           0000 0011  air blaster 2d bosses
	    00                     0000 0000  air blaster 3d stages

	    ?tb- --wh

	    ? = unknown
	    t = tile size (1 = 16x16, 0 = 8x8)
	    b = bpp (0 = 8bpp, 1 = 4bpp)
	    - = haven't seen used
	    h = tilemap height? (0 = double height)
	    w = tilemap width? (0 = double width)

	    space invaders test mode doesn't initialize this

	*/
	m_vidctrl = data;
}

uint8_t elan_eu3a05vid_device::read_unmapped(offs_t offset)
{
	logerror("%s: elan_eu3a05vid_device::read_unmapped (offset %02x)\n", machine().describe_context(), offset);
	return 0x00;
}

void elan_eu3a05vid_device::write_unmapped(offs_t offset, uint8_t data)
{
	logerror("%s: elan_eu3a05vid_device::write_unmapped (offset %02x) (data %02x)\n", machine().describe_context(), offset, data);
}
