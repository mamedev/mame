// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MOS Technology 8563 VDC emulation

**********************************************************************/

#include "emu.h"
#include "mos8563.h"

#include "screen.h"

#include <algorithm>

#define LOG_REGS    (1U << 1)

//#define VERBOSE (LOG_REGS)
#include "logmacro.h"

#define LOGREGS(...) LOGMASKED(LOG_REGS, __VA_ARGS__)


#define VSS_CBRATE                  BIT(m_vert_scroll, 5)
#define VSS_RVS                     BIT(m_vert_scroll, 6)
#define VSS_COPY                    BIT(m_vert_scroll, 7)

#define HSS_DBL                     BIT(m_horiz_scroll, 4)
#define HSS_SEMI                    BIT(m_horiz_scroll, 5)
#define HSS_ATTR                    BIT(m_horiz_scroll, 6)
#define HSS_BITMAP                  BIT(m_horiz_scroll, 7)

#define ATTR_COLOR                  (attr & 0x0f)
#define ATTR_BACKGROUND             (attr >> 4)
#define ATTR_FOREGROUND             (attr & 0x0f)
#define ATTR_BLINK                  BIT(attr, 4)
#define ATTR_UNDERLINE              BIT(attr, 5)
#define ATTR_REVERSE                BIT(attr, 6)
#define ATTR_ALTERNATE_CHARSET      BIT(attr, 7)



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MOS8563, mos8563_device, "mos8563", "MOS 8563 VDC")
DEFINE_DEVICE_TYPE(MOS8568, mos8568_device, "mos8568", "MOS 8568 VDC")

