// license:BSD-3-Clause
// copyright-holders:David Haywood, Luca Elia
/***********************************************************************

    Kaneko Pandora Sprite Chip
    GFX processor - PX79C480FP-3 (KANEKO, Pandora-Chip)

    This emulates the Kaneko Pandora Sprite Chip
    which is found on several Kaneko boards.

    there several bootleg variants of this chip,
    these are emulated in kan_panb.c instead.

    Original Games using this Chip

    Snow Bros
    Air Buster
    DJ Boy
    Heavy Unit
    Sand Scorpion
    Gals Panic (1st release)

    The SemiCom games are also using this because
    their bootleg chip appears to function in an
    identical way.

    Rendering appears to be done to a framebuffer
    and the video system can be instructed not to
    clear this, allowing for 'sprite trail' effects
    as used by Air Buster.

    The chip appears to be an 8-bit chip, and
    when used on 16-bit CPUs only the MSB or LSB
    of the data lines are connected.  Address Lines
    also appear to be swapped around on one of the
    hookups.

    to use this in a driver you must hook functions to
    VIDEO_UPDATE  (copies framebuffer to screen)
    and
    VIDEO_EOF  (renders the sprites to the framebuffer)

    also, you have to add the correspondent device in MACHINE_DRIVER

    spriteram should be accessed only with the
    pandora_spriteram_r / pandora_spriteram_w or
    pandora_spriteram_LSB_r / pandora_spriteram_LSB_w
    handlers, depending on the CPU being used with it.

***********************************************************************/

#include "emu.h"
#include "video/kan_pand.h"

const device_type KANEKO_PANDORA = &device_creator<kaneko_pandora_device>;

kaneko_pandora_device::kaneko_pandora_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, KANEKO_PANDORA, "Kaneko PANDORA GFX", tag, owner, clock, "kaneko_pandora", __FILE__),
		device_video_interface(mconfig, *this),
		m_gfx_region(0),
		m_xoffset(0),
		m_yoffset(0),
		m_gfxdecode(*this)
{
}

//-------------------------------------------------
//  static_set_gfxdecode_tag: Set the tag of the
//  gfx decoder
//-------------------------------------------------

