// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Acho A. Tang, R. Belmont
/*

Konami 007420
------
Sprite generator. 8 bytes per sprite with zoom. It uses 0x200 bytes of RAM,
and a variable amount of ROM. Nothing is known about its external interface.

TODO:
- sprite X wraparound? (Rock N Rage sprites disappears on left edge of screen)

*/

#include "emu.h"
#include "k007420.h"
#include "konami_helper.h"

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

static constexpr uint32_t K007420_SPRITERAM_SIZE = 0x200;

DEFINE_DEVICE_TYPE(K007420, k007420_device, "k007420", "Konami 007420 Sprite Generator")

k007420_device::k007420_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, K007420, tag, owner, clock)
	, device_gfx_interface(mconfig, *this)
	, m_ram(nullptr)
	, m_flipscreen(false)
	, m_wrap_y(false)
	, m_banklimit(0)
	, m_callback(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k007420_device::device_start()
{
	// bind the init function
	m_callback.resolve();

	m_ram = make_unique_clear<uint8_t[]>(K007420_SPRITERAM_SIZE);

	save_pointer(NAME(m_ram), K007420_SPRITERAM_SIZE);
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_wrap_y));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k007420_device::device_reset()
{
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

uint8_t k007420_device::read(offs_t offset)
{
	return m_ram[offset];
}

void k007420_device::write(offs_t offset, uint8_t data)
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

void k007420_device::sprites_draw(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const uint32_t codemask = m_banklimit;
	const uint32_t bankmask = ~m_banklimit;

	for (int offs = K007420_SPRITERAM_SIZE - 8; offs >= 0; offs -= 8)
	{
		static const int xoffset[4] = { 0, 1, 4, 5 };
		static const int yoffset[4] = { 0, 2, 8, 10 };

		uint32_t code = m_ram[offs + 1];
		uint32_t color = m_ram[offs + 2];
		int32_t ox = m_ram[offs + 3] - ((m_ram[offs + 4] & 0x80) << 1);
		int32_t oy = 256 - m_ram[offs + 0];
		bool flipx = m_ram[offs + 4] & 0x04;
		bool flipy = m_ram[offs + 4] & 0x08;

		m_callback(code, color);

		const uint32_t bank = code & bankmask;
		code &= codemask;

		// 0x080 = normal scale, 0x040 = double size, 0x100 half size
		uint32_t zoom = m_ram[offs + 5] | ((m_ram[offs + 4] & 0x03) << 8);
		if (!zoom)
			continue;
		zoom = 0x10000 * 128 / zoom;

		uint8_t width, height;
		switch (m_ram[offs + 4] & 0x70)
		{
			case 0x30:
				width = height = 1;
				break;

			case 0x20:
				width = 2; height = 1;
				code &= ~1;
				break;

			case 0x10:
				width = 1; height = 2;
				code &= ~2;
				break;

			case 0x00:
				width = height = 2;
				code &= ~3;
				break;

			case 0x40:
				width = height = 4;
				code &= ~0xf;
				break;

			default:
				width = 1; height = 1;
				//logerror("Unknown sprite size %02x\n",(m_ram[offs + 4] & 0x70) >> 4);
				break;
		}

		if (m_flipscreen)
		{
			ox = 256 - ox - ((zoom * width + (1 << 12)) >> 13);
			oy = 256 - oy - ((zoom * height + (1 << 12)) >> 13);
			flipx = !flipx;
			flipy = !flipy;
		}

		if (zoom == 0x10000)
		{
			for (int y = 0; y < height; y++)
			{
				const int sy = oy + 8 * y;

				for (int x = 0; x < width; x++)
				{
					uint32_t c = code;

					const int sx = ox + 8 * x;
					if (flipx)
						c += xoffset[(width - 1 - x)];
					else
						c += xoffset[x];

					if (flipy)
						c += yoffset[(height - 1 - y)];
					else
						c += yoffset[y];

					if (c & bankmask)
						continue;
					else
						c += bank;

					gfx(0)->transpen(bitmap,cliprect,
							c,
							color,
							flipx,flipy,
							sx,sy,0);

					if (m_wrap_y)
					{
						const int dy = m_flipscreen ? +256 : -256;
						gfx(0)->transpen(bitmap,cliprect,
								c,
								color,
								flipx,flipy,
								sx,sy+dy,0);
					}
				}
			}
		}
		else
		{
			for (int y = 0; y < height; y++)
			{
				const int sy = oy + ((zoom * y + (1 << 12)) >> 13);
				const int zh = (oy + ((zoom * (y + 1) + (1 << 12)) >> 13)) - sy;

				for (int x = 0; x < width; x++)
				{
					uint32_t c = code;

					const int sx = ox + ((zoom * x + (1 << 12)) >> 13);
					const int zw = (ox + ((zoom * (x + 1) + (1 << 12)) >> 13)) - sx;
					if (flipx)
						c += xoffset[(width - 1 - x)];
					else
						c += xoffset[x];

					if (flipy)
						c += yoffset[(height - 1 - y)];
					else
						c += yoffset[y];

					if (c & bankmask)
						continue;
					else
						c += bank;

					gfx(0)->zoom_transpen(bitmap,cliprect,
							c,
							color,
							flipx,flipy,
							sx,sy,
							(zw << 16) / 8,(zh << 16) / 8,0);

					if (m_wrap_y)
					{
						const int dy = m_flipscreen ? +256 : -256;
						gfx(0)->zoom_transpen(bitmap,cliprect,
								c,
								color,
								flipx,flipy,
								sx,sy+dy,
								(zw << 16) / 8,(zh << 16) / 8,0);
					}
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
