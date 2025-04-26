// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Brooktree Bt481/Bt482

    256 Color, 15-bit, 16-bit, 24-bit RAMDAC

    Notes:
    - Bt482 additionally supports a custom shaped 32x32x2 cursor

    TODO:
    - 6-bit mode
    - Cursor mode 2 and 3
    - Access command registers without using RS2

***************************************************************************/

#include "emu.h"
#include "bt48x.h"

#define LOG_ACCESS   (1U << 1) // logs all access to handlers
#define LOG_INDIRECT (1U << 2) // logs reads/writes to indirect registers (except cursor movement)
#define LOG_CURSOR   (1U << 3) // logs cursor movement

#define VERBOSE (LOG_GENERAL | LOG_INDIRECT)

#include "logmacro.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BT481, bt481_device, "bt481", "Brooktree Bt481 RAMDAC")
DEFINE_DEVICE_TYPE(BT482, bt482_device, "bt482", "Brooktree Bt482 RAMDAC")

bt481_device::bt481_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_palette_interface(mconfig, *this)
{
}

bt481_device::bt481_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	bt481_device(mconfig, BT481, tag, owner, clock)
{
}

bt482_device::bt482_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	bt481_device(mconfig, BT482, tag, owner, clock)
{
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void bt481_device::device_start()
{
	// initialize power-up values
	m_cra = 0x00;
	m_crb = 0x1e;
	m_cursor = 0x00;

	// register for save states
	save_item(NAME(m_cra));
	save_item(NAME(m_crb));
	save_item(NAME(m_color));
	save_item(NAME(m_addr));
	save_item(NAME(m_addr_rgb));
	save_item(NAME(m_indirect_index));
	save_item(NAME(m_read_mask));
	save_item(NAME(m_overlay_mask));
	save_item(NAME(m_cursor));
}

void bt482_device::device_start()
{
	bt481_device::device_start();

	// register for save states
	save_item(NAME(m_cram));
	save_item(NAME(m_cxlr));
	save_item(NAME(m_cxhr));
	save_item(NAME(m_cylr));
	save_item(NAME(m_cyhr));
	save_item(NAME(m_cx));
	save_item(NAME(m_cy));
}

void bt481_device::map(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(bt481_device::address_r), FUNC(bt481_device::address_w));
	map(0x01, 0x01).rw(FUNC(bt481_device::palette_r), FUNC(bt481_device::palette_w));
	map(0x02, 0x02).rw(FUNC(bt481_device::mask_r), FUNC(bt481_device::mask_w));
	map(0x03, 0x03).w(FUNC(bt481_device::address_read_w));
	map(0x04, 0x04).rw(FUNC(bt481_device::overlay_address_r), FUNC(bt481_device::overlay_address_w));
	map(0x05, 0x05).rw(FUNC(bt481_device::overlay_r), FUNC(bt481_device::overlay_w));
	map(0x06, 0x06).w(FUNC(bt481_device::command_w));
	map(0x07, 0x07).w(FUNC(bt481_device::overlay_address_read_w));
}

uint8_t bt481_device::address_r()
{
	LOGMASKED(LOG_ACCESS, "address_r: %02x\n", m_addr);

	return m_addr;
}

void bt481_device::address_w(uint8_t data)
{
	LOGMASKED(LOG_ACCESS, "address_w: %02x\n", data);

	if (BIT(m_cra, 0))
		m_indirect_index = data & 0x07;

	m_addr = data;
	m_addr_rgb = 0;
}

uint8_t bt481_device::palette_r()
{
	LOGMASKED(LOG_ACCESS, "palette_r (index %02x)\n", m_addr);

	uint32_t color = pen_color(m_addr);
	return color >> (2 - m_addr_rgb);
}

