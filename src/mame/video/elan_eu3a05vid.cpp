// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "elan_eu3a05vid.h"

DEFINE_DEVICE_TYPE(ELAN_EU3A05_VID, elan_eu3a05vid_device, "elan_eu3a05vid", "Elan EU3A05 Video")

// tilemaps start at 0x0600 in mainram, sprites at 0x3e00, unlike eu3a14 these could be fixed addresses

elan_eu3a05vid_device::elan_eu3a05vid_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: elan_eu3a05commonvid_device(mconfig, ELAN_EU3A05_VID, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_bank(*this, finder_base::DUMMY_TAG),
	m_space_config("regs", ENDIANNESS_NATIVE, 8, 5, 0, address_map_constructor(FUNC(elan_eu3a05vid_device::map), this))
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

uint8_t elan_eu3a05vid_device::read_spriteram(int offset)
{
	address_space& cpuspace = m_cpu->space(AS_PROGRAM);
	int realoffset = offset+0x3e00;
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
	int realoffset = offset+0x0600;
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

void elan_eu3a05vid_device::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	address_space& fullbankspace = m_bank->space(AS_PROGRAM);

	/*
	    Sprites
	    FF yy xx AA XX YY aa bb

	    yy = y position
	    xx = x position
	    XX = texture x start
	    YY = texture y start
	    aa = same as unk2 on tiles
	    bb = sometimes set in invaders
	    AA = same as attr on tiles (colour / priority?)

	    FF = flags  ( e-dD fFsS )
	    e = enable
		D = ZoomX to double size (boss explosions on Air Blaster)
		d = ZoomY to double size (boss explosions on Air Blaster)
	    S = SizeX
	    s = SizeY
	    F = FlipX
	    f = FlipY (assumed, not seen)

	*/

	for (int i = 0; i < 512; i += 8)
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

		//int priority = attr & 0x0f;
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
		if ((attr == 0xff) && (unk2 == 0xff))
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

		if (doubleX)
			sizex = sizex * 2;

		if (doubleY)
			sizey = sizey * 2;

		for (int yy = 0; yy < sizey; yy++)
		{
			uint16_t* row;

			if (flags & 0x08) // guess flipy
			{
				row = &bitmap.pix16((y + (sizey - 1 - yy)) & 0xff);
			}
			else
			{
				row = &bitmap.pix16((y + yy) & 0xff);
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
						row[(x + (sizex - 1 - xx)) & 0xff] = (pix + ((colour & 0x70) << 1)) & 0xff;
					}
					else
					{
						row[(x + xx) & 0xff] = (pix + ((colour & 0x70) << 1)) & 0xff;
					}
				}
			}
		}
	}
}


// a hacky mess for now
bool elan_eu3a05vid_device::get_tile_data(int base, int drawpri, int& tile, int &attr, int &unk2)
{
	tile = read_vram(base * 4) + (read_vram((base * 4) + 1) << 8);

	// these seem to be the basically the same as attr/unk2 in the sprites, which also make
	// very little sense.
	attr = read_vram((base * 4) + 2);
	unk2 = read_vram((base * 4) + 3);

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
void elan_eu3a05vid_device::draw_tilemaps(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect, int drawpri)
{
	/*
		this doesn't handle 8x8 4bpp (not used by anything yet)
	*/

	int scroll = get_scroll(1);
	address_space& fullbankspace = m_bank->space(AS_PROGRAM);

	// Phoenix scrolling actually skips a pixel, jumping from 0x001 to 0x1bf, scroll 0x000 isn't used, maybe it has other meanings?

	int totalyrow;
	int totalxcol;
	int mapyrowsbase;
	int tileysize;
	int tilexsize;
	int startrow;

	if (m_vidctrl & 0x40) // 16x16 tiles
	{
		totalyrow = 16;
		totalxcol = 16;
		mapyrowsbase = 14;
		tileysize = 16;
		tilexsize = 16;
		startrow = (scroll >> 4) & 0x1f;
	}
	else
	{
		totalyrow = 32;
		totalxcol = 32;
		mapyrowsbase = 28;
		tileysize = 8;
		tilexsize = 8;
		startrow = (scroll >> 3) & 0x3f;
	}

	for (int y = 0; y < totalyrow; y++)
	{
		for (int x = 0; x < totalxcol * 2; x++)
		{
			int realstartrow = (startrow + y);

			int yrows;

			if (m_vidctrl & 0x01)
				yrows = mapyrowsbase;
			else
				yrows = mapyrowsbase * 2;

			if (realstartrow >= yrows)
				realstartrow -= yrows;

			// in double width & double height mode the page addressing needs adjusting
			if (!(m_vidctrl & 0x02))
			{
				if (!(m_vidctrl & 0x01))
				{
					if (realstartrow >= (yrows / 2))
					{
						realstartrow += yrows / 2;
					}
				}
			}

			for (int i = 0; i < tileysize; i++)
			{
				int drawline = (y * tileysize) + i;
				drawline -= scroll & (tileysize - 1);

				if ((drawline >= 0) && (drawline < 256))
				{
					int scrollx;

					// split can be probably configured in more ways than this
					// exact enable conditions unclear
					if (drawline < m_splitpos[0])
					{
						scrollx = get_scroll(0);
					}
					else if (drawline < m_splitpos[1])
					{
						scrollx = get_scroll(2);
					}
					else
					{
						scrollx = get_scroll(3);
					}

					int base;

					if (m_vidctrl & 0x40) // 16x16 tiles
					{
						base = (((realstartrow + y) & 0x3f) * 8) + x;
					}
					else
					{
						base = (((realstartrow) & 0x7f) * totalxcol) + (x & (totalxcol - 1));
					}

					if (!(m_vidctrl & 0x02))
					{
						if (x & totalxcol)
						{
							base += totalxcol * mapyrowsbase;
						}
					}

					int tile, attr, unk2;

					if (!get_tile_data(base, drawpri, tile, attr, unk2))
						continue;

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

					uint16_t* row = &bitmap.pix16(drawline);

					if (m_vidctrl & 0x20) // 4bpp
					{
						for (int xx = 0; xx < tilexsize; xx += 2)
						{
							int realaddr = ((tile + i * 16) << 3) + (xx >> 1);
							uint8_t pix = fullbankspace.read_byte(realaddr);

							int drawxpos;

							drawxpos = x * tilexsize + xx + 0 - scrollx;
							drawxpos &= 0x1ff;
							if ((drawxpos >= 0) && (drawxpos < 256))
								row[drawxpos] = ((pix & 0xf0) >> 4) + colour;

							drawxpos = x * tilexsize + xx + 1 - scrollx;
							drawxpos &= 0x1ff;
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

							int drawxpos = x * tilexsize + xx - scrollx;
							drawxpos &= 0x1ff;
							if ((drawxpos >= 0) && (drawxpos < 256))
								row[drawxpos] = (pix + ((colour & 0x70) << 1)) & 0xff;
						}
					}
				}
			}
		}
	}
}

uint32_t elan_eu3a05vid_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	draw_tilemaps(screen,bitmap,cliprect,0);
	draw_sprites(screen,bitmap,cliprect);
	draw_tilemaps(screen,bitmap,cliprect,1);

	return 0;
}

