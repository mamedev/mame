// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Brooktree Bt431 Monolithic CMOS 64x64 Pixel Cursor Generator.
 *
 * Sources:
 *   - http://bitsavers.org/components/brooktree/_dataBooks/1993_Brooktree_Graphics_and_Imaging_Product_Databook.pdf
 *
 * TODO:
 *   - test, profile and optimize
 */

#include "emu.h"
#include "bt431.h"

#define LOG_GENERAL   (1U << 0)

//#define VERBOSE       (LOG_GENERAL)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(BT431, bt431_device, "bt431", "Bt431 64x64 Pixel Cursor Generator")

bt431_device::bt431_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, BT431, tag, owner, clock)
{
}

void bt431_device::device_start()
{
	save_item(NAME(m_address));
	save_item(NAME(m_command));

	save_item(NAME(m_cursor_x));
	save_item(NAME(m_cursor_y));
	save_item(NAME(m_window_x));
	save_item(NAME(m_window_y));
	save_item(NAME(m_window_w));
	save_item(NAME(m_window_h));

	save_item(NAME(m_ram));
}

void bt431_device::device_reset()
{
	m_address = 0;
	m_command = 0;

	update();
}

void bt431_device::map(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(bt431_device::addr_r<0>), FUNC(bt431_device::addr_w<0>));
	map(0x01, 0x01).rw(FUNC(bt431_device::addr_r<8>), FUNC(bt431_device::addr_w<8>));
	map(0x02, 0x02).rw(FUNC(bt431_device::ram_r), FUNC(bt431_device::ram_w));
	map(0x03, 0x03).rw(FUNC(bt431_device::reg_r), FUNC(bt431_device::reg_w));
}

u8 bt431_device::ram_r()
{
	u8 const data = m_ram[m_address & ADDRESS_MASK];

	// increment address register
	if (!machine().side_effects_disabled())
		m_address = (m_address + 1) & ADDRESS_MASK;

	return data;
}

void bt431_device::ram_w(u8 data)
{
	m_ram[m_address & ADDRESS_MASK] = data;

	// increment address register
	if (!machine().side_effects_disabled())
		m_address = (m_address + 1) & ADDRESS_MASK;
}

u8 bt431_device::reg_r()
{
	u8 data = 0;

	switch (m_address & 0xf)
	{
	case REG_COMMAND: data = m_command; break;

	case REG_CURSOR_X_LO: data = m_cursor_x & 0xff; break;
	case REG_CURSOR_X_HI: data = (m_cursor_x >> 8); break;
	case REG_CURSOR_Y_LO: data = m_cursor_y & 0xff; break;
	case REG_CURSOR_Y_HI: data = (m_cursor_y >> 8); break;

	case REG_WINDOW_X_LO: data = m_window_x & 0xff; break;
	case REG_WINDOW_X_HI: data = (m_window_x >> 8); break;
	case REG_WINDOW_Y_LO: data = m_window_y & 0xff; break;
	case REG_WINDOW_Y_HI: data = (m_window_y >> 8); break;

	case REG_WINDOW_W_LO: data = m_window_w & 0xff; break;
	case REG_WINDOW_W_HI: data = (m_window_w >> 8); break;
	case REG_WINDOW_H_LO: data = m_window_h & 0xff; break;
	case REG_WINDOW_H_HI: data = (m_window_h >> 8); break;

	default:
		LOG("read from unknown address 0x%04x (%s)\n",
			m_address, machine().describe_context());
		break;
	}

	// increment address register
	if (!machine().side_effects_disabled())
		m_address = (m_address + 1) & ADDRESS_MASK;

	return data;
}

void bt431_device::reg_w(u8 data)
{
	switch (m_address & 0xf)
	{
	case REG_COMMAND:
		m_command = data & CR_WM;
		LOG("64x64 cursor %s, cross hair cursor %s, cursor format %s, cross hair thickness %d\n",
			(data & CR_D6) ? "enable" : "disable",
			(data & CR_D5) ? "enable" : "disable",
			(data & CR_D4) ? "OR" : "XOR",
			((data & CR_D1D0) << 1) + 1);
		break;

	case REG_CURSOR_X_LO:
		m_cursor_x = (m_cursor_x & 0x0f00) | data;
		LOG("cursor x low register: 0x%02x\n", data);
		break;
	case REG_CURSOR_X_HI:
		m_cursor_x = ((data & 0xf) << 8) | (m_cursor_x & 0xff);
		LOG("cursor x high register: 0x%02x\n", data);
		break;
	case REG_CURSOR_Y_LO:
		m_cursor_y = (m_cursor_y & 0x0f00) | data;
		LOG("cursor y low register: 0x%02x\n", data);
		break;
	case REG_CURSOR_Y_HI:
		m_cursor_y = ((data & 0xf) << 8) | (m_cursor_y & 0xff);
		LOG("cursor y high register: 0x%02x\n", data);
		break;

	case REG_WINDOW_X_LO:
		m_window_x = (m_window_x & 0x0f00) | data;
		LOG("window x low register: 0x%02x\n", data);
		break;
	case REG_WINDOW_X_HI:
		m_window_x = ((data & 0xf) << 8) | (m_window_x & 0xff);
		LOG("window x high register: 0x%02x\n", data);
		break;
	case REG_WINDOW_Y_LO:
		m_window_y = (m_window_y & 0x0f00) | data;
		LOG("window y low register: 0x%02x\n", data);
		break;
	case REG_WINDOW_Y_HI:
		m_window_y = ((data & 0xf) << 8) | (m_window_y & 0xff);
		LOG("window y high register: 0x%02x\n", data);
		break;

	case REG_WINDOW_W_LO:
		m_window_w = (m_window_w & 0x0f00) | data;
		LOG("window width low register: 0x%02x\n", data);
		break;
	case REG_WINDOW_W_HI:
		m_window_w = ((data & 0xf) << 8) | (m_window_w & 0xff);
		LOG("window width high register: 0x%02x\n", data);
		break;
	case REG_WINDOW_H_LO:
		m_window_h = (m_window_h & 0x0f00) | data;
		LOG("window height low register: 0x%02x\n", data);
		break;
	case REG_WINDOW_H_HI:
		m_window_h = ((data & 0xf) << 8) | (m_window_h & 0xff);
		LOG("window height high register: 0x%02x\n", data);
		break;

	default:
		LOG("write to unknown address 0x%04x data 0x%02x (%s)\n",
			m_address, data, machine().describe_context());
		break;
	}

	// increment address register
	m_address = (m_address + 1) & ADDRESS_MASK;

	update();
}