void bt481_device::palette_w(uint8_t data)
{
	LOGMASKED(LOG_ACCESS, "palette_w: %02x (index %02x)\n", data, m_addr);

	m_color[m_addr_rgb] = data;

	// we have all components, update palette and increment index
	if (m_addr_rgb == 2)
		set_pen_color(m_addr++, rgb_t(m_color[0], m_color[1], m_color[2]));

	m_addr_rgb = (m_addr_rgb + 1) % 3;
}

uint8_t bt481_device::mask_r()
{
	uint8_t data = 0xff;

	if (BIT(m_cra, 0))
	{
		switch (m_indirect_index)
		{
			case READ_MASK_REGISTER:
				LOGMASKED(LOG_INDIRECT, "R Read Mask Register = %02x\n", m_read_mask);
				data = m_read_mask;
				break;

			case OVERLAY_MASK_REGISTER:
				LOGMASKED(LOG_INDIRECT, "R Overlay Mask Register = %02x\n", m_overlay_mask);
				data = m_overlay_mask;
				break;

			case COMMAND_REGISTER_B:
				LOGMASKED(LOG_INDIRECT, "R Command Register B = %02x\n", m_crb);
				data = m_crb;
				break;

			case CURSOR_REGISTER:
				LOGMASKED(LOG_INDIRECT, "R Cursor Register = %02x\n", m_cursor);
				data = m_cursor;
				break;

			default:
				LOGMASKED(LOG_INDIRECT, "Read from unsupported indirect register %02x\n", m_indirect_index);
				data = 0xff;
				break;
		}
	}
	else
	{
		data = m_read_mask;
	}

	LOGMASKED(LOG_ACCESS, "mask_r: %02x\n", data);

	return data;
}

uint8_t bt482_device::mask_r()
{
	uint8_t data = 0xff;

	if (BIT(m_cra, 0))
	{
		switch (m_indirect_index)
		{
			case READ_MASK_REGISTER:
				LOGMASKED(LOG_INDIRECT, "R Read Mask Register = %02x\n", m_read_mask);
				data = m_read_mask;
				break;

			case OVERLAY_MASK_REGISTER:
				LOGMASKED(LOG_INDIRECT, "R Overlay Mask Register = %02x\n", m_overlay_mask);
				data = m_overlay_mask;
				break;

			case COMMAND_REGISTER_B:
				LOGMASKED(LOG_INDIRECT, "R Command Register B = %02x\n", m_crb);
				data = m_crb;
				break;

			case CURSOR_REGISTER:
				LOGMASKED(LOG_INDIRECT, "R Cursor Register = %02x\n", m_cursor);
				data = m_cursor;
				break;

			case CURSOR_X_LOW_REGISTER:
				LOGMASKED(LOG_CURSOR, "R Cursor X Low = %02x\n", m_cxlr);
				data = m_cxlr;
				break;

			case CURSOR_X_HIGH_REGISTER:
				LOGMASKED(LOG_CURSOR, "R Cursor X High = %02x\n", m_cxhr);
				data = m_cxhr;
				break;

			case CURSOR_Y_LOW_REGISTER:
				LOGMASKED(LOG_CURSOR, "R Cursor Y Low = %02x\n", m_cylr);
				data = m_cylr;
				break;

			case CURSOR_Y_HIGH_REGISTER:
				LOGMASKED(LOG_CURSOR, "R Cursor Y High = %02x\n", m_cyhr);
				data = m_cyhr;
				break;
		}
	}
	else
	{
		data = m_read_mask;
	}

	LOGMASKED(LOG_ACCESS, "mask_r: %02x\n", data);

	return data;
}

