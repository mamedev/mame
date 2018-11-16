// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * An implementation of the Brooktree Bt459 150MHz Monolithic CMOS 256x24 Color Palette RAMDAC device.
 *
 * The device was initially rated at 135MHz and increased to 150MHz with revision B. The revision
 * register (the only software-visible change) is implemented in this emulation.
 *
 * Reference: http://www.bitsavers.org/components/brooktree/_dataBooks/1991_Brooktree_Product_Databook.pdf
 *
 * TODO
 *   - pixel pan and zoom
 *   - overlay/underlay
 */

#include "emu.h"
#include "bt459.h"

#include "screen.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(BT459, bt459_device, "bt459", "Brooktree Bt459 256 Color RAMDAC")

void bt459_device::map(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(bt459_device::address_lo_r), FUNC(bt459_device::address_lo_w));
	map(0x01, 0x01).rw(FUNC(bt459_device::address_hi_r), FUNC(bt459_device::address_hi_w));
	map(0x02, 0x02).rw(FUNC(bt459_device::register_r), FUNC(bt459_device::register_w));
	map(0x03, 0x03).rw(FUNC(bt459_device::palette_r), FUNC(bt459_device::palette_w));
}

bt459_device::bt459_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BT459, tag, owner, clock)
	, device_palette_interface(mconfig, *this)
{
}

void bt459_device::device_start()
{
	save_item(NAME(m_address));
	save_item(NAME(m_address_rgb));

	save_item(NAME(m_overlay_color));
	save_item(NAME(m_cursor_color));

	save_item(NAME(m_command_0));
	save_item(NAME(m_command_1));
	save_item(NAME(m_command_2));
	save_item(NAME(m_pixel_read_mask));
	save_item(NAME(m_pixel_blink_mask));
	save_item(NAME(m_overlay_read_mask));
	save_item(NAME(m_overlay_blink_mask));
	save_item(NAME(m_interleave));
	save_item(NAME(m_test));
	save_item(NAME(m_red_signature));
	save_item(NAME(m_green_signature));
	save_item(NAME(m_blue_signature));
	save_item(NAME(m_cursor_command));

	save_item(NAME(m_cursor_x));
	save_item(NAME(m_cursor_y));
	save_item(NAME(m_window_x));
	save_item(NAME(m_window_y));
	save_item(NAME(m_window_w));
	save_item(NAME(m_window_h));

	save_item(NAME(m_cursor_ram));
	save_item(NAME(m_palette_ram));

	save_item(NAME(m_blink_start));
	save_item(NAME(m_contrast));
}

void bt459_device::device_reset()
{
	m_blink_start = -1;
	m_contrast = 0xff;
}

/*
 * To write color data, the MPU loads the address register with the address of
 * the primary color palette RAM, overlay RAM or cursor color register location
 * to be modified. The MPU performs three successive write cycles (8 bits each
 * of red, green, and blue), using C0 and C1 to select either the primary color
 * palette RAM, overlay RAM or cursor color registers. After the blue write
 * cycle, the address register then increments to the next location, which the
 * MPU may modify by writing another sequence of red, green and blue data.
 * Reading color data is similar to writing it, except the MPU executes read
 * cycles when it reads color data.
 *
 * When the MPU is accessing the color palette RAM, overlay RAM or cursor color
 * registers, the address register increments after each blue read or write
 * cycle. To keep track of the red, green and blue read/write cycles, the
 * address register has two additional bits (ADDRa, ADDRb) that count modulo
 * three. They are reset to zero when the MPU reads or writes the address
 * register. The MPU does not have access to these bits.
 */
u8 bt459_device::get_component(rgb_t *arr, int index)
{
	switch (m_address_rgb)
	{
	case 0: // red component
		if (!machine().side_effects_disabled())
			m_address_rgb = 1;
		return (m_command_2 & CR2524) == CR2524_RED ? arr[index].g() : arr[index].r();

	case 1: // green component
		if (!machine().side_effects_disabled())
			m_address_rgb = 2;
		return arr[index].g();

	case 2: // blue component
		if (!machine().side_effects_disabled())
		{
			m_address_rgb = 0;
			m_address = (m_address + 1) & ADDRESS_MASK;
		}
		return (m_command_2 & CR2524) == CR2524_BLUE ? arr[index].g() : arr[index].b();
	}

	// can't happen
	return 0;
}

