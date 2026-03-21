// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Chips 82C245 CGA LCD/CRT Controller

    TODO:
    - currently assumes LCD screen, CRT timings and CGA palette not used.
    - SMARTMAP, intelligently map colors to gray scales.

**********************************************************************/

#include "emu.h"
#include "82c425.h"

#include "video/cgapal.h"

#include "screen.h"


#define LOG_SETUP  (1U << 1)
#define LOG_REGS   (1U << 2)

//#define VERBOSE (LOG_REGS|LOG_SETUP)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

#define LOGSETUP(...)   LOGMASKED(LOG_SETUP,  __VA_ARGS__)
#define LOGREGS(...)    LOGMASKED(LOG_REGS,  __VA_ARGS__)


DEFINE_DEVICE_TYPE(F82C425, f82c425_device, "82c425", "82C425 LCD/CRT Controller")


f82c425_device::f82c425_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, F82C425, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, device_video_interface(mconfig, *this)
	, m_space_config("dispfont", ENDIANNESS_LITTLE, 8, 15, 0, address_map_constructor(FUNC(f82c425_device::dispfont_map), this))
	, m_palette(*this, finder_base::DUMMY_TAG)
	, m_crt_lcd_cb(*this)
{
}

// default display/font storage
void f82c425_device::dispfont_map(address_map &map)
{
	if (!has_configured_map(0))
		map(0x0000, 0x5fff).ram();
}

void f82c425_device::io_map(address_map &map)
{
	map.global_mask(0x0f);
	map(0x04, 0x04).rw(FUNC(f82c425_device::address_r), FUNC(f82c425_device::address_w));
	map(0x05, 0x05).rw(FUNC(f82c425_device::register_r), FUNC(f82c425_device::register_w));
	map(0x08, 0x0c).rw(FUNC(f82c425_device::extreg_r), FUNC(f82c425_device::extreg_w));
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector f82c425_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}


void f82c425_device::device_start()
{
	assert(clock() > 0);

	// arbitrary startup values */
	m_horiz_total  = 0xff;
	m_max_scan_row = 0x0f;
	m_vert_total   = 0x7f;
	m_horiz_sync_pos = 1;
	m_vert_sync_pos = 1;
	m_horiz_disp = m_vert_disp = 0;
	m_vert_sync_pos = 0;
	m_vert_total_adj = 0;
	m_cursor_start_scan = m_cursor_end_scan = m_cursor_addr = 0;
	m_disp_start_addr = 0;

	m_blink_state  = false;
	m_cursor_state = false;
	m_dispen_state = false;

	// default startup values (from datasheet)
	m_hsync_width = 0x40;
	m_vsync_width = 0x72;

	save_item(NAME(m_register_address));
	save_item(NAME(m_horiz_total));
	save_item(NAME(m_horiz_disp));
	save_item(NAME(m_horiz_sync_pos));
	save_item(NAME(m_vert_total));
	save_item(NAME(m_vert_total_adj));
	save_item(NAME(m_vert_disp));
	save_item(NAME(m_vert_sync_pos));
	save_item(NAME(m_max_scan_row));
	save_item(NAME(m_cursor_start_scan));
	save_item(NAME(m_cursor_end_scan));
	save_item(NAME(m_disp_start_addr));
	save_item(NAME(m_cursor_addr));
	save_item(NAME(m_light_pen_addr));
	save_item(NAME(m_light_pen_latched));
	save_item(NAME(m_ac_control));
	save_item(NAME(m_threshold));
	save_item(NAME(m_shift_param));
	save_item(NAME(m_hsync_width));
	save_item(NAME(m_vsync_width));
	save_item(NAME(m_timing_control));
	save_item(NAME(m_func_control));
	save_item(NAME(m_mode_control));
	save_item(NAME(m_color_select));
	save_item(NAME(m_input_status));
	save_item(NAME(m_blink_state));
	save_item(NAME(m_cursor_state));
	save_item(NAME(m_dispen_state));
}


void f82c425_device::device_reset()
{
	m_register_address  = 0;
	m_light_pen_addr    = 0;
	m_light_pen_latched = false;

	// reset values (from datasheet)
	m_ac_control   = 0x00;
	m_threshold    = 0x00;
	m_shift_param  = 0x00;
	m_func_control = 0x00;
	m_mode_control = 0x00;
	m_color_select = 0x00;
	m_input_status = 0x00;
}