void bt481_device::mask_w(uint8_t data)
{
	LOGMASKED(LOG_ACCESS, "mask_w: %02x\n", data);

	if (BIT(m_cra, 0))
	{
		switch (m_indirect_index)
		{
			case READ_MASK_REGISTER:
				LOGMASKED(LOG_INDIRECT, "W Read Mask Register = %02x\n", data);
				m_read_mask = data;
				break;

			case OVERLAY_MASK_REGISTER:
				LOGMASKED(LOG_INDIRECT, "W Overlay Mask Register = %02x\n", data);
				m_overlay_mask = data & 0x0f;
				break;

			case COMMAND_REGISTER_B:
				LOGMASKED(LOG_INDIRECT, "W Command Register B = %02x\n", data);
				m_crb = data;
				break;

			case CURSOR_REGISTER:
				LOGMASKED(LOG_INDIRECT, "W Cursor Register = %02x\n", data);
				m_cursor = data;
				break;

			default:
				LOGMASKED(LOG_INDIRECT, "Write to unsupported indirect register %02x\n", m_indirect_index);
				break;
		}
	}
	else
	{
		m_read_mask = data;
	}
}

void bt482_device::mask_w(uint8_t data)
{
	LOGMASKED(LOG_ACCESS, "mask_w: %02x\n", data);

	if (BIT(m_cra, 0))
	{
		switch (m_indirect_index)
		{
			case READ_MASK_REGISTER:
				LOGMASKED(LOG_INDIRECT, "W Read Mask Register = %02x\n", data);
				m_read_mask = data;
				break;

			case OVERLAY_MASK_REGISTER:
				LOGMASKED(LOG_INDIRECT, "W Overlay Mask Register = %02x\n", data);
				m_overlay_mask = data & 0x0f;
				break;

			case COMMAND_REGISTER_B:
				LOGMASKED(LOG_INDIRECT, "W Command Register B = %02x\n", data);
				m_crb = data;
				break;

			case CURSOR_REGISTER:
				LOGMASKED(LOG_INDIRECT, "W Cursor Register = %02x\n", data);
				m_cursor = data;
				break;

			case CURSOR_X_LOW_REGISTER:
				LOGMASKED(LOG_CURSOR, "W Cursor X Low = %02x\n", data);
				m_cxlr = data;
				break;

			case CURSOR_X_HIGH_REGISTER:
				LOGMASKED(LOG_CURSOR, "W Cursor X High = %02x\n", data);
				m_cxhr = data & 0x0f;
				m_cx = (m_cxhr << 8) | m_cxlr;
				break;

			case CURSOR_Y_LOW_REGISTER:
				LOGMASKED(LOG_CURSOR, "W Cursor Y Low = %02x\n", data);
				m_cylr = data;
				break;

			case CURSOR_Y_HIGH_REGISTER:
				LOGMASKED(LOG_CURSOR, "W Cursor Y High = %02x\n", data);
				m_cyhr = data & 0x0f;
				m_cy = (m_cyhr << 8) | m_cylr;
				break;
		}
	}
	else
	{
		m_read_mask = data;
	}
}

void bt481_device::address_read_w(uint8_t data)
{
	LOGMASKED(LOG_ACCESS, "address_read_w: %02x\n", data);

	m_addr = data;
	m_addr_rgb = 0;
}

uint8_t bt481_device::overlay_address_r()
{
	LOGMASKED(LOG_ACCESS, "overlay_address_r: %02x\n", m_addr);

	return m_addr;
}

void bt481_device::overlay_address_w(uint8_t data)
{
	LOGMASKED(LOG_ACCESS, "overlay_address_w: %02x\n", data);

	m_addr = data;
	m_addr_rgb = 0;
}

uint8_t bt481_device::overlay_r()
{
	LOGMASKED(LOG_ACCESS, "overlay_r (index %02x)\n", m_addr);

	if (m_addr >= 20)
	{
		LOG("Overlay index out of range: %02x\n", m_addr);
		return 0;
	}

	uint32_t color = pen_color(256 + m_addr);
	return color >> (2 - m_addr_rgb);
}

uint8_t bt482_device::overlay_r()
{
	if (BIT(m_cursor, 3))
	{
		LOGMASKED(LOG_ACCESS, "overlay_r (index %02x)\n", m_addr);
		return m_cram[m_addr];
	}
	else
	{
		return bt481_device::overlay_r();
	}
}

