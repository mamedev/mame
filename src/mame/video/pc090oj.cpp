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

#define PC090OJ_RAM_SIZE 0x4000
#define PC090OJ_ACTIVE_RAM_SIZE 0x800



/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/


const device_type PC090OJ = &device_creator<pc090oj_device>;

pc090oj_device::pc090oj_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, PC090OJ, "Taito PC090OJ", tag, owner, clock, "pc090oj", __FILE__),
	m_ctrl(0),
	m_sprite_ctrl(0),
	m_ram(nullptr),
	m_ram_buffered(nullptr),
	m_gfxnum(0),
	m_x_offset(0),
	m_y_offset(0),
	m_use_buffer(0),
	m_gfxdecode(*this),
	m_palette(*this)
{
}

//-------------------------------------------------
//  static_set_gfxdecode_tag: Set the tag of the
//  gfx decoder
//-------------------------------------------------

void pc090oj_device::static_set_gfxdecode_tag(device_t &device, std::string tag)
{
	downcast<pc090oj_device &>(device).m_gfxdecode.set_tag(tag);
}

//-------------------------------------------------
//  static_set_palette_tag: Set the tag of the
//  palette device
//-------------------------------------------------

void pc090oj_device::static_set_palette_tag(device_t &device, std::string tag)
{
	downcast<pc090oj_device &>(device).m_palette.set_tag(tag);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pc090oj_device::device_start()
{
	m_ram = make_unique_clear<UINT16[]>(PC090OJ_RAM_SIZE / 2);
	m_ram_buffered = make_unique_clear<UINT16[]>(PC090OJ_RAM_SIZE / 2);

	save_pointer(NAME(m_ram.get()), PC090OJ_RAM_SIZE / 2);
	save_pointer(NAME(m_ram_buffered.get()), PC090OJ_RAM_SIZE / 2);
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

void pc090oj_device::set_sprite_ctrl( UINT16 sprctrl )
{
	m_sprite_ctrl = sprctrl;
}

READ16_MEMBER( pc090oj_device::word_r )
{
	return m_ram[offset];
}

WRITE16_MEMBER( pc090oj_device::word_w )
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

void pc090oj_device::eof_callback( )
{
	if (m_use_buffer)
	{
		int i;
		for (i = 0; i < PC090OJ_ACTIVE_RAM_SIZE / 2; i++)
			m_ram_buffered[i] = m_ram[i];
	}
}


void pc090oj_device::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, int pri_type )
{
	int offs, priority = 0;
	int sprite_colbank = (m_sprite_ctrl & 0xf) << 4; /* top nibble */

	switch (pri_type)
	{
		case 0x00:
			priority = 0;   /* sprites over top bg layer */
			break;

		case 0x01:
			priority = 1;   /* sprites under top bg layer */
			break;

		case 0x02:
			priority = m_sprite_ctrl >> 15;  /* variable sprite/tile priority */
	}

	for (offs = 0; offs < PC090OJ_ACTIVE_RAM_SIZE / 2; offs += 4)
	{
		int flipx, flipy;
		int x, y;
		int data, code, color;

		data = m_ram_buffered[offs];
		flipy = (data & 0x8000) >> 15;
		flipx = (data & 0x4000) >> 14;
		color = (data & 0x000f) | sprite_colbank;

		code = m_ram_buffered[offs + 2] & 0x1fff;
		x = m_ram_buffered[offs + 3] & 0x1ff;   /* mask verified with Rainbowe board */
		y = m_ram_buffered[offs + 1] & 0x1ff;   /* mask verified with Rainbowe board */

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

		m_gfxdecode->gfx(m_gfxnum)->prio_transpen(bitmap,cliprect,
				code,
				color,
				flipx,flipy,
				x,y,
				priority_bitmap,
				priority ? 0xfc : 0xf0,0);
	}
}