uint8_t f82c425_device::register_r()
{
	uint8_t data = 0x00;

	switch (m_register_address)
	{
	case 0x00: data = m_horiz_total; break;
	case 0x01: data = m_horiz_disp; break;
	case 0x02: data = m_horiz_sync_pos; break;
	case 0x03: break;
	case 0x04: data = m_vert_total; break;
	case 0x05: data = m_vert_total_adj; break;
	case 0x06: data = m_vert_disp; break;
	case 0x07: data = m_vert_sync_pos; break;
	case 0x08: break;
	case 0x09: data = m_max_scan_row; break;
	case 0x0a: data = m_cursor_start_scan; break;
	case 0x0b: data = m_cursor_end_scan; break;
	case 0x0c: data = (m_disp_start_addr >> 8) & 0xff; break;
	case 0x0d: data = (m_disp_start_addr >> 0) & 0xff; break;
	case 0x0e: data = (m_cursor_addr >> 8) & 0xff; break;
	case 0x0f: data = (m_cursor_addr >> 0) & 0xff; break;
	case 0x10: data = (m_light_pen_addr >> 8) & 0xff; m_light_pen_latched = false; break;;
	case 0x11: data = (m_light_pen_addr >> 0) & 0xff; m_light_pen_latched = false; break;
	// extension registers
	case 0xd9: data = m_ac_control; break;
	case 0xda: data = m_threshold; break;
	case 0xdb: data = m_shift_param; break;
	case 0xdc: data = m_hsync_width; break;
	case 0xdd: data = m_vsync_width; break;
	case 0xde: data = m_timing_control; break;
	case 0xdf: data = m_func_control; break;
	}

	LOGREGS("%s register_r: 0x%02x = 0x%02x\n", machine().describe_context(), m_register_address, data);

	return data;
}


void f82c425_device::register_w(uint8_t data)
{
	if (m_register_address < 0x12 && (m_register_address & 0xfe) != 0x0e)
		LOGREGS("%s register_w: 0x%02x = 0x%02x\n", machine().describe_context(), m_register_address, data);

	if (m_register_address < 0x12 && (m_register_address & 0xfe) != 0x0e)
		LOGSETUP(" *  %02x <= %3u [%02x] %s\n", m_register_address, data, data, std::array<char const *, 18>
		 {{ "R00 - Horizontal Total",     "R01 - Horizontal Displayed",   "R02 - Horizontal Sync Position",
			"R03 - Ignored",              "R04 - Vertical Total",         "R05 - Vertical Total Adjust",
			"R06 - Vertical Displayed",   "R07 - Vertical Sync Position", "R08 - Ignored",
			"R09 - Maximum Scans/Row",    "R0A - Cursor Start Scan",      "R0B - Cursor End Scan",
			"R0C - Start Address High",   "R0D - Start Address Low",      "R0E - Cursor Address High",
			"R0F - Cursor Address Low",   "R10 - Light Pen High",         "R11 - Light Pen Low" }}[m_register_address]);
	else if (m_register_address >= 0xd8)
		LOGSETUP(" *  %02x <= %3u [%02x] %s\n", m_register_address, data, data, std::array<char const *, 8>
		 {{ "RD8 - Ignored",              "RD9 - AC Control",             "RDA - Threshold",
			"RDB - Shift Parameter",      "RDC - Horizontal Sync Width",  "RDD - Vertical Sync Width",
			"RDE - Timing Control",       "RDF - Function Control" }}[m_register_address & 0x07]);

	switch (m_register_address)
	{
	case 0x00: m_horiz_total       = data; break;
	case 0x01: m_horiz_disp        = data; break;
	case 0x02: m_horiz_sync_pos    = data; break;
	case 0x03: break;
	case 0x04: m_vert_total        = data & 0x7f; break;
	case 0x05: m_vert_total_adj    = data & 0x0f; break;
	case 0x06: m_vert_disp         = data & 0x7f; break;
	case 0x07: m_vert_sync_pos     = data & 0x7f; break;
	case 0x08: break;
	case 0x09: m_max_scan_row      = data & 0x0f; break;
	case 0x0a: m_cursor_start_scan = data & 0x7f; break;
	case 0x0b: m_cursor_end_scan   = data & 0x1f; break;
	case 0x0c: m_disp_start_addr = ((data & 0x3f) << 8) | (m_disp_start_addr & 0x00ff); break;
	case 0x0d: m_disp_start_addr = ((data & 0xff) << 0) | (m_disp_start_addr & 0xff00); break;
	case 0x0e: m_cursor_addr     = ((data & 0x3f) << 8) | (m_cursor_addr & 0x00ff); break;
	case 0x0f: m_cursor_addr     = ((data & 0xff) << 0) | (m_cursor_addr & 0xff00); break;
	case 0x10: break;
	case 0x11: break;
	// extension registers
	case 0xd9: m_ac_control      = data; break;
	case 0xda: m_threshold       = data; break;
	case 0xdb: m_shift_param     = data; break;
	case 0xdc: m_hsync_width     = data; break;
	case 0xdd: m_vsync_width     = data; break;
	case 0xde: m_timing_control  = data; break;
	case 0xdf: m_func_control    = data; m_crt_lcd_cb(BIT(data, 3)); break;
	}
}


