// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*
Taito pc090oj
-------

        Information from Raine (todo: reformat)

        OBJECT RAM
        ----------

        - 8 bytes/sprite
        - 256 sprites (0x800 bytes)
        - First sprite has *highest* priority

        -----+--------+-------------------------
        Byte | Bit(s) | Use
        -----+76543210+-------------------------
          0  |.x......| Flip Y Axis
          0  |x.......| Flip X Axis
          1  |....xxxx| Colour Bank
          2  |.......x| Sprite Y
          3  |xxxxxxxx| Sprite Y
          4  |...xxxxx| Sprite Tile
          5  |xxxxxxxx| Sprite Tile
          6  |.......x| Sprite X
          7  |xxxxxxxx| Sprite X
        -----+--------+-------------------------

        SPRITE CONTROL
        --------------

        - Maze of Flott [603D MASK] 201C 200B 200F
        - Earth Joker 001C
        - Cadash 0011 0013 0010 0000

        -----+--------+-------------------------
        Byte | Bit(s) | Use
        -----+76543210+-------------------------
          0  |.......x| ?
          0  |......x.| Write Acknowledge?
          0  |..xxxx..| Colour Bank Offset
          0  |xx......| Unused
          1  |...xxxxx| Unused
          1  |..x.....| BG1:Sprite Priority
          1  |.x......| Priority?
          1  |x.......| Unused
        -----+--------+-------------------------

        OLD SPRITE CONTROL (RASTAN TYPE)
        --------------------------------

        -----+--------+-------------------------
        Byte | Bit(s) | Use
        -----+76543210+-------------------------
          1  |.......x| BG1:Sprite Priority?
          1  |......x.| Write Acknowledge?
          1  |xxx.....| Colour Bank Offset
        -----+--------+-------------------------
*/

#include "emu.h"
#include "pc090oj.h"
#include "screen.h"

static constexpr u32 PC090OJ_RAM_SIZE        = 0x4000;
static constexpr u32 PC090OJ_ACTIVE_RAM_SIZE = 0x800;


/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/


DEFINE_DEVICE_TYPE(PC090OJ, pc090oj_device, "pc090oj", "Taito PC090OJ")

pc090oj_device::pc090oj_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, PC090OJ, tag, owner, clock)
	, device_gfx_interface(mconfig, *this, nullptr)
	, m_colpri_cb(*this)
	, m_ctrl(0)
	, m_sprite_ctrl(0)
	, m_ram(nullptr)
	, m_ram_buffered(nullptr)
	, m_x_offset(0)
	, m_y_offset(0)
	, m_use_buffer(false)
{
}

/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

GFXDECODE_MEMBER(pc090oj_device::gfxinfo)
	GFXDECODE_DEVICE(DEVICE_SELF, 0, gfx_16x16x4_packed_msb, 0, 1)
GFXDECODE_END

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pc090oj_device::device_start()
{
	decode_gfx(gfxinfo);
	gfx(0)->set_colors(palette().entries() / 16);

	m_colpri_cb.resolve();

	m_ram = make_unique_clear<u16[]>(PC090OJ_RAM_SIZE / 2);
	m_ram_buffered = make_unique_clear<u16[]>(PC090OJ_RAM_SIZE / 2);

	save_pointer(NAME(m_ram), PC090OJ_RAM_SIZE / 2);
	save_pointer(NAME(m_ram_buffered), PC090OJ_RAM_SIZE / 2);
	save_item(NAME(m_ctrl));
	save_item(NAME(m_sprite_ctrl));  // should this be set in intf?!?
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pc090oj_device::device_reset()
{
	m_ctrl = 0;
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

void pc090oj_device::sprite_ctrl_w(u16 data)
{
	m_sprite_ctrl = data;
}

u16 pc090oj_device::word_r(offs_t offset)
{
	return m_ram[offset];
}

void pc090oj_device::word_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_ram[offset]);

	/* If we're not buffering sprite ram, write it straight through... */
	if (!m_use_buffer)
		m_ram_buffered[offset] = m_ram[offset];

	if (offset == 0xdff)
	{
		/* Bit 0 is flip control, others seem unused */
		m_ctrl = data;

#if 0
	popmessage("pc090oj ctrl = %4x", data);
#endif
	}
}

void pc090oj_device::eof_callback()
{
	if (m_use_buffer)
	{
		for (int i = 0; i < PC090OJ_ACTIVE_RAM_SIZE / 2; i++)
			m_ram_buffered[i] = m_ram[i];
	}
}


void pc090oj_device::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const bool priority = !m_colpri_cb.isnull();
	u32 sprite_colbank = 0, pri_mask = 0;
	if (priority)
		m_colpri_cb(sprite_colbank, pri_mask, m_sprite_ctrl);

	int start, end, inc;
	if (priority) { start =                                 0; end = PC090OJ_ACTIVE_RAM_SIZE / 2; inc =  4; }
	else          { start = (PC090OJ_ACTIVE_RAM_SIZE / 2) - 4; end =                          -4; inc = -4; }

	for (int offs = start; offs != end; offs += inc)
	{
		const u16 data = m_ram_buffered[offs];
		int flipy = (data & 0x8000) >> 15;
		int flipx = (data & 0x4000) >> 14;
		const u32 color = (data & 0x000f) | sprite_colbank;

		const u32 code = m_ram_buffered[offs + 2] & 0x1fff;
		int x = m_ram_buffered[offs + 3] & 0x1ff;   /* mask verified with Rainbowe board */
		int y = m_ram_buffered[offs + 1] & 0x1ff;   /* mask verified with Rainbowe board */

		/* treat coords as signed */
		if (x > 0x140) x -= 0x200;
		if (y > 0x140) y -= 0x200;

		if (!(m_ctrl & 1))   /* sprites flipscreen */
		{
			x = 320 - x - 16;
			y = 256 - y - 16;
			flipx = !flipx;
			flipy = !flipy;
		}

		x += m_x_offset;
		y += m_y_offset;

		if (priority)
		{
			gfx(0)->prio_transpen(bitmap,cliprect,
					code,
					color,
					flipx,flipy,
					x,y,
					screen.priority(),pri_mask,
					0);
		}
		else
		{
			gfx(0)->transpen(bitmap,cliprect,
					code,
					color,
					flipx,flipy,
					x,y,
					0);
		}
	}
}