void bt481_device::overlay_w(uint8_t data)
{
	LOGMASKED(LOG_ACCESS, "overlay_w: %02x (index %02x)\n", data, m_addr);

	if (m_addr >= 20)
	{
		LOG("Overlay index out of range: %02x\n", m_addr);
		return;
	}

	m_color[m_addr_rgb] = data;

	if (m_addr_rgb == 2)
	{
		// we have all components, update palette and increment index
		set_pen_color(256 + m_addr, rgb_t(m_color[0], m_color[1], m_color[2]));

		if (m_addr < 16)
			// while accessing the overlay color registers the 4 high bits are ignored
			m_addr = (m_addr + 1) & 0x0f;
		else
			// accessing cursor color
			m_addr++;
	}

	m_addr_rgb = (m_addr_rgb + 1) % 3;
}

void bt482_device::overlay_w(uint8_t data)
{
	if (BIT(m_cursor, 3))
	{
		LOGMASKED(LOG_ACCESS, "overlay_w: %02x (index %02x)\n", data, m_addr);
		m_cram[m_addr++] = data;
	}
	else
	{
		bt481_device::overlay_w(data);
	}
}

void bt481_device::command_w(uint8_t data)
{
	LOGMASKED(LOG_ACCESS, "command_w: %02x\n", data);

	m_cra = data;

	if (BIT(m_cra, 7))
	{
		switch ((m_cra >> 4) & 0x07)
		{
			case 0: LOG("Color mode: 5:5:5 dual-edge (33K colors)\n"); break;
			case 1: LOG("Color mode: 8:8:8:OL dual-edge (16.8M colors)\n"); break;
			case 2: LOG("Color mode: 5:5:5 single-edge (33K colors)\n"); break;
			case 4: LOG("Color mode: 5:6:5 dual-edge (65K colors)\n"); break;
			case 6: LOG("Color mode: 5:6:5 single-edge (65K colors)\n"); break;
			case 7: LOG("Color mode: 8:8:8 single-edge (16.8M colors)\n"); break;

			default: LOG("Invalid color mode selected!\n");
		}
	}
	else
	{
		LOG("Color mode: Pseudo color (256 colors)\n");
	}

	LOG("Extended register set %sabled\n", BIT(m_cra, 0) ? "en": "dis");
}

void bt481_device::overlay_address_read_w(uint8_t data)
{
	LOGMASKED(LOG_ACCESS, "overlay_address_read_w: %02x\n", data);

	m_addr = data;
	m_addr_rgb = 0;
}


//**************************************************************************
//  CURSOR RENDERING
//**************************************************************************

uint32_t bt482_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// only the 3 color cursor is supported
	if ((m_cursor & 0x03) == CURSOR_3_COLOR)
	{
		// cursor area in screen coordinates
		rectangle cursor(m_cx - 32, m_cx - 1, m_cy - 32, m_cy - 1);

		// intersect with cliprect
		cursor &= bitmap.cliprect();

		if (!cursor.empty())
		{
			for (int y = 0; y < 32; y++)
			{
				const int ypos = m_cy - 32 + y;

				for (int x = 0; x < 32; x++)
				{
					const int xpos = m_cx - 32 + x;

					if (cursor.contains(xpos, ypos))
					{
						// fetch color data from plane 0 and 1
						uint8_t p0 = BIT(m_cram[0x00 + (y * 4) + (x / 8)], 7 - (x % 8));
						uint8_t p1 = BIT(m_cram[0x80 + (y * 4) + (x / 8)], 7 - (x % 8));

						uint8_t color = (p1 << 1) | p0;

						// if we have a color draw it
						if (color)
							bitmap.pix(ypos, xpos) = pen_color(256 + 16 + color);
					}
				}
			}
		}
	}

	return 0;
}