uint8_t f82c425_device::extreg_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (offset)
	{
	case 0x00: data = m_mode_control; break;
	case 0x01: data = m_color_select; break;
	case 0x02: m_input_status ^= 9; data = m_input_status; break; // TODO: bits 0/3 behaviour controlled by Function Control
	case 0x03: m_light_pen_latched = false; break;
	case 0x04: /*assert_light_pen_input();*/ break;
	}

	LOGREGS("%s extreg_r:  0x%02x = 0x%02x\n", machine().describe_context(), offset | 0x3d8, data);

	return data;
}


void f82c425_device::extreg_w(offs_t offset, uint8_t data)
{
	LOGREGS("%s extreg_w:  0x%02x = 0x%02x\n", machine().describe_context(), offset | 0x3d8, data);

	LOGSETUP(" * %02x <= %3u [%02x] %s\n", offset | 0x3d8, data, data, std::array<char const *, 5>
		 {{ "3D8 - Mode Control",         "3D9 - Color Select",           "3DA - Input Status",
			"3DB - Clear Light Pen",      "3DC - Set Light Pen" }}[offset]);

	switch (offset)
	{
	case 0x00: m_mode_control      = data; break;
	case 0x01: m_color_select      = data; break;
	case 0x02: break;
	case 0x03: m_light_pen_latched = false; break;
	case 0x04: /*assert_light_pen_input();*/ break;
	}
}


uint8_t f82c425_device::mem_r(offs_t offset)
{
	uint8_t data = 0x00;

	if (BIT(m_func_control, 0)) // Decode Enable
	{
		if (!BIT(m_func_control, 1) || (offset & 0x2000))
			data = space().read_byte(offset);
		else
			data = space().read_byte(offset + 0x4000);
	}

	return data;
}

void f82c425_device::mem_w(offs_t offset, uint8_t data)
{
	if (BIT(m_func_control, 0)) // Decode Enable
	{
		if (!BIT(m_func_control, 1) || (offset & 0x2000))
			space().write_byte(offset, data);
		else
			space().write_byte(offset + 0x4000, data);
	}
}


bool f82c425_device::cursor_visible(uint16_t ma, uint8_t ra)
{
	if (m_cursor_addr == ma && ra >= (m_cursor_start_scan & 0x1f) && ra <= (m_cursor_end_scan & 0x1f) + 1)
		return true;
	else
		return false;
}


void f82c425_device::update_blink_cursor_state(uint64_t frame)
{
	const uint8_t blink_rate = (m_vsync_width >> 4) + 1;

	// blink state
	if (frame % blink_rate == 0)
		m_blink_state = !m_blink_state;

	// cursor state
	switch (m_cursor_start_scan & 0x60)
	{
	case 0x20: // cursor off
		m_cursor_state = false;
		break;

	case 0x00:
	case 0x40: // cursor blinked at blink rate
		if (frame % blink_rate == 0)
			m_cursor_state = !m_cursor_state;
		break;

	case 0x60: // cursor blinked at half blink rate
		if (frame % (blink_rate * 2) == 0)
			m_cursor_state = !m_cursor_state;
		break;
	}
}