// Tile bases

WRITE8_MEMBER(elan_eu3a05vid_device::tile_gfxbase_lo_w)
{
	//logerror("%s: tile_gfxbase_lo_w (select GFX base lower) %02x\n", machine().describe_context(), data);
	m_tile_gfxbase_lo_data = data;
}

WRITE8_MEMBER(elan_eu3a05vid_device::tile_gfxbase_hi_w)
{
	//logerror("%s: tile_gfxbase_hi_w (select GFX base upper) %02x\n", machine().describe_context(), data);
	m_tile_gfxbase_hi_data = data;
}

READ8_MEMBER(elan_eu3a05vid_device::tile_gfxbase_lo_r)
{
	//logerror("%s: tile_gfxbase_lo_r (GFX base lower)\n", machine().describe_context());
	return m_tile_gfxbase_lo_data;
}

READ8_MEMBER(elan_eu3a05vid_device::tile_gfxbase_hi_r)
{
	//logerror("%s: tile_gfxbase_hi_r (GFX base upper)\n", machine().describe_context());
	return m_tile_gfxbase_hi_data;
}



// Sprite Tile bases

WRITE8_MEMBER(elan_eu3a05vid_device::sprite_gfxbase_lo_w)
{
	//logerror("%s: sprite_gfxbase_lo_w (select Sprite GFX base lower) %02x\n", machine().describe_context(), data);
	m_sprite_gfxbase_lo_data = data;
}

WRITE8_MEMBER(elan_eu3a05vid_device::sprite_gfxbase_hi_w)
{
	//logerror("%s: sprite_gfxbase_hi_w (select Sprite GFX base upper) %02x\n", machine().describe_context(), data);
	m_sprite_gfxbase_hi_data = data;
}

READ8_MEMBER(elan_eu3a05vid_device::sprite_gfxbase_lo_r)
{
	//logerror("%s: sprite_gfxbase_lo_r (Sprite GFX base lower)\n", machine().describe_context());
	return m_sprite_gfxbase_lo_data;
}

READ8_MEMBER(elan_eu3a05vid_device::sprite_gfxbase_hi_r)
{
	//logerror("%s: sprite_gfxbase_hi_r (Sprite GFX base upper)\n", machine().describe_context());
	return m_sprite_gfxbase_hi_data;
}



READ8_MEMBER(elan_eu3a05vid_device::tile_scroll_r)
{
	return m_tile_scroll[offset];
}

WRITE8_MEMBER(elan_eu3a05vid_device::tile_scroll_w)
{
	m_tile_scroll[offset] = data;
}

READ8_MEMBER(elan_eu3a05vid_device::splitpos_r)
{
	return m_splitpos[offset];
}

WRITE8_MEMBER(elan_eu3a05vid_device::splitpos_w)
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

READ8_MEMBER(elan_eu3a05vid_device::elan_eu3a05_vidctrl_r)
{
	return m_vidctrl;
}

WRITE8_MEMBER(elan_eu3a05vid_device::elan_eu3a05_vidctrl_w)
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

READ8_MEMBER(elan_eu3a05vid_device::read_unmapped)
{
	logerror("%s: elan_eu3a05vid_device::read_unmapped (offset %02x)\n", machine().describe_context(), offset);
	return 0x00;
}

WRITE8_MEMBER(elan_eu3a05vid_device::write_unmapped)
{
	logerror("%s: elan_eu3a05vid_device::write_unmapped (offset %02x) (data %02x)\n", machine().describe_context(), offset, data);
}
