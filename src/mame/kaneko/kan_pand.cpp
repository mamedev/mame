// license:BSD-3-Clause
// copyright-holders:David Haywood, Luca Elia
/***********************************************************************

    Kaneko Pandora Sprite Chip
    GFX processor - PX79C480FP-3 (KANEKO, Pandora-Chip)

    This emulates the Kaneko Pandora Sprite Chip
    which is found on several Kaneko boards.

    there several bootleg variants of this chip,
    these are emulated in kaneko/snowbros_v.cpp instead.

    Original Games using this Chip

    Snow Bros (kaneko/snowbros.cpp)
    Air Buster (kaneko/airbustr.cpp)
    DJ Boy (kaneko/djboy.cpp)
    Heavy Unit (kaneko/hvyunit.cpp)
    Sand Scorpion (kaneko/sandscrp.cpp)
    Gals Panic (1st release) (kaneko/galpanic.cpp)

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
    screen_update (copies framebuffer to screen)
    and
    screen_vblank (renders the sprites to the framebuffer)

    also, you have to add the correspondent device in machine_config

    spriteram should be accessed only with the
    spriteram_r / spriteram_w or
    spriteram_lsb_r / spriteram_lsb_w
    handlers, depending on the CPU being used with it.

***********************************************************************/

#include "emu.h"
#include "kan_pand.h"
#include "screen.h"

DEFINE_DEVICE_TYPE(KANEKO_PANDORA, kaneko_pandora_device, "kaneko_pandora", "Kaneko PANDORA Sprite Generator")

kaneko_pandora_device::kaneko_pandora_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, KANEKO_PANDORA, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, device_gfx_interface(mconfig, *this)
	, m_buffer(0)
	, m_clear_bitmap(false)
	, m_bg_pen(0)
	, m_xoffset(0)
	, m_yoffset(0)
	, m_flip_screen(false)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void kaneko_pandora_device::device_start()
{
	m_bg_pen = 0;
	m_flip_screen = false;

	m_spriteram = std::make_unique<u8[]>(0x1000);

	// 4 64x4 DRAMs - 256x256 8 bit, double buffered
	for (auto &elem : m_sprites_bitmap)
	{
		elem.allocate(256, 256);
		elem.fill(m_bg_pen);
	}

	save_item(NAME(m_buffer));
	save_item(NAME(m_clear_bitmap));
	save_item(NAME(m_bg_pen));
	save_item(NAME(m_flip_screen));
	save_pointer(NAME(m_spriteram), 0x1000);
	save_item(NAME(m_sprites_bitmap[0]));
	save_item(NAME(m_sprites_bitmap[1]));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void kaneko_pandora_device::device_reset()
{
	memset(m_spriteram.get(), 0x00, 0x1000);

	m_clear_bitmap = true;
}


/*****************************************************************************
    IMPLEMENTATION
*****************************************************************************/

void kaneko_pandora_device::set_bg_pen(u16 pen)
{
	m_bg_pen = pen;
}

void kaneko_pandora_device::set_clear_bitmap(int clear)
{
	m_clear_bitmap = clear != 0;
}

void kaneko_pandora_device::update(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap_trans(bitmap, m_sprites_bitmap[m_buffer], 0, 0, 0, 0, cliprect, 0);
}


void kaneko_pandora_device::draw(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x = 0, y = 0;

	/*
	 * Sprite Tile Format
	 * ------------------
	 *
	 * Byte | Bit(s)   | Use
	 * -----+-76543210-+----------------
	 *  0-2 | -------- | unused
	 *  3   | xxxx.... | Palette Bank
	 *  3   | .....x.. | Use Relative offsets
	 *  3   | ......x. | YPos - Sign Bit
	 *  3   | .......x | XPos - Sign Bit
	 *  4   | xxxxxxxx | XPos
	 *  5   | xxxxxxxx | YPos
	 *  6   | xxxxxxxx | Sprite Number (low 8 bits)
	 *  7   | x....... | Flip Sprite Y-Axis
	 *  7   | .x...... | Flip Sprite X-Axis
	 *  7   | ..xxxxxx | Sprite Number (high 6 bits)
	 */

	for (int offs = 0; offs < 0x1000; offs += 8)
	{
		int dx              = m_spriteram[offs + 4];
		int dy              = m_spriteram[offs + 5];
		const u8 tilecolour = m_spriteram[offs + 3];
		const u8 attr       = m_spriteram[offs + 7];
		bool flipx          = BIT(attr, 7);
		bool flipy          = BIT(attr, 6);
		const u32 tile      = ((attr & 0x3f) << 8) + (m_spriteram[offs + 6] & 0xff);

		if (BIT(tilecolour, 0))
			dx |= 0x100;
		if (BIT(tilecolour, 1))
			dy |= 0x100;

		if (BIT(tilecolour, 2))
		{
			x += dx;
			y += dy;
		}
		else
		{
			x = dx;
			y = dy;
		}

		int sx = x, sy = y;
		if (m_flip_screen)
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		/* global offset */
		sx += m_xoffset;
		sy += m_yoffset;

		sx = util::sext(sx & 0x1ff, 9);
		sy = util::sext(sy & 0x1ff, 9);

		gfx(0)->transpen(bitmap,cliprect,
				tile,
				(tilecolour & 0xf0) >> 4,
				flipx, flipy,
				sx, sy, 0);
	}
}

void kaneko_pandora_device::eof()
{
	m_buffer ^= 1;
	// the games can disable the clearing of the sprite bitmap, to leave sprite trails
	if (m_clear_bitmap)
		m_sprites_bitmap[m_buffer].fill(m_bg_pen);

	draw(m_sprites_bitmap[m_buffer], m_sprites_bitmap[m_buffer].cliprect());
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

void kaneko_pandora_device::spriteram_w(offs_t offset, u8 data)
{
	// it's either hooked up oddly on this, or on the 16-bit games
	// either way, we swap the address lines so that the spriteram is in the same format
	offset = bitswap<16>(offset,  15,14,13,12, 11,   7,6,5,4,3,2,1,0,   10,9,8);

	if (offset >= 0x1000)
	{
		logerror("%s: spriteram_w write past spriteram, offset %04x %02x\n", machine().describe_context(), offset, data);
		return;
	}

	m_spriteram[offset] = data;
}

u8 kaneko_pandora_device::spriteram_r(offs_t offset)
{
	// it's either hooked up oddly on this, or on the 16-bit games
	// either way, we swap the address lines so that the spriteram is in the same format
	offset = bitswap<16>(offset,  15,14,13,12, 11,  7,6,5,4,3,2,1,0,  10,9,8);

	if (offset >= 0x1000)
	{
		if (!machine().side_effects_disabled())
			logerror("%s: spriteram_r read past spriteram, offset %04x\n", machine().describe_context(), offset);
		return 0x00;
	}
	return m_spriteram[offset];
}

/* I don't know if this MSB/LSB mirroring is correct, or if there is twice as much ram, with half of it unused */
void kaneko_pandora_device::spriteram_lsb_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_8_15)
	{
		m_spriteram[offset] = (data >> 8) & 0xff;
	}

	if (ACCESSING_BITS_0_7)
	{
		m_spriteram[offset] = data & 0xff;
	}
}

u16 kaneko_pandora_device::spriteram_lsb_r(offs_t offset)
{
	return m_spriteram[offset] | (m_spriteram[offset] << 8);
}