void kaneko_pandora_device::static_set_gfxdecode_tag(device_t &device, const char *tag)
{
	downcast<kaneko_pandora_device &>(device).m_gfxdecode.set_tag(tag);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void kaneko_pandora_device::device_start()
{
	m_bg_pen = 0;

	m_spriteram = std::make_unique<UINT8[]>(0x1000);

	m_sprites_bitmap = std::make_unique<bitmap_ind16>(m_screen->width(), m_screen->height());

	save_item(NAME(m_clear_bitmap));
	save_item(NAME(m_bg_pen));
	save_pointer(NAME(m_spriteram.get()), 0x1000);
	save_item(NAME(*m_sprites_bitmap));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void kaneko_pandora_device::device_reset()
{
	memset(m_spriteram.get(), 0x00, 0x1000);

	m_clear_bitmap = 1;
}


/*****************************************************************************
    IMPLEMENTATION
*****************************************************************************/

void kaneko_pandora_device::set_bg_pen( int pen )
{
	m_bg_pen = pen;
}

void kaneko_pandora_device::set_clear_bitmap( int clear )
{
	m_clear_bitmap = clear;
}

void kaneko_pandora_device::update( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	if (!m_sprites_bitmap)
	{
		printf("ERROR: pandora_update with no pandora_sprites_bitmap\n");
		return;
	}

	copybitmap_trans(bitmap, *m_sprites_bitmap, 0, 0, 0, 0, cliprect, 0);
}


void kaneko_pandora_device::draw( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int sx = 0, sy = 0, x = 0, y = 0, offs;


	/*
	 * Sprite Tile Format
	 * ------------------
	 *
	 * Byte | Bit(s)   | Use
	 * -----+-76543210-+----------------
	 *  0-2 | -------- | unused
	 *  3   | xxxx.... | Palette Bank
	 *  3   | .......x | XPos - Sign Bit
	 *  3   | ......x. | YPos - Sign Bit
	 *  3   | .....x.. | Use Relative offsets
	 *  4   | xxxxxxxx | XPos
	 *  5   | xxxxxxxx | YPos
	 *  6   | xxxxxxxx | Sprite Number (low 8 bits)
	 *  7   | ....xxxx | Sprite Number (high 4 bits)
	 *  7   | x....... | Flip Sprite Y-Axis
	 *  7   | .x...... | Flip Sprite X-Axis
	 */

	for (offs = 0; offs < 0x1000; offs += 8)
	{
		int dx = m_spriteram[offs + 4];
		int dy = m_spriteram[offs + 5];
		int tilecolour = m_spriteram[offs + 3];
		int attr = m_spriteram[offs + 7];
		int flipx =   attr & 0x80;
		int flipy =  (attr & 0x40) << 1;
		int tile  = ((attr & 0x3f) << 8) + (m_spriteram[offs + 6] & 0xff);

		if (tilecolour & 1)
			dx |= 0x100;
		if (tilecolour & 2)
			dy |= 0x100;

		if (tilecolour & 4)
		{
			x += dx;
			y += dy;
		}
		else
		{
			x = dx;
			y = dy;
		}

		if (machine().driver_data()->flip_screen())
		{
			sx = 240 - x;
			sy = 240 - y;
			flipx = !flipx;
			flipy = !flipy;
		}
		else
		{
			sx = x;
			sy = y;
		}

		/* global offset */
		sx += m_xoffset;
		sy += m_yoffset;

		sx &= 0x1ff;
		sy &= 0x1ff;

		if (sx & 0x100)
			sx -= 0x200;
		if (sy & 0x100)
			sy -= 0x200;

		m_gfxdecode->gfx(m_gfx_region)->transpen(bitmap,cliprect,
				tile,
				(tilecolour & 0xf0) >> 4,
				flipx, flipy,
				sx,sy,0);
	}
}

void kaneko_pandora_device::eof( )
{
	assert(m_spriteram != nullptr);

	// the games can disable the clearing of the sprite bitmap, to leave sprite trails
	if (m_clear_bitmap)
		m_sprites_bitmap->fill(m_bg_pen, m_screen->visible_area());

	kaneko_pandora_device::draw(*m_sprites_bitmap, m_screen->visible_area());
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

WRITE8_MEMBER ( kaneko_pandora_device::spriteram_w )
{
	// it's either hooked up oddly on this, or on the 16-bit games
	// either way, we swap the address lines so that the spriteram is in the same format
	offset = BITSWAP16(offset,  15,14,13,12, 11,   7,6,5,4,3,2,1,0,   10,9,8  );

	if (!m_spriteram)
	{
		printf("ERROR: spriteram_w with no m__spriteram\n");
		return;
	}

	if (offset >= 0x1000)
	{
		logerror("spriteram_w write past spriteram, offset %04x %02x\n", offset, data);
		return;
	}

	m_spriteram[offset] = data;
}

READ8_MEMBER( kaneko_pandora_device::spriteram_r )
{
	// it's either hooked up oddly on this, or on the 16-bit games
	// either way, we swap the address lines so that the spriteram is in the same format
	offset = BITSWAP16(offset,  15,14,13,12, 11,  7,6,5,4,3,2,1,0,  10,9,8  );

	if (!m_spriteram)
	{
		printf("ERROR: spriteram_r with no m_spriteram\n");
		return 0x00;
	}

	if (offset >= 0x1000)
	{
		logerror("spriteram_r read past spriteram, offset %04x\n", offset);
		return 0x00;
	}
	return m_spriteram[offset];
}

/* I don't know if this MSB/LSB mirroring is correct, or if there is twice as much ram, with half of it unused */
WRITE16_MEMBER( kaneko_pandora_device::spriteram_LSB_w )
{
	if (!m_spriteram)
	{
		printf("ERROR: m_spriteram_LSB_w with no m_spriteram\n");
		return;
	}

	if (ACCESSING_BITS_8_15)
	{
		m_spriteram[offset] = (data >> 8) & 0xff;
	}

	if (ACCESSING_BITS_0_7)
	{
		m_spriteram[offset] = data & 0xff;
	}
}

READ16_MEMBER( kaneko_pandora_device::spriteram_LSB_r )
{
	if (!m_spriteram)
	{
		printf("ERROR: spriteram_LSB_r with no m_spriteram\n");
		return 0x0000;
	}

	return m_spriteram[offset] | (m_spriteram[offset] << 8);
}