uint32_t f82c425_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (BIT(m_mode_control, 3)) // video enable bit
	{
		for (uint16_t y = cliprect.min_y; y <= cliprect.max_y; y++)
		{
			uint8_t ra = y % (m_max_scan_row + 1);

			switch (m_mode_control & 0x13)
			{
			case 0x00: lcd_draw_line_text(bitmap, ra, y, screen.frame_number()); break; // 40 x 25 Text
			case 0x01: lcd_draw_line_text(bitmap, ra, y, screen.frame_number()); break; // 80 x 25 Text
			case 0x02: lcd_draw_line_gfx2(bitmap, ra, y, screen.frame_number()); break; // 320 x 200 Graphics
			case 0x12: lcd_draw_line_gfx1(bitmap, ra, y, screen.frame_number()); break; // 640 x 200 Graphics
			}
		}
	}
	else
	{
		bitmap.fill(0, cliprect);
	}
	return 0;
}


void f82c425_device::lcd_draw_line_text(bitmap_rgb32 &bitmap, uint8_t ra, uint16_t y, uint64_t frame)
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	const uint8_t *disp_ram = (uint8_t *)space().get_read_ptr(m_disp_start_addr);
	const uint8_t *font_ram = (uint8_t *)space().get_read_ptr(0x4000 + (BIT(m_func_control, 2) * 0x1000));
	const uint16_t ma = (y >> 3) * m_horiz_disp;

	uint32_t *p = &bitmap.pix(y);

	// update blink/cursor state for this frame
	if (y == 0) update_blink_cursor_state(frame);

	for (int x = 0; x < m_horiz_disp; x++)
	{
		const uint16_t offset = ((ma + x) << 1) & 0x3fff;
		const uint8_t chr  = disp_ram[offset];
		const uint8_t attr = disp_ram[offset + 1];
		uint8_t data = 0x00;

		// attribute colors
		uint8_t fg = BIT(attr, 0, 3);
		uint8_t bg = BIT(attr, 4, 3);

		// alternate font
		if (BIT(m_func_control, 6) && BIT(attr, 3))
			data = font_ram[(chr * 8) + ra + 0x1000];
		else
			data = font_ram[(chr * 8) + ra];

		// blinking
		if (BIT(m_mode_control, 5) && BIT(attr, 7) && !m_blink_state)
			data = 0x00;

		// cursor
		if (m_cursor_state && cursor_visible(ma + x, ra))
			data = 0xff;

		// inverted video
		if (BIT(m_func_control, 7))
			data ^= 0xff;

		for (int i = 7; i >= 0; i--)
		{
			*p++ = palette[BIT(data, i) ? fg : bg];

			if (m_horiz_disp == 40) // 40 columns double pixel width
				*p++ = palette[BIT(data, i) ? fg : bg];
		}
	}
}


void f82c425_device::lcd_draw_line_gfx2(bitmap_rgb32 &bitmap, uint8_t ra, uint16_t y, uint64_t frame)
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	const uint8_t *disp_ram = (uint8_t *)space().get_read_ptr(m_disp_start_addr);
	const uint16_t ma = (y >> 1) * 80;

	uint32_t *p = &bitmap.pix(y);

	// TODO: implement 4-level gray scale scheme (shouldn't use gray scale palette)

	for (int x = 0; x < 80; x++)
	{
		// even scanlines begin at B8000h, odd scanlines at BA000h
		uint8_t data = disp_ram[((ma + x) & 0x1fff) | ((y & 1) << 13)];

		// inverted video
		if (BIT(m_func_control, 7))
			data ^= 0xff;

		for (int pixel = 0; pixel < 4; pixel++)
		{
			*p++ = palette[((data >> 6) & 3) * 2];
			*p++ = palette[((data >> 6) & 3) * 2];
			data <<= 2;
		}
	}
}


void f82c425_device::lcd_draw_line_gfx1(bitmap_rgb32 &bitmap, uint8_t ra, uint16_t y, uint64_t frame)
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	const uint8_t *disp_ram = (uint8_t *)space().get_read_ptr(m_disp_start_addr);
	const uint16_t ma = (y >> 1) * 80;

	uint32_t *p = &bitmap.pix(y);

	for (int x = 0; x < 80; x++)
	{
		uint8_t data = disp_ram[((ma + x) & 0x1fff) | ((y & 1) << 13)];

		// inverted video
		if (BIT(m_func_control, 7))
			data ^= 0xff;

		for (int pixel = 0; pixel < 8; pixel++)
		{
			*p++ = palette[BIT(data, 7) ? 7 : 0];
			data <<= 1;
		}
	}
}