// default address maps
void mos8563_device::mos8563_videoram_map(address_map &map)
{
	if (!has_configured_map(0))
		map(0x0000, 0xffff).ram();
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mos8563_device - constructor
//-------------------------------------------------

mos8563_device::mos8563_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: mc6845_device(mconfig, type, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, device_palette_interface(mconfig, *this)
	, m_videoram_space_config("videoram", ENDIANNESS_LITTLE, 8, 16, 0, address_map_constructor(FUNC(mos8563_device::mos8563_videoram_map), this))
{
	m_clk_scale = 8;
}

mos8563_device::mos8563_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mos8563_device(mconfig, MOS8563, tag, owner, clock)
{
}


//-------------------------------------------------
//  mos8568_device - constructor
//-------------------------------------------------

mos8568_device::mos8568_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mos8563_device(mconfig, MOS8568, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mos8563_device::device_start()
{
	mc6845_device::device_start();

	/* create the timers */
	m_block_copy_timer = timer_alloc(FUNC(mos8563_device::block_copy_tick), this);

	m_line_counter_mask = 0xff;
	m_interlace_fields = true;
	m_latch_row_end = true;
	m_supports_vert_sync_width = true;
	m_supports_status_reg_d5 = true;
	m_supports_status_reg_d6 = true;
	m_supports_status_reg_d7 = true;
	m_update_ready_bit = 1;

	m_char_blink_state = false;
	m_char_blink_count = 0;
	m_attribute_addr = 0;
	m_horiz_char = 0x78;
	m_vert_char_disp = 0;
	m_vert_scroll = 0;
	m_horiz_scroll = 0;
	m_color = 0;
	m_row_addr_incr = 0;
	m_char_base_addr = 0;
	m_underline_ras = 0;
	m_word_count = 0;
	m_data = 0;
	m_block_addr = 0;
	m_de_begin = 0;
	m_de_end = 0;
	m_draw_ra = 0;
	m_draw_row_ra = 0;
	m_draw_row = 0;
	m_draw_addr = 0;
	m_attr_row_addr = 0;
	m_bitmap_addr = 0;
	m_dram_refresh = 0;
	m_sync_polarity = 0;
	m_frame_started = false;
	m_line_index = 0;
	m_frame_lines = 0;
	m_field_lines = 0;
	m_screen_line = 0;
	m_retrace_lines = 0;
	m_latched_disp_addr = 0;
	m_latched_attr_addr = 0;
	m_frame_disp_addr = 0;
	m_frame_attr_addr = 0;
	m_start_addr_latched = false;
	m_vsync_seen = false;

	std::fill_n(m_line_addr, LINE_COUNT, 0);
	std::fill_n(m_line_bitmap_addr, LINE_COUNT, 0);
	std::fill_n(m_line_attr_addr, LINE_COUNT, 0);
	std::fill_n(m_line_ra, LINE_COUNT, 0);
	std::fill_n(m_line_shown, LINE_COUNT, false);
	std::fill_n(m_line_field, LINE_COUNT, 0);

	m_revision = 1;

	// initialize video RAM
	uint8_t data = 0xff;

	for (offs_t offset = 0; offset < 0x10000; offset++)
	{
		space(0).write_byte(offset, data);
		data ^= 0xff;
	}

	// VICE palette
	set_pen_color(0, rgb_t::black());
	set_pen_color(1, rgb_t(0x55, 0x55, 0x55));
	set_pen_color(2, rgb_t(0x00, 0x00, 0xaa));
	set_pen_color(3, rgb_t(0x55, 0x55, 0xff));
	set_pen_color(4, rgb_t(0x00, 0xaa, 0x00));
	set_pen_color(5, rgb_t(0x55, 0xff, 0x55));
	set_pen_color(6, rgb_t(0x00, 0xaa, 0xaa));
	set_pen_color(7, rgb_t(0x55, 0xff, 0xff));
	set_pen_color(8, rgb_t(0xaa, 0x00, 0x00));
	set_pen_color(9, rgb_t(0xff, 0x55, 0x55));
	set_pen_color(10, rgb_t(0xaa, 0x00, 0xaa));
	set_pen_color(11, rgb_t(0xff, 0x55, 0xff));
	set_pen_color(12, rgb_t(0xaa, 0x55, 0x00));
	set_pen_color(13, rgb_t(0xff, 0xff, 0x55));
	set_pen_color(14, rgb_t(0xaa, 0xaa, 0xaa));
	set_pen_color(15, rgb_t::white());

	save_item(NAME(m_char_buffer));
	save_item(NAME(m_attr_buffer));
	save_item(NAME(m_attribute_addr));
	save_item(NAME(m_horiz_char));
	save_item(NAME(m_vert_char_disp));
	save_item(NAME(m_vert_scroll));
	save_item(NAME(m_horiz_scroll));
	save_item(NAME(m_color));
	save_item(NAME(m_row_addr_incr));
	save_item(NAME(m_char_base_addr));
	save_item(NAME(m_underline_ras));
	save_item(NAME(m_word_count));
	save_item(NAME(m_data));
	save_item(NAME(m_block_addr));
	save_item(NAME(m_de_begin));
	save_item(NAME(m_de_end));
	save_item(NAME(m_draw_ra));
	save_item(NAME(m_draw_row_ra));
	save_item(NAME(m_draw_row));
	save_item(NAME(m_draw_addr));
	save_item(NAME(m_attr_row_addr));
	save_item(NAME(m_bitmap_addr));
	save_item(NAME(m_dram_refresh));
	save_item(NAME(m_sync_polarity));
	save_item(NAME(m_revision));
	save_item(NAME(m_clk_scale));
	save_item(NAME(m_char_blink_state));
	save_item(NAME(m_char_blink_count));
	save_item(NAME(m_frame_started));
	save_item(NAME(m_line_index));
	save_item(NAME(m_frame_lines));
	save_item(NAME(m_field_lines));
	save_item(NAME(m_screen_line));
	save_item(NAME(m_retrace_lines));
	save_item(NAME(m_latched_disp_addr));
	save_item(NAME(m_latched_attr_addr));
	save_item(NAME(m_frame_disp_addr));
	save_item(NAME(m_frame_attr_addr));
	save_item(NAME(m_start_addr_latched));
	save_item(NAME(m_vsync_seen));
	save_item(NAME(m_line_addr));
	save_item(NAME(m_line_bitmap_addr));
	save_item(NAME(m_line_attr_addr));
	save_item(NAME(m_line_ra));
	save_item(NAME(m_line_shown));
	save_item(NAME(m_line_field));
}

void mos8568_device::device_start()
{
	mos8563_device::device_start();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mos8563_device::device_reset()
{
	mc6845_device::device_reset();

	m_sync_polarity = 0xc0;
}

void mos8568_device::device_reset() { mos8563_device::device_reset(); }


void mos8563_device::device_post_load()
{
	mc6845_device::device_post_load();

	update_screen_geometry();
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector mos8563_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_videoram_space_config),
	};
}


offs_t mos8563_device::vram_address(offs_t offset) const
{
	if (BIT(m_char_base_addr, 4))
		return offset & 0xffff;
	else
		return (offset & 0x80ff) | ((offset & 0x3f00) << 1) | (offset & 0x0100);
}

inline uint8_t mos8563_device::read_videoram(offs_t offset)
{
	return space(0).read_byte(vram_address(offset));
}

inline void mos8563_device::write_videoram(offs_t offset, uint8_t data)
{
	space(0).write_byte(vram_address(offset), data);
}


void mos8563_device::address_w(uint8_t data)
{
	m_register_address_latch = data & 0x3f;
}


uint8_t mos8563_device::status_r()
{
	uint8_t ret = m_revision;

	/* VBLANK bit */
	if (!m_line_enable_ff)
		ret = ret | 0x20;

	/* light pen latched */
	if (m_light_pen_latched)
		ret = ret | 0x40;

	/* UPDATE ready */
	if (m_update_ready_bit)
		ret = ret | 0x80;

	return ret;
}


uint8_t mos8563_device::register_r()
{
	uint8_t ret = 0xff;

	switch (m_register_address_latch)
	{
		case 0x00:  ret = m_horiz_char_total; break;
		case 0x01:  ret = m_horiz_disp; break;
		case 0x02:  ret = m_horiz_sync_pos; break;
		case 0x03:  ret = m_sync_width; break;
		case 0x04:  ret = m_vert_char_total; break;
		case 0x05:  ret = m_vert_total_adj | 0xc0; break;
		case 0x06:  ret = m_vert_disp; break;
		case 0x07:  ret = m_vert_sync_pos; break;
		case 0x08:  ret = m_mode_control | 0xfc; break;
		case 0x09:  ret = m_max_ras_addr | 0xe0; break;
		case 0x0a:  ret = m_cursor_start_ras | 0x80; break;
		case 0x0b:  ret = m_cursor_end_ras | 0xe0; break;
		case 0x0c:  ret = (m_disp_start_addr >> 8) & 0xff; break;
		case 0x0d:  ret = (m_disp_start_addr >> 0) & 0xff; break;
		case 0x0e:  ret = (m_cursor_addr     >> 8) & 0xff; break;
		case 0x0f:  ret = (m_cursor_addr     >> 0) & 0xff; break;
		case 0x10:  ret = (m_light_pen_addr  >> 8) & 0xff; m_light_pen_latched = false; break;
		case 0x11:  ret = (m_light_pen_addr  >> 0) & 0xff; m_light_pen_latched = false; break;
		case 0x12:  ret = (m_update_addr     >> 8) & 0xff; break;
		case 0x13:  ret = (m_update_addr     >> 0) & 0xff; break;
		case 0x14:  ret = (m_attribute_addr  >> 8) & 0xff; break;
		case 0x15:  ret = (m_attribute_addr  >> 0) & 0xff; break;
		case 0x16:  ret = m_horiz_char; break;
		case 0x17:  ret = m_vert_char_disp | 0xe0; break;
		case 0x18:  ret = m_vert_scroll; break;
		case 0x19:  ret = m_horiz_scroll; break;
		case 0x1a:  ret = m_color; break;
		case 0x1b:  ret = m_row_addr_incr; break;
		case 0x1c:  ret = m_char_base_addr | 0x0f; break;
		case 0x1d:  ret = m_underline_ras | 0xe0; break;
		case 0x1e:  ret = m_word_count; break;
		case 0x1f:  ret = read_videoram(m_update_addr++); break;
		case 0x20:  ret = (m_block_addr      >> 8) & 0xff; break;
		case 0x21:  ret = (m_block_addr      >> 0) & 0xff; break;
		case 0x22:  ret = m_de_begin; break;
		case 0x23:  ret = m_de_end; break;
		case 0x24:  ret = m_dram_refresh | 0xf0; break;
		case 0x25:  ret = m_sync_polarity | 0x3f; break;
	}

	return ret;
}


void mos8563_device::register_w(uint8_t data)
{
	LOGREGS("%s:MOS8563 reg 0x%02x = 0x%02x\n", machine().describe_context(), m_register_address_latch, data);

	bool const display_reg = (m_register_address_latch < 0x10) || ((m_register_address_latch > 0x13) && (m_register_address_latch < 0x1e)) || (m_register_address_latch > 0x21);

	if (display_reg && has_screen())
		screen().update_partial(screen().vpos());

	switch (m_register_address_latch)
	{
		case 0x00:  m_horiz_char_total =   data & 0xff; break;
		case 0x01:  m_horiz_disp       =   data & 0xff; break;
		case 0x02:  m_horiz_sync_pos   =   data & 0xff; break;
		case 0x03:  m_sync_width       =   data & 0xff; break;
		case 0x04:  m_vert_char_total  =   data & 0xff; break;
		case 0x05:  m_vert_total_adj   =   data & 0x1f; break;
		case 0x06:  m_vert_disp        =   data & 0xff; break;
		case 0x07:  m_vert_sync_pos    =   data & 0xff; break;
		case 0x08:  m_mode_control     =   data & 0x03; break;
		case 0x09:  m_max_ras_addr     =   data & 0x1f; break;
		case 0x0a:  m_cursor_start_ras =   data & 0x7f; break;
		case 0x0b:  m_cursor_end_ras   =   data & 0x1f; break;
		case 0x0c:  m_disp_start_addr  = ((data & 0xff) << 8) | (m_disp_start_addr & 0x00ff); break;
		case 0x0d:  m_disp_start_addr  = ((data & 0xff) << 0) | (m_disp_start_addr & 0xff00); break;
		case 0x0e:  m_cursor_addr      = ((data & 0xff) << 8) | (m_cursor_addr & 0x00ff); break;
		case 0x0f:  m_cursor_addr      = ((data & 0xff) << 0) | (m_cursor_addr & 0xff00); break;
		case 0x10: /* read-only */ break;
		case 0x11: /* read-only */ break;
		case 0x12:  m_update_addr      = ((data & 0xff) << 8) | (m_update_addr & 0x00ff); break;
		case 0x13:  m_update_addr      = ((data & 0xff) << 0) | (m_update_addr & 0xff00); break;
		case 0x14:  m_attribute_addr   = ((data & 0xff) << 8) | (m_attribute_addr & 0x00ff); break;
		case 0x15:  m_attribute_addr   = ((data & 0xff) << 0) | (m_attribute_addr & 0xff00); break;
		case 0x16:
			m_horiz_char = data & 0xff;
			update_char_width();
			break;
		case 0x17:  m_vert_char_disp   =   data & 0x1f; break;
		case 0x18:  m_vert_scroll      =   data & 0xff; break;
		case 0x19:
			m_horiz_scroll = data & 0xff;
			update_char_width();
			break;
		case 0x1a:  m_color            =   data & 0xff; break;
		case 0x1b:  m_row_addr_incr    =   data & 0xff; break;
		case 0x1c:  m_char_base_addr   =   data & 0xf0; break;
		case 0x1d:  m_underline_ras    =   data & 0x1f; break;
		case 0x1e:
			m_word_count = data & 0xff;
			m_update_ready_bit = 0;
			m_block_copy_timer->adjust(cclks_to_attotime(1));
			break;
		case 0x1f:
			m_data = data & 0xff;
			write_videoram(m_update_addr++, m_data);
			break;
		case 0x20:  m_block_addr       = ((data & 0xff) << 8) | (m_block_addr & 0x00ff); break;
		case 0x21:  m_block_addr       = ((data & 0xff) << 0) | (m_block_addr & 0xff00); break;
		case 0x22:  m_de_begin         =   data & 0xff; break;
		case 0x23:  m_de_end           =   data & 0xff; break;
		case 0x24:  m_dram_refresh     =   data & 0x0f; break;
		case 0x25:  m_sync_polarity    =   data & 0xc0; break;
	}

	recompute_parameters(false);
}


TIMER_CALLBACK_MEMBER(mos8563_device::block_copy_tick)
{
	uint8_t data = VSS_COPY ? read_videoram(m_block_addr++) : m_data;

	write_videoram(m_update_addr++, data);

	if (--m_word_count)
	{
		m_block_copy_timer->adjust(cclks_to_attotime(1));
	}
	else
	{
		m_update_ready_bit = 1;
	}
}


void mos8563_device::update_char_width()
{
	int const total = m_horiz_char >> 4;
	int const clocks = std::max(HSS_DBL ? (total * 2) : (total + 1), 1);
	int const pixels = std::max(HSS_DBL ? total : (total + 1), 1);

	if ((clocks != m_clk_scale) || (pixels != m_hpixels_per_column))
	{
		m_clk_scale = clocks;
		set_hpixels_per_column(pixels);
		recompute_parameters(true);
	}
}


void mos8563_device::update_cursor_state()
{
	mc6845_device::update_cursor_state();

	/* save and increment character blink counter */
	uint8_t last_char_blink_count = m_char_blink_count;
	m_char_blink_count++;

	/* switch on character blinking mode */
	if (VSS_CBRATE)
	{
		if ((last_char_blink_count & 0x20) != (m_char_blink_count & 0x20))
			m_char_blink_state = !m_char_blink_state;
	}
	else
	{
		if ((last_char_blink_count & 0x10) != (m_char_blink_count & 0x10))
			m_char_blink_state = !m_char_blink_state;
	}
}


void mos8563_device::update_screen_geometry()
{
	if (!has_screen() || !m_has_valid_parameters || (m_frame_lines < 2))
		return;

	int const width = m_horiz_pix_total;
	int const height = interlace_video() ? (m_frame_lines * 2) : m_frame_lines;

	// a monitor shows neither the vertical sync pulse nor its retrace after it
	int const vsync_lines = (m_sync_width >> 4) ? (m_sync_width >> 4) : 16;
	int const visible_lines = std::max(m_frame_lines - vsync_lines - RETRACE_LINES, 2);

	// nor the horizontal blanking around the sync pulse, which ends at x = 0
	int const min_x = std::min<int>(RETRACE_DOTS, width - 2);
	int const max_x = std::max<int>(min_x, width - (m_hsync_off_pos - m_hsync_on_pos) - FRONT_PORCH_DOTS - 1);
	rectangle const visarea(min_x, max_x, 0, (interlace_video() ? (visible_lines * 2) : visible_lines) - 1);
	attotime const refresh = cclks_to_attotime((m_horiz_char_total + 1) * m_frame_lines);

	if ((width != screen().width()) || (height != screen().height()) || (visarea != screen().visible_area()) || (refresh != screen().frame_period()))
		screen().configure(width, height, visarea, refresh);

	if (interlace_video() && ((m_field_bitmap.width() != width) || (m_field_bitmap.height() != height)))
	{
		m_field_bitmap.allocate(width, height);
		m_field_bitmap.fill(pen(m_color & 0x0f));
	}
}


void mos8563_device::configure_screen(int width, int height, const rectangle &visarea, const attotime &refresh)
{
	if (!m_frame_lines)
		mc6845_device::configure_screen(width, height, visarea, refresh);
}


void mos8563_device::frame_start()
{
	if (m_frame_started)
	{
		int const lines = m_line_index + 1;

		m_frame_lines = interlace_video() ? std::min(lines, m_field_lines) : lines;
		m_field_lines = lines;
	}

	m_frame_started = true;
	m_line_index = -1;

	m_frame_disp_addr = m_start_addr_latched ? m_latched_disp_addr : m_disp_start_addr;
	m_frame_attr_addr = m_start_addr_latched ? m_latched_attr_addr : m_attribute_addr;
	m_start_addr_latched = false;

	if (!m_vsync_seen)
	{
		m_screen_line = 0;
		update_screen_geometry();

		if (has_screen())
			screen().reset_origin();
	}

	m_vsync_seen = false;
}


void mos8563_device::set_vsync(int state)
{
	bool const vsync_end = m_vsync && !state;

	mc6845_device::set_vsync(state);

	if (!m_line_enable_ff && !m_start_addr_latched)
	{
		m_latched_disp_addr = m_disp_start_addr;
		m_latched_attr_addr = m_attribute_addr;
		m_start_addr_latched = true;
	}

	if (vsync_end)
	{
		m_vsync_seen = true;
		m_retrace_lines = RETRACE_LINES + 1;
	}

	if (m_retrace_lines && !--m_retrace_lines)
	{
		// called once per scanline, so the screen origin moves to the first line the monitor shows after its retrace
		m_screen_line = 0;
		update_screen_geometry();

		if (has_screen())
			screen().reset_origin();
	}

	start_draw_line();
}


void mos8563_device::reload_draw_counters()
{
	int const field = interlace_video() ? m_field : 0;

	m_draw_ra = ((m_vert_scroll & 0x1f) + field) & 0x1f;
	m_draw_row_ra = field;
	m_draw_row = 0;

	m_draw_addr = m_frame_disp_addr;

	if (!(field && HSS_BITMAP))
	{
		if (interlace_video() && HSS_BITMAP)
			m_attr_row_addr = m_frame_attr_addr - (m_horiz_disp + m_row_addr_incr);
		else
			m_attr_row_addr = m_frame_attr_addr - ((m_vert_char_total == 0xff) ? m_horiz_disp : 0);
		m_bitmap_addr = m_draw_addr + (m_vert_scroll & 0x1f) * (m_horiz_disp + m_row_addr_incr);
	}
}


void mos8563_device::advance_draw_line()
{
	bool const interlace = interlace_video();
	uint8_t const ra = m_draw_ra;

	m_bitmap_addr = (m_bitmap_addr + m_horiz_disp + m_row_addr_incr) & 0xffff;

	if (interlace ? ((ra >= m_max_ras_addr) || ((ra + 1) == m_max_ras_addr)) : (ra == m_max_ras_addr))
	{
		m_draw_ra = interlace ? (~(ra ^ m_max_ras_addr) & 1) : 0;
		m_draw_addr = (m_draw_addr + m_horiz_disp + m_row_addr_incr) & 0xffff;
		m_attr_row_addr = (m_attr_row_addr + m_horiz_disp + m_row_addr_incr) & 0xffff;
	}
	else
	{
		m_draw_ra = (ra + (interlace ? 2 : 1)) & 0x1f;
	}

	if (interlace ? ((m_draw_row_ra >= m_max_ras_addr) || ((m_draw_row_ra + 1) == m_max_ras_addr)) : (m_draw_row_ra == m_max_ras_addr))
	{
		m_draw_row_ra = interlace ? (~(m_draw_row_ra ^ m_max_ras_addr) & 1) : 0;
		m_draw_row++;
	}
	else
	{
		m_draw_row_ra = (m_draw_row_ra + (interlace ? 2 : 1)) & 0x1f;
	}
}


void mos8563_device::start_draw_line()
{
	if (m_line_index < LINE_COUNT)
		m_line_index++;

	if (!m_line_index)
		reload_draw_counters();

	bool const shown = m_draw_row < m_vert_disp;

	if (m_screen_line < LINE_COUNT)
	{
		m_line_addr[m_screen_line] = m_draw_addr;
		m_line_bitmap_addr[m_screen_line] = m_bitmap_addr;
		m_line_attr_addr[m_screen_line] = m_attr_row_addr;
		m_line_ra[m_screen_line] = m_draw_ra;
		m_line_shown[m_screen_line] = shown;
		m_line_field[m_screen_line] = m_field;
		m_screen_line++;
	}

	if (shown)
	{
		advance_draw_line();
	}
	else
	{
		// FIXME: fitted, not from hardware: in interlace the address counter runs into the row after R6, all but its last line, unless R6 = R4
		bool const extra_row = interlace_video() && (m_vert_disp < m_vert_char_total) && (m_draw_row == m_vert_disp);
		bool const row_last_line = (m_draw_row_ra >= m_max_ras_addr) || ((m_draw_row_ra + 1) == m_max_ras_addr);

		if (extra_row && !row_last_line)
			advance_draw_line();
	}
}


void mos8563_device::draw_line(int line, int row, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if ((line >= LINE_COUNT) || !m_line_shown[line])
		return;

	uint16_t const addr = m_line_addr[line];
	uint8_t const ra = m_line_ra[line];
	bool const cursor_visible = check_cursor_visible(ra, addr);
	int8_t const cursor_x = cursor_visible ? (m_cursor_addr - addr) : -1;
	int const hbp = std::max(m_horiz_pix_total - m_hsync_off_pos, 0);

	draw_row(bitmap, cliprect, HSS_BITMAP ? m_line_bitmap_addr[line] : addr, m_line_attr_addr[line], ra, row, cursor_x, hbp);
}


uint32_t mos8563_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bool const interlace = interlace_video() && (m_field_bitmap.height() == screen.height());
	bitmap_rgb32 &dest = interlace ? m_field_bitmap : bitmap;
	rgb_t const border = pen(m_color & 0x0f);

	if (!interlace)
		bitmap.fill(border, cliprect);

	if (m_has_valid_parameters)
	{
		bool const blank = ((m_de_begin > m_horiz_char_total) || (m_de_begin == m_de_end)) && (m_de_end <= m_horiz_char_total);

		for (int row = cliprect.min_y; row <= cliprect.max_y; row++)
		{
			if (interlace)
			{
				// the field that continues the bitmap address is woven half a line above the one that reloads it
				if (((row >> 1) >= LINE_COUNT) || ((row & 1) == m_line_field[row >> 1]))
					continue;

				dest.plot_box(cliprect.min_x, row, cliprect.width(), 1, border);

				if (!blank)
					draw_line(row >> 1, row, dest, cliprect);
			}
			else if (!blank)
			{
				draw_line(row, row, dest, cliprect);
			}
		}
	}

	if (interlace)
		copybitmap(bitmap, m_field_bitmap, 0, 0, 0, 0, cliprect);

	return 0;
}


int mos8563_device::displayed_width(uint8_t r22)
{
	int const total = r22 >> 4;
	int const disp = r22 & 0x0f;

	if (total < 8)
		return (disp < total) ? (disp + 1) : (disp == total) ? 0 : (total + 1);
	else if (disp < 7)
		return std::min(8, disp + total - 6);
	else if (disp == 7)
		return 0;
	else
		return (disp <= total) ? (disp - 7) : 8;
}


bool mos8563_device::semigraphics_gap(uint8_t r22)
{
	int const total = r22 >> 4;
	int const disp = r22 & 0x0f;

	return (total >= 7) && (((disp < 8) && (disp + total <= 14)) || ((disp >= 8) && (disp <= total)));
}


void mos8563_device::draw_row(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint16_t ma, uint16_t attr_row_addr, uint8_t ra, int y, int8_t cursor_x, int hbp)
{
	int const x_count = m_horiz_disp;

	uint8_t cth = (m_horiz_char >> 4) + (HSS_DBL ? 0 : 1);
	uint8_t cdv = m_vert_char_disp;

	if ((m_horiz_scroll & 0x0f) + HSS_DBL > (m_horiz_char >> 4))
		return;

	uint8_t r22 = m_horiz_char;
	if (HSS_DBL && !BIT(r22, 3) && (r22 >= 0x10))
		r22 -= 0x10;

	int displayed = displayed_width(r22);
	uint8_t semi_test = 0;
	uint8_t semi_type = 0;
	bool const semi_forced = HSS_DBL && ((m_horiz_char >> 4) == (m_horiz_char & 0x0f));

	if (HSS_SEMI || semi_forced)
	{
		if (semi_forced)
		{
			if ((m_horiz_char & 0x0f) <= 8)
				displayed = 1;
		}
		else if (!displayed)
		{
			displayed = 8;
		}

		semi_test = 0x80 >> (displayed - 1);
		semi_type = semigraphics_gap(r22) ? 0xff : 0x00;
	}

	uint8_t const dmask = ~(0xff >> std::min(displayed, 8));
	int const gap_total = (m_horiz_char >> 4) - 6;
	uint8_t const d2mask = (gap_total > (HSS_DBL ? 2 : 1)) ? ~(0xff >> std::min(gap_total, 8)) : 0x00;

	// one character beyond R1 is fetched to fill the pixels uncovered at the right edge by the horizontal smooth scroll
	for (int column = 0; column <= x_count; column++)
	{
		uint8_t code = read_videoram(ma + column);
		uint8_t attr = 0;

		int fg = m_color >> 4;
		int bg = m_color & 0x0f;

		if (HSS_ATTR)
		{
			offs_t attr_addr = attr_row_addr + column;
			attr = read_videoram(attr_addr);
		}

		uint8_t data;
		uint8_t gap = 0;

		if (HSS_BITMAP)
		{
			if (HSS_ATTR)
			{
				fg = ATTR_FOREGROUND;
				bg = ATTR_BACKGROUND;
			}

			data = (ra > cdv) ? 0 : code;
			data &= dmask;

			if (data & semi_test)
			{
				data |= semi_test - 1;
				gap = semi_type;
			}
		}
		else
		{
			if (HSS_ATTR)
			{
				fg = ATTR_COLOR;
			}

			offs_t font_addr;

			if (m_max_ras_addr < 16)
			{
				font_addr = ((m_char_base_addr & 0xe0) << 8) + (ATTR_ALTERNATE_CHARSET << 12) + (code << 4) + ra;
			}
			else
			{
				font_addr = ((m_char_base_addr & 0xc0) << 8) + (ATTR_ALTERNATE_CHARSET << 13) + (code << 5) + ra;
			}

			data = (ra > cdv) ? 0 : read_videoram(font_addr);
			data &= dmask;

			if (ATTR_UNDERLINE && (ra == m_underline_ras)) { data = 0xff; gap = 0xff; }
			if (ATTR_BLINK && !m_char_blink_state) { data = 0; gap = 0; }

			if (data & semi_test)
			{
				data |= semi_test - 1;
				gap = semi_type;
			}

			if (ATTR_REVERSE) { data ^= 0xff; gap ^= 0xff; }
			if (column == cursor_x) { data ^= 0xff; gap ^= 0xff; }
		}

		if (VSS_RVS) { data ^= 0xff; gap ^= 0xff; }

		gap &= d2mask;

		for (int bit = 0; bit < cth; bit++)
		{
			int x = (m_horiz_scroll & 0x0f) - cth + 1 + (column * cth) + bit;
			int color = BIT((bit < 8) ? data : gap, 7 - (bit & 7)) ? fg : bg;

			if (x >= 0 && x < x_count * cth && cliprect.contains(hbp + x, y))
				bitmap.pix(y, hbp + x) = pen(color);
		}
	}
}