void bt431_device::update()
{
	/*
	 * The cursor (x) value to be written is calculated as follows:
	 *
	 *   Cx = desired display screen (x) position + D + H - P
	 *
	 * where
	 *
	 *   P = 37 if 1:1 output multiplexing, 52 if 4:1 output multiplexing,
	 *       57 if 5:1 output multiplexing
	 *   D = skew (in pixels) between the output cursor data and external pixel
	 *       data
	 *   H = number of pixels between the first rising edge of CLOCK
	 *       following the falling edge of HSYNC* to active video
	 *
	 * The P value is one-half cursor RAM width + (internal pipeline delay in
	 * clock cycles * one, four or five, depending on multiplex selection).
	 *
	 * The cursor (y) value to be written is calculated as follows:
	 *
	 *   Cy = desired display screen (y) position + V - 32
	 *
	 * where
	 *
	 *   V = number of scan lines from the first falling edge of HSYNC* that is
	 *       two or more clock cycles after the falling edge of VSYNC* to
	 *       active video.
	 *
	 * Values from $0FC0 (-64) to $0FBF (+4031) may be loaded into the
	 * cursor (y) register. The negative values ($0FC0 to $0FFF) are used
	 * in situations where V < 32, and the cursor must be moved off the
	 * top of the screen.
	 */
	const int cursor_x = m_cursor_x + (
		(m_command & CR_D3D2) == CR_D3D2_11 ? 37 :
		(m_command & CR_D3D2) == CR_D3D2_41 ? 52 :
		(m_command & CR_D3D2) == CR_D3D2_51 ? 57 : 0);
	const int cursor_y = (m_cursor_y < 0xfc0 ? m_cursor_y : m_cursor_y - 0x1000) + 32;

	// update bitmap cursor drawing rectangle
	m_bm_window.set(cursor_x - 31, cursor_x + 32, cursor_y - 31, cursor_y + 32);

	// update cross hair cursor drawing rectangles
	const int thickness = m_command & CR_D1D0;
	if (m_window_x == 0 && m_window_y == 0 && m_window_w == 0x0fff && m_window_h == 0x0fff)
	{
		// full screen cross hair cursor
		m_ch_v.set(cursor_x - thickness, cursor_x + thickness, m_window_y, m_window_y + m_window_h - 1);
		m_ch_h.set(m_window_x, m_window_x + m_window_w - 1, cursor_y - thickness, cursor_y + thickness);
	}
	else
	{
		// windowed cross hair cursor
		const int window_x = m_window_x + (
			(m_command & CR_D3D2) == CR_D3D2_11 ? 5 :
			(m_command & CR_D3D2) == CR_D3D2_41 ? 20 :
			(m_command & CR_D3D2) == CR_D3D2_51 ? 25 : 0);
		const int window_y = m_window_y;

		const int window_w = m_window_w + (
			(m_command & CR_D3D2) == CR_D3D2_11 ? 2 :
			(m_command & CR_D3D2) == CR_D3D2_41 ? 8 :
			(m_command & CR_D3D2) == CR_D3D2_51 ? 10 : 0);
		const int window_h = m_window_h + 2;

		m_ch_v.set(cursor_x - thickness, cursor_x + thickness, window_y + 1, window_y + window_h - 2);
		m_ch_h.set(window_x + 1, window_x + window_w - 2, cursor_y - thickness, cursor_y + thickness);
	}
}

bool bt431_device::cur_r(unsigned x, unsigned y) const
{
	bool data = false;

	// cross hair cursor
	if ((m_command & CR_D5) && (m_ch_h.contains(x, y) || m_ch_v.contains(x, y)))
		data = true;

	// bitmap cursor
	if ((m_command & CR_D6) && m_bm_window.contains(x, y))
	{
		bool const bit = BIT(m_ram[(y - m_bm_window.top()) * 8 + (x - m_bm_window.left()) / 8], 7 - (x - m_bm_window.left()) % 8);

		if (m_command & CR_D4)
			data |= bit;
		else
			data ^= bit;
	}

	return data;
}