void bt459_device::set_component(rgb_t *const arr, const int index, const u8 data)
{
	switch (m_address_rgb)
	{
	case 0: // red component
		m_address_rgb = 1;
		(m_command_2 & CR2524) == CR2524_RED ? arr[index].set_g(data) : arr[index].set_r(data);
		break;

	case 1: // green component
		m_address_rgb = 2;
		arr[index].set_g(data);
		break;

	case 2: // blue component
		m_address_rgb = 0;
		m_address = (m_address + 1) & ADDRESS_MASK;
		(m_command_2 & CR2524) == CR2524_BLUE ? arr[index].set_g(data) : arr[index].set_b(data);
		break;
	}
}

READ8_MEMBER(bt459_device::address_lo_r)
{
	// reset component pointer and return address register lsb
	if (!machine().side_effects_disabled())
		m_address_rgb = 0;
	return m_address & ADDRESS_LSB;
}

WRITE8_MEMBER(bt459_device::address_lo_w)
{
	// reset component pointer and set address register lsb
	m_address_rgb = 0;
	m_address = (m_address & ADDRESS_MSB) | data;
}

READ8_MEMBER(bt459_device::address_hi_r)
{
	// reset component pointer and return address register msb
	if (!machine().side_effects_disabled())
		m_address_rgb = 0;
	return (m_address & ADDRESS_MSB) >> 8;
}

WRITE8_MEMBER(bt459_device::address_hi_w)
{
	// reset component pointer and set address register msb
	m_address_rgb = 0;
	m_address = ((data << 8) | (m_address & ADDRESS_LSB)) & ADDRESS_MASK;
}

READ8_MEMBER(bt459_device::register_r)
{
	u8 result = 0;

	switch (m_address)
	{
	case REG_OVERLAY_COLOR_0:
	case REG_OVERLAY_COLOR_1:
	case REG_OVERLAY_COLOR_2:
	case REG_OVERLAY_COLOR_3:
	case REG_OVERLAY_COLOR_4:
	case REG_OVERLAY_COLOR_5:
	case REG_OVERLAY_COLOR_6:
	case REG_OVERLAY_COLOR_7:
	case REG_OVERLAY_COLOR_8:
	case REG_OVERLAY_COLOR_9:
	case REG_OVERLAY_COLOR_10:
	case REG_OVERLAY_COLOR_11:
	case REG_OVERLAY_COLOR_12:
	case REG_OVERLAY_COLOR_13:
	case REG_OVERLAY_COLOR_14:
	case REG_OVERLAY_COLOR_15:
		return get_component(m_overlay_color, m_address & 0xf);

	case REG_CURSOR_COLOR_1: return get_component(m_cursor_color, 0);
	case REG_CURSOR_COLOR_2: return get_component(m_cursor_color, 1);
	case REG_CURSOR_COLOR_3: return get_component(m_cursor_color, 2);

	case REG_ID:
		result = m_id;
		LOG("id register read (%s)\n", machine().describe_context());
		break;

	case REG_COMMAND_0: result = m_command_0; break;
	case REG_COMMAND_1: result = m_command_1; break;
	case REG_COMMAND_2: result = m_command_2; break;
	case REG_PIXEL_READ_MASK: result = m_pixel_read_mask; break;

	case REG_PIXEL_BLINK_MASK: result = m_pixel_blink_mask; break;

	case REG_OVERLAY_READ_MASK: result = m_overlay_read_mask; break;
	case REG_OVERLAY_BLINK_MASK: result = m_overlay_blink_mask; break;
	case REG_INTERLEAVE: result = m_interleave; break;

	case REG_TEST: result = m_test; break;
	case REG_RED_SIGNATURE: result = m_red_signature; break;
	case REG_GREEN_SIGNATURE: result = m_green_signature; break;
	case REG_BLUE_SIGNATURE: result = m_blue_signature; break;

	case REG_REVISION:
		result = m_revision;
		LOG("revision register read (%s)\n", machine().describe_context());
		break;

	case REG_CURSOR_COMMAND: result = m_cursor_command; break;

	case REG_CURSOR_X_LO: result = m_cursor_x & 0xff; break;
	case REG_CURSOR_X_HI: result = (m_cursor_x >> 8); break;
	case REG_CURSOR_Y_LO: result = m_cursor_y & 0xff; break;
	case REG_CURSOR_Y_HI: result = (m_cursor_y >> 8); break;

	case REG_WINDOW_X_LO: result = m_window_x & 0xff; break;
	case REG_WINDOW_X_HI: result = (m_window_x >> 8); break;
	case REG_WINDOW_Y_LO: result = m_window_y & 0xff; break;
	case REG_WINDOW_Y_HI: result = (m_window_y >> 8); break;

	case REG_WINDOW_W_LO: result = m_window_w & 0xff; break;
	case REG_WINDOW_W_HI: result = (m_window_w >> 8); break;
	case REG_WINDOW_H_LO: result = m_window_h & 0xff; break;
	case REG_WINDOW_H_HI: result = (m_window_h >> 8); break;

	default:
		if (m_address >= CURSOR_RAM_START && m_address <= CURSOR_RAM_END)
			result = m_cursor_ram[m_address & CURSOR_RAM_MASK];
		else
			LOG("read from unknown address 0x%04x (%s)\n", m_address, machine().describe_context());
		break;
	}

	// increment address register and return result
	if (!machine().side_effects_disabled())
		m_address = (m_address + 1) & ADDRESS_MASK;

	return result;
}

