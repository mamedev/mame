// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MOS Technology 8563 VDC emulation

**********************************************************************/

#include "emu.h"
#include "mos8563.h"

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

	m_supports_status_reg_d5 = true;
	m_supports_status_reg_d6 = true;
	m_supports_status_reg_d7 = true;
	m_update_ready_bit = 1;

	// default update_row delegate
	m_update_row_cb.set(*this, FUNC(mos8563_device::vdc_update_row));

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
	m_attr_row_addr = 0;
	m_bitmap_addr = 0;
	m_dram_refresh = 0;
	m_sync_polarity = 0;

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
	save_item(NAME(m_attr_row_addr));
	save_item(NAME(m_bitmap_addr));
	save_item(NAME(m_dram_refresh));
	save_item(NAME(m_sync_polarity));
	save_item(NAME(m_revision));
	save_item(NAME(m_clk_scale));
	save_item(NAME(m_char_blink_state));
	save_item(NAME(m_char_blink_count));
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


uint8_t mos8563_device::draw_scanline(int y, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (y == 0)
	{
		m_draw_ra = m_vert_scroll & 0x1f;
		m_attr_row_addr = m_attribute_addr;
		m_bitmap_addr = m_current_disp_addr + (m_vert_scroll & 0x1f) * (m_horiz_disp + m_row_addr_incr);
	}

	uint8_t ra = m_draw_ra;

	int cursor_visible = check_cursor_visible(ra, m_current_disp_addr);
	int8_t cursor_x = cursor_visible ? (m_cursor_addr - m_current_disp_addr) : -1;
	int de = (y <= m_max_visible_y) ? 1 : 0;
	int vbp = m_vert_pix_total - m_vsync_off_pos;
	if (vbp < 0) vbp = 0;
	int hbp = m_horiz_pix_total - m_hsync_off_pos;
	if (hbp < 0) hbp = 0;

	m_update_row_cb(bitmap, cliprect, HSS_BITMAP ? m_bitmap_addr : m_current_disp_addr, ra, y, m_horiz_disp, cursor_x, de, hbp, vbp);

	m_bitmap_addr = (m_bitmap_addr + m_horiz_disp + m_row_addr_incr) & 0xffff;

	if (ra == m_max_ras_addr)
	{
		m_draw_ra = 0;
		m_current_disp_addr = (m_current_disp_addr + m_horiz_disp + m_row_addr_incr) & 0xffff;
		m_attr_row_addr = (m_attr_row_addr + m_horiz_disp + m_row_addr_incr) & 0xffff;
	}
	else
	{
		m_draw_ra = (ra + 1) & 0x1f;
	}

	return ra;
}


uint32_t mos8563_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(pen(m_color & 0x0f), cliprect);

	if (((m_de_begin > m_horiz_char_total) || (m_de_begin == m_de_end)) && (m_de_end <= m_horiz_char_total))
		return 0;

	return mc6845_device::screen_update(screen, bitmap, cliprect);
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


MC6845_UPDATE_ROW( mos8563_device::vdc_update_row )
{
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

	for (int column = 0; column < x_count; column++)
	{
		uint8_t code = read_videoram(ma + column);
		uint8_t attr = 0;

		int fg = m_color >> 4;
		int bg = m_color & 0x0f;

		if (HSS_ATTR)
		{
			offs_t attr_addr = m_attr_row_addr + column;
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

			if (x >= 0 && x < x_count * cth && bitmap.cliprect().contains(hbp + x, vbp + y))
				bitmap.pix(vbp + y, hbp + x) = pen(de ? color : (m_color & 0x0f));
		}
	}
}
