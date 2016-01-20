// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Acho A. Tang, R. Belmont
/*
Konami 007420
------
Sprite generator. 8 bytes per sprite with zoom. It uses 0x200 bytes of RAM,
and a variable amount of ROM. Nothing is known about its external interface.
*/

#include "emu.h"
#include "k007420.h"
#include "konami_helper.h"

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

#define K007420_SPRITERAM_SIZE 0x200

const device_type K007420 = &device_creator<k007420_device>;

k007420_device::k007420_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, K007420, "K007420 Sprite Generator", tag, owner, clock, "k007420", __FILE__),
	m_ram(nullptr),
	m_flipscreen(0),
	m_palette(*this),
	m_banklimit(0)
	//m_regs[8],
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k007420_device::device_start()
{
	// bind the init function
	m_callback.bind_relative_to(*owner());

	m_ram = make_unique_clear<UINT8[]>(0x200);

	save_pointer(NAME(m_ram.get()), 0x200);
	save_item(NAME(m_flipscreen));   // current one uses 7342 one
	save_item(NAME(m_regs)); // current one uses 7342 ones
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k007420_device::device_reset()
{
	int i;

	m_flipscreen = 0;
	for (i = 0; i < 8; i++)
		m_regs[i] = 0;
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

READ8_MEMBER( k007420_device::read )
{
	return m_ram[offset];
}

WRITE8_MEMBER( k007420_device::write )
{
	m_ram[offset] = data;
}

/*
 * Sprite Format
 * ------------------
 *
 * Byte | Bit(s)   | Use
 * -----+-76543210-+----------------
 *   0  | xxxxxxxx | y position
 *   1  | xxxxxxxx | sprite code (low 8 bits)
 *   2  | xxxxxxxx | depends on external conections. Usually banking
 *   3  | xxxxxxxx | x position (low 8 bits)
 *   4  | x------- | x position (high bit)
 *   4  | -xxx---- | sprite size 000=16x16 001=8x16 010=16x8 011=8x8 100=32x32
 *   4  | ----x--- | flip y
 *   4  | -----x-- | flip x
 *   4  | ------xx | zoom (bits 8 & 9)
 *   5  | xxxxxxxx | zoom (low 8 bits)  0x080 = normal, < 0x80 enlarge, > 0x80 reduce
 *   6  | xxxxxxxx | unused
 *   7  | xxxxxxxx | unused
 */

void k007420_device::sprites_draw( bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx )
{
	int offs;
	int codemask = m_banklimit;
	int bankmask = ~m_banklimit;

	for (offs = K007420_SPRITERAM_SIZE - 8; offs >= 0; offs -= 8)
	{
		int ox, oy, code, color, flipx, flipy, zoom, w, h, x, y, bank;
		static const int xoffset[4] = { 0, 1, 4, 5 };
		static const int yoffset[4] = { 0, 2, 8, 10 };

		code = m_ram[offs + 1];
		color = m_ram[offs + 2];
		ox = m_ram[offs + 3] - ((m_ram[offs + 4] & 0x80) << 1);
		oy = 256 - m_ram[offs + 0];
		flipx = m_ram[offs + 4] & 0x04;
		flipy = m_ram[offs + 4] & 0x08;

		m_callback(&code, &color);

		bank = code & bankmask;
		code &= codemask;

		/* 0x080 = normal scale, 0x040 = double size, 0x100 half size */
		zoom = m_ram[offs + 5] | ((m_ram[offs + 4] & 0x03) << 8);
		if (!zoom)
			continue;
		zoom = 0x10000 * 128 / zoom;

		switch (m_ram[offs + 4] & 0x70)
		{
			case 0x30: w = h = 1; break;
			case 0x20: w = 2; h = 1; code &= (~1); break;
			case 0x10: w = 1; h = 2; code &= (~2); break;
			case 0x00: w = h = 2; code &= (~3); break;
			case 0x40: w = h = 4; code &= (~3); break;
			default: w = 1; h = 1;
//logerror("Unknown sprite size %02x\n",(m_ram[offs + 4] & 0x70) >> 4);
		}

		if (m_flipscreen)
		{
			ox = 256 - ox - ((zoom * w + (1 << 12)) >> 13);
			oy = 256 - oy - ((zoom * h + (1 << 12)) >> 13);
			flipx = !flipx;
			flipy = !flipy;
		}

		if (zoom == 0x10000)
		{
			int sx, sy;

			for (y = 0; y < h; y++)
			{
				sy = oy + 8 * y;

				for (x = 0; x < w; x++)
				{
					int c = code;

					sx = ox + 8 * x;
					if (flipx)
						c += xoffset[(w - 1 - x)];
					else
						c += xoffset[x];

					if (flipy)
						c += yoffset[(h - 1 - y)];
					else
						c += yoffset[y];

					if (c & bankmask)
						continue;
					else
						c += bank;

					gfx->transpen(bitmap,cliprect,
						c,
						color,
						flipx,flipy,
						sx,sy,0);

					if (m_regs[2] & 0x80)
						gfx->transpen(bitmap,cliprect,
							c,
							color,
							flipx,flipy,
							sx,sy-256,0);
				}
			}
		}
		else
		{
			int sx, sy, zw, zh;
			for (y = 0; y < h; y++)
			{
				sy = oy + ((zoom * y + (1 << 12)) >> 13);
				zh = (oy + ((zoom * (y + 1) + (1 << 12)) >> 13)) - sy;

				for (x = 0; x < w; x++)
				{
					int c = code;

					sx = ox + ((zoom * x + (1<<12)) >> 13);
					zw = (ox + ((zoom * (x + 1) + (1 << 12)) >> 13)) - sx;
					if (flipx)
						c += xoffset[(w - 1 - x)];
					else
						c += xoffset[x];

					if (flipy)
						c += yoffset[(h - 1 - y)];
					else
						c += yoffset[y];

					if (c & bankmask)
						continue;
					else
						c += bank;

					gfx->zoom_transpen(bitmap,cliprect,
						c,
						color,
						flipx,flipy,
						sx,sy,
						(zw << 16) / 8,(zh << 16) / 8,0);

					if (m_regs[2] & 0x80)
						gfx->zoom_transpen(bitmap,cliprect,
							c,
							color,
							flipx,flipy,
							sx,sy-256,
							(zw << 16) / 8,(zh << 16) / 8,0);
				}
			}
		}
	}
#if 0
	{
		static int current_sprite = 0;

		if (machine().input().code_pressed_once(KEYCODE_Z)) current_sprite = (current_sprite+1) & ((K007420_SPRITERAM_SIZE/8)-1);
		if (machine().input().code_pressed_once(KEYCODE_X)) current_sprite = (current_sprite-1) & ((K007420_SPRITERAM_SIZE/8)-1);

		popmessage("%02x:%02x %02x %02x %02x %02x %02x %02x %02x", current_sprite,
			m_ram[(current_sprite*8)+0], m_ram[(current_sprite*8)+1],
			m_ram[(current_sprite*8)+2], m_ram[(current_sprite*8)+3],
			m_ram[(current_sprite*8)+4], m_ram[(current_sprite*8)+5],
			m_ram[(current_sprite*8)+6], m_ram[(current_sprite*8)+7]);
	}
#endif
}

//-------------------------------------------------
//  static_set_palette_tag: Set the tag of the
//  palette device
//-------------------------------------------------

void k007420_device::static_set_palette_tag(device_t &device, std::string tag)
{
	downcast<k007420_device &>(device).m_palette.set_tag(tag);
}