WRITE8_MEMBER(bt459_device::register_w)
{
	switch (m_address)
	{
	case REG_COMMAND_0:
		m_command_0 = data;
		LOG("command register 0: multiplex select %s, use %s, blink rate %s, block mode %d bits per pixel\n",
			(data & CR0706) == CR0706_51MPX ? "5:1" :
			(data & CR0706) == CR0706_11MPX ? "1:1" :
			(data & CR0706) == CR0706_41MPX ? "4:1" : "reserved",
			(data & CR05) ? "overlay color 0" : "color palette RAM",
			(data & CR0302) == CR0302_6464 ? "64 on 64 off" :
			(data & CR0302) == CR0302_3232 ? "32 on 32 off" :
			(data & CR0302) == CR0302_1616 ? "16 on 16 off" : "16 on 48 off",
			8 >> (data & CR0100));

		// reset the blink timer
		m_blink_start = -1;
		break;

	case REG_COMMAND_1:
		m_command_1 = data;
		LOG("command register 1: pan select %d pixels, zoom factor %dx\n",
			(data >> 5), (data & CR1310) + 1);
		break;

	case REG_COMMAND_2:
		m_command_2 = data;
		LOG("command register 2: %s sync, %s IRE pedestal, load palette RAM select %s, PLL select %s, %s overlays, %s cursor, %s test\n",
			(data & CR27) ? "enable" : "disable",
			(data & CR26) ? "7.5" : "0",
			(data & CR2524) == CR2524_BLUE ? "blue RAMDAC" :
			(data & CR2524) == CR2524_GREEN ? "green RAMDAC" :
			(data & CR2524) == CR2524_RED ? "red RAMDAC" : "normal",
			(data & CR23) ? "BLANK*" : "SYNC*",
			(data & CR22) ? "X Windows" : "normal",
			(data & CR21) ? "X Windows" : "normal",
			(data & CR20) ? "data strobe" : "signature analysis");
		break;

	case REG_PIXEL_READ_MASK:
		m_pixel_read_mask = data;
		LOG("pixel read mask register: 0x%02x\n", data);
		break;

	case REG_PIXEL_BLINK_MASK:
		m_pixel_blink_mask = data;
		LOG("pixel blink mask register: 0x%02x\n", data);
		break;

	case REG_OVERLAY_READ_MASK:
		m_overlay_read_mask = data;
		LOG("overlay read mask register: 0x%02x\n", data);
		break;

	case REG_OVERLAY_BLINK_MASK:
		m_overlay_blink_mask = data;
		LOG("overlay blink mask register: 0x%02x\n", data);
		break;

	case REG_INTERLEAVE:
		m_interleave = data;
		LOG("interleave register: interleave select %d pixels, first pixel select pixel %c, overlay interleave %s, underlay %s\n",
			data >> 5,
			((data & CR3432) >> 2) + 'A',
			(data & CR31) ? "enabled" : "disabled",
			(data & CR30) ? "enabled" : "disabled");
		break;

	case REG_TEST:
		m_test = data;
		LOG("test register: 0x%02x\n", data);
		break;

	case REG_RED_SIGNATURE:
		m_red_signature = data;
		LOG("red signature register: 0x%02x\n", data);
		break;
	case REG_GREEN_SIGNATURE:
		m_green_signature = data;
		LOG("green signature register: 0x%02x\n", data);
		break;
	case REG_BLUE_SIGNATURE:
		m_blue_signature = data;
		LOG("blue signature register: 0x%02x\n", data);
		break;

	case REG_CURSOR_COMMAND:
		m_cursor_command = data;
		LOG("cursor command register: 64x64 cursor plane1 %s, 64x64 cursor plane0 %s, cross hair cursor plane1 %s, "
			"cross hair cursor plane0 %s, cursor format %s, cross hair thickness %d pixels, cursor blink %s\n",
			(data & CR47) ? "enable" : "disable",
			(data & CR46) ? "enable" : "disable",
			(data & CR45) ? "enable" : "disable",
			(data & CR44) ? "enable" : "disable",
			(data & CR43) ? "OR" : "XOR",
			(data & CR4241) + 1,
			(data & CR40) ? "enable" : "disable"
		);
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

	case REG_OVERLAY_COLOR_0:
	case REG_OVERLAY_COLOR_1:
	case REG_OVERLAY_COLOR_2:
	case REG_OVERLAY_COLOR_3:
	case REG_OVERLAY_COLOR_4:
	case REG_OVERLAY_COLOR_5:
	case REG_OVERLAY_COLOR_6:
	case REG_OVERLAY_COLOR_7:
	case REG_OVERLAY_COLOR_8:
	case REG_OVERLAY_COLOR_9:
	case REG_OVERLAY_COLOR_10:
	case REG_OVERLAY_COLOR_11:
	case REG_OVERLAY_COLOR_12:
	case REG_OVERLAY_COLOR_13:
	case REG_OVERLAY_COLOR_14:
	case REG_OVERLAY_COLOR_15:
	{
		const int index = m_address & 0xf;
		set_component(m_overlay_color, index, data);

		// update the mame palette to match the device
		if (m_address_rgb == 0)
			set_pen_color(BT459_PIXEL_COLORS + index, m_overlay_color[index]);
		return;
	}

	case REG_CURSOR_COLOR_1:
		set_component(m_cursor_color, 0, data);

		// update the mame palette to match the device
		if (m_address_rgb == 0)
			set_pen_color(BT459_PIXEL_COLORS + BT459_OVERLAY_COLORS + 0, m_cursor_color[0]);
		return;

	case REG_CURSOR_COLOR_2:
		set_component(m_cursor_color, 1, data);

		// update the mame palette to match the device
		if (m_address_rgb == 0)
			set_pen_color(BT459_PIXEL_COLORS + BT459_OVERLAY_COLORS + 1, m_cursor_color[1]);
		return;

	case REG_CURSOR_COLOR_3:
		set_component(m_cursor_color, 2, data);

		// update the mame palette to match the device
		if (m_address_rgb == 0)
			set_pen_color(BT459_PIXEL_COLORS + BT459_OVERLAY_COLORS + 2, m_cursor_color[2]);
		return;

	default:
		if (m_address >= CURSOR_RAM_START && m_address <= CURSOR_RAM_END)
			m_cursor_ram[m_address & CURSOR_RAM_MASK] = data;
		else
			LOG("write to unknown address 0x%04x data 0x%02x (%s)\n", m_address, data, machine().describe_context());
		break;
	}

	// increment address register
	m_address = (m_address + 1) & ADDRESS_MASK;
}

READ8_MEMBER(bt459_device::palette_r)
{
	// return component from palette ram
	return get_component(m_palette_ram, m_address & 0xff);
}

WRITE8_MEMBER(bt459_device::palette_w)
{
	// set component in color palette ram
	const int index = m_address & 0xff;
	set_component(m_palette_ram, index, data);

	// update the mame palette to match the device
	if (m_address_rgb == 0)
		set_pen_color(index, m_palette_ram[index]);
}

void bt459_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, u8 *pixel_data)
{
	// initialise the blink timer
	if (m_blink_start > screen.frame_number())
		m_blink_start = screen.frame_number();

	// compute the blink state according to the programmed duty cycle
	const bool blink_state = ((screen.frame_number() - m_blink_start) & (
		(m_command_0 & CR0302) == CR0302_1616 ? 0x10 :
		(m_command_0 & CR0302) == CR0302_3232 ? 0x20 :
		(m_command_0 & CR0302) == CR0302_6464 ? 0x40 : 0x30)) == 0;

	// compute the pixel mask from the pixel read mask and blink mask/state
	const u8 pixel_mask = m_pixel_read_mask & (blink_state ? 0xffU : ~m_pixel_blink_mask);

	// draw visible pixel data
	switch (m_command_0 & CR0100)
	{
	case CR0100_1BPP:
		for (int y = screen.visible_area().min_y; y <= screen.visible_area().max_y; y++)
			for (int x = screen.visible_area().min_x; x <= screen.visible_area().max_x; x += 8)
			{
				u8 data = *pixel_data++;

				bitmap.pix(y, x + 7) = get_rgb(data & 0x1, pixel_mask); data >>= 1;
				bitmap.pix(y, x + 6) = get_rgb(data & 0x1, pixel_mask); data >>= 1;
				bitmap.pix(y, x + 5) = get_rgb(data & 0x1, pixel_mask); data >>= 1;
				bitmap.pix(y, x + 4) = get_rgb(data & 0x1, pixel_mask); data >>= 1;
				bitmap.pix(y, x + 3) = get_rgb(data & 0x1, pixel_mask); data >>= 1;
				bitmap.pix(y, x + 2) = get_rgb(data & 0x1, pixel_mask); data >>= 1;
				bitmap.pix(y, x + 1) = get_rgb(data & 0x1, pixel_mask); data >>= 1;
				bitmap.pix(y, x + 0) = get_rgb(data & 0x1, pixel_mask);
			}
		break;

	case CR0100_2BPP:
		for (int y = screen.visible_area().min_y; y <= screen.visible_area().max_y; y++)
			for (int x = screen.visible_area().min_x; x <= screen.visible_area().max_x; x += 4)
			{
				u8 data = *pixel_data++;

				bitmap.pix(y, x + 3) = get_rgb(data & 0x3, pixel_mask); data >>= 2;
				bitmap.pix(y, x + 2) = get_rgb(data & 0x3, pixel_mask); data >>= 2;
				bitmap.pix(y, x + 1) = get_rgb(data & 0x3, pixel_mask); data >>= 2;
				bitmap.pix(y, x + 0) = get_rgb(data & 0x3, pixel_mask);
			}
		break;

	case CR0100_4BPP:
		for (int y = screen.visible_area().min_y; y <= screen.visible_area().max_y; y++)
			for (int x = screen.visible_area().min_x; x <= screen.visible_area().max_x; x += 2)
			{
				u8 data = *pixel_data++;

				bitmap.pix(y, x + 1) = get_rgb(data & 0xf, pixel_mask); data >>= 4;
				bitmap.pix(y, x + 0) = get_rgb(data & 0xf, pixel_mask);
			}
		break;

	case CR0100_8BPP:
		for (int y = screen.visible_area().min_y; y <= screen.visible_area().max_y; y++)
			for (int x = screen.visible_area().min_x; x <= screen.visible_area().max_x; x++)
				bitmap.pix(y, x) = get_rgb(*pixel_data++, pixel_mask);
		break;
	}

	// draw cursors when visible and not blinked off
	if ((m_cursor_command & (CR47 | CR46 | CR45 | CR44)) && ((m_cursor_command & CR40) == 0 || blink_state))
	{
		// get 64x64 bitmap and cross hair cursor plane enable
		const u8 bm_cursor_enable = (m_cursor_command & (CR47 | CR46)) >> 6;
		const u8 ch_cursor_enable = (m_cursor_command & (CR45 | CR44)) >> 4;

		// get cross hair cursor half thickness
		const int ch_thickness = (m_cursor_command & CR4241) >> 1;

		/*
		 * The cursor (x) value to be written is calculated as follows:
		 *
		 *   Cx = desired display screen (x) position + H - P
		 *
		 * where
		 *
		 *   P = 37 if 1:1 input multiplexing, 52 if 4:1 input multiplexing,
		 *       57 if 5:1 input multiplexing
		 *   H = number of pixels between the first rising edge of LD*
		 *       following the falling edge of HSYNC* to active video
		 *
		 * The cursor (y) value to be written is calculated as follows:
		 *
		 *   Cy = desired display screen (y) position + V - 32
		 *
		 * where
		 *
		 *   V = number of scan lines from the second sync pulse during
		 *       vertical blanking to active video
		 *
		 * Values from $0FC0 (-64) to $0FBF (+4031) may be loaded into the
		 * cursor (y) register. The negative values ($0FC0 to $0FFF) are used
		 * in situations where V < 32, and the cursor must be moved off the
		 * top of the screen.
		 */
		const int cursor_x = m_cursor_x + (
			(m_command_0 & CR0706) == CR0706_11MPX ? 37 :
			(m_command_0 & CR0706) == CR0706_41MPX ? 52 :
			(m_command_0 & CR0706) == CR0706_51MPX ? 57 : 0);
		const int cursor_y = (m_cursor_y < 0xfc0 ? m_cursor_y : m_cursor_y - 0x1000) + 32;

		// 64x64 bitmap cursor
		if (bm_cursor_enable)
		{
			// compute target 64x64 rectangle
			rectangle cursor(cursor_x - 31, cursor_x + 32, cursor_y - 31, cursor_y + 32);

			// intersect with screen bitmap
			cursor &= bitmap.cliprect();

			// draw if any portion is visible
			if (!cursor.empty())
			{
				for (int y = 0; y < 64; y++)
				{
					// get screen y pixel coordinate
					const int ypos = cursor_y - 31 + y;

					for (int x = 0; x < 64; x++)
					{
						// get screen x pixel coordinate
						const int xpos = cursor_x - 31 + x;

						// check if pixel is visible
						if (cursor.contains(xpos, ypos))
						{
							// retrieve 2 bits of 64x64 bitmap cursor data
							u8 data = (m_cursor_ram[y * 16 + (x >> 2)] >> ((3 - (x & 3)) << 1)) & bm_cursor_enable;

							// check for dual-cursor mode and combine with cross-hair data
							if (ch_cursor_enable)
								if (((x >= 31 - ch_thickness) && (x <= 31 + ch_thickness)) || ((y >= 31 - ch_thickness) && (y <= 31 + ch_thickness)))
									data = (m_cursor_command & CR43) ? (data | ch_cursor_enable) : (data ^ ch_cursor_enable);

							// write cursor data to screen (normal or X Window mode)
							if (data && !((m_command_2 & CR21) && data == 1))
							{
								rgb_t rgb = m_cursor_color[data - 1];
								if (m_contrast != 0xff)
									rgb.scale8(m_contrast + 1);

								bitmap.pix(ypos, xpos) = rgb;
							}
						}
					}
				}
			}
		}

		// cross hair cursor
		if (ch_cursor_enable)
		{
			// get the cross hair cursor color
			rgb_t ch_color = m_cursor_color[ch_cursor_enable - 1];
			if (m_contrast != 0xff)
				ch_color.scale8(m_contrast + 1);

			/*
			 * The window (x) value to be written is calculated as follows:
			 *
			 *   Wx = desired display screen (x) position + H - P
			 *
			 * where
			 *
			 *   P = 5 if 1:1 input multiplexing, 20 if 4:1 input multiplexing,
			 *       25 if 5:1 input multiplexing
			 *   H = number of pixels between the first rising edge of LD*
			 *       following the falling edge of HSYNC* to active video
			 *
			 * The window (y) value to be written is calculated as follows:
			 *
			 *   Wy = desired display screen (y) position + V
			 *
			 * where
			 *
			 *   V = number of scan lines from the second sync pulse during
			 *       vertical blanking to active video
			 *
			 * Values from $0000 to $0FFF may be written to the window (x) and
			 * (y) registers. A full-screen cross hair is implemented by
			 * loading the window (x,y) registers with $0000, and the window
			 * width and height registers with $0FFF.
			 */
			const bool full_screen = (m_window_x == 0 && m_window_y == 0 && m_window_w == 0x0fff && m_window_h == 0x0fff);
			const int window_x = full_screen ? screen.visible_area().min_x : m_window_x + (
				(m_command_0 & CR0706) == CR0706_11MPX ? 5 :
				(m_command_0 & CR0706) == CR0706_41MPX ? 20 :
				(m_command_0 & CR0706) == CR0706_51MPX ? 25 : 0);
			const int window_y = full_screen ? screen.visible_area().min_y : m_window_y;

			/*
			 * The actual window width is 2, 8 or 10 pixels more than the
			 * value specified by the window width register, depending on
			 * whether 1:1, 4:1 or 5:1 input multiplexing is specified. The
			 * actual window height is 2 pixels more than the value specified
			 * by the window height register. Therefore, the minimum window
			 * width is 2, 8 or 10 pixels for 1:1, 4:1 and 5:1 multiplexing,
			 * respectively. The minimum window height is 2 pixels.
			 *
			 * Values from $0000 to $0FFF may be written to the window width
			 * and height registers.
			 *
			 * Note: testing indicates the cross-hair cursor should be drawn
			 * strictly inside the window, although this is not 100% clear from
			 * the documentation.
			 */
			const int window_w = full_screen ? screen.visible_area().width() : m_window_w + (
				(m_command_0 & CR0706) == CR0706_11MPX ? 2 :
				(m_command_0 & CR0706) == CR0706_41MPX ? 8 :
				(m_command_0 & CR0706) == CR0706_51MPX ? 10 : 0);
			const int window_h = full_screen ? screen.visible_area().height() : m_window_h + 2;

			// check for dual-cursor mode
			if (bm_cursor_enable)
			{
				// draw the cross hair cursor as vertical and horizontal filled rectangles broken by the 64x64 cursor area
				rectangle v1(cursor_x - ch_thickness, cursor_x + ch_thickness, window_y + 1, cursor_y - 32);
				rectangle v2(cursor_x - ch_thickness, cursor_x + ch_thickness, cursor_y + 33, window_y + window_h);
				rectangle h1(window_x + 1, cursor_x - 32, cursor_y - ch_thickness, cursor_y + ch_thickness);
				rectangle h2(cursor_x + 33, window_x + window_w, cursor_y - ch_thickness, cursor_y + ch_thickness);

				v1 &= bitmap.cliprect();
				v2 &= bitmap.cliprect();
				h1 &= bitmap.cliprect();
				h2 &= bitmap.cliprect();

				if (!v1.empty())
					bitmap.fill(ch_color, v1);
				if (!v2.empty())
					bitmap.fill(ch_color, v2);
				if (!h1.empty())
					bitmap.fill(ch_color, h1);
				if (!h2.empty())
					bitmap.fill(ch_color, h2);
			}
			else
			{
				// draw the cross hair cursor as unbroken vertical and horizontal filled rectangles
				rectangle v(cursor_x - ch_thickness, cursor_x + ch_thickness, window_y + 1, window_y + window_h);
				rectangle h(window_x + 1, window_x + window_w, cursor_y - ch_thickness, cursor_y + ch_thickness);

				v &= bitmap.cliprect();
				h &= bitmap.cliprect();

				if (!v.empty())
					bitmap.fill(ch_color, v);
				if (!h.empty())
					bitmap.fill(ch_color, h);
			}
		}
	}
}
