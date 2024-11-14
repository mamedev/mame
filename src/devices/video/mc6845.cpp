// license:BSD-3-Clause
// copyright-holders:Wilbert Pol,Nigel Barnes
/**********************************************************************

    Motorola MC6845 and compatible CRT controller emulation

    The following variations exist that are different in
    functionality and not just in speed rating(1):
        * Motorola 6845, 6845-1
        * Hitachi 6845 (= 46505R), 6845S (= 46505S), 6345/6445
        * Rockwell 6545, 6545-1 (= Synertek SY6545-1)
        * MOS Technology 6545-1

    (1) as per the document at
    http://www.6502.org/users/andre/hwinfo/crtc/diffs.html

    The various speed rated devices are usually identified by a
    letter, e.g. MC68A45, MC68B45. Hitachi's older HD46505 numbering
    identifies speed by numerical suffixes (-1, -2), which other
    manufacturers use to identify functional variants instead.

    The chip is originally designed by Hitachi, not by Motorola.

**********************************************************************/

/*

    TODO:

    - Change device video emulation x/y offsets when "show border color"
      is true
    - Support 'interlace and video' mode

    - hd6345
        - smooth scrolling
        - second cursor
        - interrupt request

*/

#include "emu.h"
#include "mc6845.h"

#include "screen.h"

#define LOG_SETUP   (1U << 1)
#define LOG_REGS    (1U << 2)
#define LOG_CONF    (1U << 3)

//#define VERBOSE (LOG_SETUP|LOG_CONF|LOG_REGS)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

#define LOGSETUP(...)   LOGMASKED(LOG_SETUP,  __VA_ARGS__)
#define LOGREGS(...)    LOGMASKED(LOG_REGS,  __VA_ARGS__)
#define LOGCONF(...)    LOGMASKED(LOG_CONF,  __VA_ARGS__)

DEFINE_DEVICE_TYPE(MC6845,   mc6845_device,   "mc6845",   "Motorola MC6845 CRTC")
DEFINE_DEVICE_TYPE(MC6845_1, mc6845_1_device, "mc6845_1", "Motorola MC6845-1 CRTC")
DEFINE_DEVICE_TYPE(R6545_1,  r6545_1_device,  "r6545_1",  "Rockwell R6545-1 CRTC")
DEFINE_DEVICE_TYPE(C6545_1,  c6545_1_device,  "c6545_1",  "C6545-1 CRTC")
DEFINE_DEVICE_TYPE(HD6845S,  hd6845s_device,  "hd6845s",  "Hitachi HD6845S CRTC") // same as HD46505S
DEFINE_DEVICE_TYPE(SY6545_1, sy6545_1_device, "sy6545_1", "Synertek SY6545-1 CRTC")
DEFINE_DEVICE_TYPE(SY6845E,  sy6845e_device,  "sy6845e",  "Synertek SY6845E CRTC")
DEFINE_DEVICE_TYPE(HD6345,   hd6345_device,   "hd6345",   "Hitachi HD6345 CRTC-II")
DEFINE_DEVICE_TYPE(AMS40489, ams40489_device, "ams40489", "AMS40489 ASIC (CRTC)")


/* mode macros */
#define MODE_TRANSPARENT            ((m_mode_control & 0x08) != 0)
#define MODE_TRANSPARENT_PHI2       ((m_mode_control & 0x88) == 0x88)
/* FIXME: not supported yet */
#define MODE_TRANSPARENT_BLANK      ((m_mode_control & 0x88) == 0x08)
#define MODE_UPDATE_STROBE          ((m_mode_control & 0x40) != 0)
#define MODE_CURSOR_SKEW            ((m_mode_control & 0x20) != 0)
#define MODE_DISPLAY_ENABLE_SKEW    ((m_mode_control & 0x10) != 0)
#define MODE_ROW_COLUMN_ADDRESSING  ((m_mode_control & 0x04) != 0)
#define MODE_INTERLACE_AND_VIDEO    ((m_mode_control & 0x03) == 3)


mc6845_device::mc6845_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_video_interface(mconfig, *this, false)
	, m_line_timer(nullptr)
	, m_show_border_area(true)
	, m_noninterlace_adjust(0)
	, m_interlace_adjust(0)
	, m_clk_scale(1)
	, m_visarea_adjust_min_x(0)
	, m_visarea_adjust_max_x(0)
	, m_visarea_adjust_min_y(0)
	, m_visarea_adjust_max_y(0)
	, m_hpixels_per_column(0)
	, m_reconfigure_cb(*this)
	, m_begin_update_cb(*this)
	, m_update_row_cb(*this)
	, m_end_update_cb(*this)
	, m_on_update_addr_changed_cb(*this)
	, m_out_de_cb(*this)
	, m_out_cur_cb(*this)
	, m_out_hsync_cb(*this)
	, m_out_vsync_cb(*this)
{
}

mc6845_device::mc6845_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mc6845_device(mconfig, MC6845, tag, owner, clock)
{
}


void mc6845_device::device_post_load()
{
	recompute_parameters(true);
}


void mc6845_device::device_clock_changed()
{
	recompute_parameters(true);
}


void mc6845_device::call_on_update_address(int strobe)
{
	if (!m_on_update_addr_changed_cb.isnull())
		m_upd_trans_timer->adjust(attotime::zero, (m_update_addr << 8) | strobe);
	else
		fatalerror("M6845: transparent memory mode without handler\n");
}


void mc6845_device::address_w(uint8_t data)
{
	m_register_address_latch = data & 0x1f;
}


uint8_t mc6845_device::status_r()
{
	uint8_t ret = 0;

	/* VBLANK bit */
	if (m_supports_status_reg_d5 && !m_line_enable_ff)
		ret = ret | 0x20;

	/* light pen latched */
	if (m_supports_status_reg_d6 && m_light_pen_latched)
		ret = ret | 0x40;

	/* UPDATE ready */
	if (m_supports_status_reg_d7 && m_update_ready_bit)
		ret = ret | 0x80;

	return ret;
}


void mc6845_device::transparent_update()
{
	if (m_supports_transparent && MODE_TRANSPARENT)
	{
		if (MODE_TRANSPARENT_PHI2)
		{
			m_update_addr++;
			m_update_addr &= 0x3fff;
			call_on_update_address(MODE_UPDATE_STROBE);
		}
		else
		{
			/* MODE_TRANSPARENT_BLANK */
			if (m_update_ready_bit)
			{
				m_update_ready_bit = false;
				update_upd_adr_timer();
			}
		}
	}
}


uint8_t mc6845_device::register_r()
{
	uint8_t ret = 0;

	switch (m_register_address_latch)
	{
		case 0x0c:  ret = m_supports_disp_start_addr_r ? (m_disp_start_addr >> 8) & 0xff : 0; break;
		case 0x0d:  ret = m_supports_disp_start_addr_r ? (m_disp_start_addr >> 0) & 0xff : 0; break;
		case 0x0e:  ret = (m_cursor_addr    >> 8) & 0xff; break;
		case 0x0f:  ret = (m_cursor_addr    >> 0) & 0xff; break;
		// FIXME: status flag should not be reset if LPEN input is held high
		case 0x10:  ret = (m_light_pen_addr >> 8) & 0xff; m_light_pen_latched = false; break;
		case 0x11:  ret = (m_light_pen_addr >> 0) & 0xff; m_light_pen_latched = false; break;
		case 0x1f:  transparent_update(); break;

		/* all other registers are write only and return 0 */
		default: break;
	}

	return ret;
}


void mc6845_device::register_w(uint8_t data)
{
	LOGREGS("%s:M6845 reg 0x%02x = 0x%02x\n", machine().describe_context(), m_register_address_latch, data);

	/* Omits LOGSETUP logs of cursor registers as they tend to be spammy */
	if (m_register_address_latch < 0x0e &&
		m_register_address_latch != 0x0a &&
		m_register_address_latch != 0x0b) LOGSETUP(" * %02x <= %3u [%02x] %s\n", m_register_address_latch,
							  data, data, std::array<char const *, 16>
		 {{ "R0 - Horizontal Total",       "R1 - Horizontal Displayed",   "R2 - Horizontal Sync Position",
			"R3 - Sync Width",             "R4 - Vertical Total",         "R5 - Vertical Total Adjust",
			"R6 - Vertical Displayed",     "R7 - Vertical Sync Position", "R8 - Interlace & Skew",
			"R9 - Maximum Raster Address", "R10 - Cursor Start Raster",   "R11 - Cursor End Raster",
			"R12 - Start Address (H)",     "R13 - Start Address (L)",     "R14 - Cursor (H)",
			"R15 - Cursor (L)" }}[(m_register_address_latch & 0x0f)]);

	switch (m_register_address_latch)
	{
		case 0x00:  m_horiz_char_total =   data & 0xff; break;
		case 0x01:  m_horiz_disp       =   data & 0xff; break;
		case 0x02:  m_horiz_sync_pos   =   data & 0xff; break;
		case 0x03:  m_sync_width       =   data & 0xff; break;
		case 0x04:  m_vert_char_total  =   data & 0x7f; break;
		case 0x05:  m_vert_total_adj   =   data & 0x1f; break;
		case 0x06:  m_vert_disp        =   data & 0x7f; break;
		case 0x07:  m_vert_sync_pos    =   data & 0x7f; break;
		case 0x08:  m_mode_control     =   data & 0xff; break;
		case 0x09:  m_max_ras_addr     =   data & 0x1f; break;
		case 0x0a:  m_cursor_start_ras =   data & 0x7f; break;
		case 0x0b:  m_cursor_end_ras   =   data & 0x1f; break;
		case 0x0c:  m_disp_start_addr  = ((data & 0x3f) << 8) | (m_disp_start_addr & 0x00ff); break;
		case 0x0d:  m_disp_start_addr  = ((data & 0xff) << 0) | (m_disp_start_addr & 0xff00); break;
		case 0x0e:  m_cursor_addr      = ((data & 0x3f) << 8) | (m_cursor_addr & 0x00ff); break;
		case 0x0f:  m_cursor_addr      = ((data & 0xff) << 0) | (m_cursor_addr & 0xff00); break;
		case 0x10: /* read-only */ break;
		case 0x11: /* read-only */ break;
		case 0x12:
			if (m_supports_transparent)
			{
				m_update_addr = ((data & 0x3f) << 8) | (m_update_addr & 0x00ff);
				if(MODE_TRANSPARENT_PHI2)
					call_on_update_address(MODE_UPDATE_STROBE);
			}
			break;
		case 0x13:
			if (m_supports_transparent)
			{
				m_update_addr = ((data & 0xff) << 0) | (m_update_addr & 0xff00);
				if(MODE_TRANSPARENT_PHI2)
					call_on_update_address(MODE_UPDATE_STROBE);
			}
			break;
		case 0x1f:  transparent_update(); break;
		default: break;
	}

	/* display message if the Mode Control register is not zero */
	if ((m_register_address_latch == 0x08) && (m_mode_control != 0))
		if (!m_supports_transparent)
			logerror("M6845: Mode Control %02X is not supported!!!\n", m_mode_control);

	recompute_parameters(false);
}


void hd6345_device::address_w(uint8_t data)
{
	m_register_address_latch = data & 0x3f;
}


uint8_t hd6345_device::register_r()
{
	uint8_t ret = 0;

	switch (m_register_address_latch)
	{
		case 0x0c:  ret = (m_disp_start_addr  >> 8) & 0xff; break;
		case 0x0d:  ret = (m_disp_start_addr  >> 0) & 0xff; break;
		case 0x0e:  ret = (m_cursor_addr      >> 8) & 0xff; break;
		case 0x0f:  ret = (m_cursor_addr      >> 0) & 0xff; break;
		case 0x10:  ret = (m_light_pen_addr   >> 8) & 0xff; m_light_pen_latched = false; break;
		case 0x11:  ret = (m_light_pen_addr   >> 0) & 0xff; m_light_pen_latched = false; break;
		case 0x12:  ret = m_disp2_pos; break;
		case 0x13:  ret = (m_disp2_start_addr >> 8) & 0xff; break;
		case 0x14:  ret = (m_disp2_start_addr >> 0) & 0xff; break;
		case 0x15:  ret = m_disp3_pos; break;
		case 0x16:  ret = (m_disp3_start_addr >> 8) & 0xff; break;
		case 0x17:  ret = (m_disp3_start_addr >> 0) & 0xff; break;
		case 0x18:  ret = m_disp4_pos; break;
		case 0x19:  ret = (m_disp4_start_addr >> 8) & 0xff; break;
		case 0x1a:  ret = (m_disp4_start_addr >> 0) & 0xff; break;
		case 0x1b:  ret = m_vert_sync_pos_adj; break;
		case 0x1c: /* TODO: light pen raster */ break;
		case 0x1d:  ret = m_smooth_scroll_ras; break;
		case 0x1f: /* TODO: status */ break;
		case 0x21:  ret = m_mem_width_offs; break;
		case 0x24:  ret = (m_cursor2_addr     >> 8) & 0xff; break;
		case 0x25:  ret = (m_cursor2_addr     >> 0) & 0xff; break;
		case 0x26:  ret = m_cursor_width; break;
		case 0x27:  ret = m_cursor2_width; break;
	}

	return ret;
}

void hd6345_device::register_w(uint8_t data)
{
	LOGREGS("%s:HD6345 reg 0x%02x = 0x%02x\n", machine().describe_context(), m_register_address_latch, data);

	/* Omits LOGSETUP logs of cursor registers as they tend to be spammy */
	if (m_register_address_latch < 0x28 &&
		m_register_address_latch != 0x0a && m_register_address_latch != 0x0a &&
		m_register_address_latch != 0x0e && m_register_address_latch != 0x0f)
		LOGSETUP(" * %02x <= %3u [%02x] %s\n", m_register_address_latch, data, data, std::array<char const *, 40>
		 {{ "R0 - Horizontal Total",            "R1 - Horizontal Displayed",        "R2 - Horizontal Sync Position",
			"R3 - Sync Width",                  "R4 - Vertical Total",              "R5 - Vertical Total Adjust",
			"R6 - Vertical Displayed",          "R7 - Vertical Sync Position",      "R8 - Interlace Mode & Skew",
			"R9 - Maximum Raster Address",      "R10 - Cursor 1 Start",             "R11 - Cursor 1 End",
			"R12 - Screen 1 Start Address (H)", "R13 - Screen 1 Start Address (L)", "R14 - Cursor 1 Address (H)",
			"R15 - Cursor 1 Address (L)",       "R16 - Light Pen (H)",              "R17 - Light Pen (L)",
			"R18 - Screen 2 Start Position",    "R19 - Screen 2 Start Address (H)", "R20 - Screen 2 Start Address (L)",
			"R21 - Screen 3 Start Position",    "R22 - Screen 3 Start Address (H)", "R23 - Screen 3 Start Address (L)",
			"R24 - Screen 4 Start Position",    "R25 - Screen 4 Start Address (H)", "R26 - Screen 4 Start Address (L)",
			"R27 - Vertical Sync Position Adj", "R28 - Light Pen Raster",           "R29 - Smooth Scrolling",
			"R30 - Control 1",                  "R31 - Control 2",                  "R32 - Control 3",
			"R33 - Memory Width Offset",        "R34 - Cursor 2 Start",             "R35 - Cursor 2 End",
			"R36 - Cursor 2 Address (H)",       "R37 - Cursor 2 Address (L)",       "R38 - Cursor 1 Width",
			"R39 - Cursor 2 Width" }}[(m_register_address_latch & 0x3f)]);

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
		case 0x08:  m_mode_control     =   data & 0xf3; break;
		case 0x09:  m_max_ras_addr     =   data & 0x1f; break;
		case 0x0a:  m_cursor_start_ras =   data & 0x7f; break;
		case 0x0b:  m_cursor_end_ras   =   data & 0x1f; break;
		case 0x0c:  m_disp_start_addr  = ((data & 0x3f) << 8) | (m_disp_start_addr & 0x00ff); break;
		case 0x0d:  m_disp_start_addr  = ((data & 0xff) << 0) | (m_disp_start_addr & 0xff00); break;
		case 0x0e:  m_cursor_addr      = ((data & 0x3f) << 8) | (m_cursor_addr & 0x00ff); break;
		case 0x0f:  m_cursor_addr      = ((data & 0xff) << 0) | (m_cursor_addr & 0xff00); break;
		case 0x10: /* read-only */ break;
		case 0x11: /* read-only */ break;
		case 0x12:  m_disp2_pos         =   data & 0xff; break;
		case 0x13:  m_disp2_start_addr  = ((data & 0x3f) << 8) | (m_disp2_start_addr & 0x00ff); break;
		case 0x14:  m_disp2_start_addr  = ((data & 0xff) << 0) | (m_disp2_start_addr & 0xff00); break;
		case 0x15:  m_disp3_pos         =   data & 0xff; break;
		case 0x16:  m_disp3_start_addr  = ((data & 0x3f) << 8) | (m_disp3_start_addr & 0x00ff); break;
		case 0x17:  m_disp3_start_addr  = ((data & 0xff) << 0) | (m_disp3_start_addr & 0xff00); break;
		case 0x18:  m_disp4_pos         =   data & 0xff; break;
		case 0x19:  m_disp4_start_addr  = ((data & 0x3f) << 8) | (m_disp4_start_addr & 0x00ff); break;
		case 0x1a:  m_disp4_start_addr  = ((data & 0xff) << 0) | (m_disp4_start_addr & 0xff00); break;
		case 0x1b:  m_vert_sync_pos_adj =   data & 0x1f; break;
		case 0x1c: /* read-only */ break;
		case 0x1d:  m_smooth_scroll_ras =   data & 0x1f; break;
		case 0x1e:  m_control1          =   data & 0xff; break;
		case 0x1f:  m_control2          =   data & 0xf8; break;
		case 0x20:  m_control3          =   data & 0xfe; break;
		case 0x21:  m_mem_width_offs    =   data & 0xff; break;
		case 0x22:  m_cursor2_start_ras =   data & 0x7f; break;
		case 0x23:  m_cursor2_end_ras   =   data & 0x1f; break;
		case 0x24:  m_cursor2_addr      = ((data & 0x3f) << 8) | (m_cursor2_addr & 0x00ff); break;
		case 0x25:  m_cursor2_addr      = ((data & 0xff) << 0) | (m_cursor2_addr & 0xff00); break;
		case 0x26:  m_cursor_width      =   data & 0xff; break;
		case 0x27:  m_cursor2_width     =   data & 0xff; break;
	}

	recompute_parameters(false);
}


int mc6845_device::de_r()
{
	return m_de;
}


int mc6845_device::cursor_r()
{
	return m_cur;
}


int mc6845_device::hsync_r()
{
	return m_hsync;
}


int mc6845_device::vsync_r()
{
	return m_vsync;
}


void mc6845_device::recompute_parameters(bool postload)
{
	uint16_t hsync_on_pos, hsync_off_pos, vsync_on_pos, vsync_off_pos;

	uint16_t video_char_height = m_max_ras_addr + (MODE_INTERLACE_AND_VIDEO ? m_interlace_adjust : m_noninterlace_adjust);   // fix garbage at the bottom of the screen (eg victor9k)
	// Would be useful for 'interlace and video' mode support...
	// uint16_t frame_char_height = (MODE_INTERLACE_AND_VIDEO ? m_max_ras_addr / 2 : m_max_ras_addr) + 1;

	/* compute the screen sizes */
	uint16_t horiz_pix_total = (m_horiz_char_total + 1) * m_hpixels_per_column;
	uint16_t vert_pix_total = (m_vert_char_total + 1) * video_char_height + m_vert_total_adj;

	/* determine the visible area, avoid division by 0 */
	uint16_t max_visible_x = m_horiz_disp * m_hpixels_per_column - 1;
	uint16_t max_visible_y = m_vert_disp * video_char_height - 1;

	/* determine the syncing positions */
	uint8_t horiz_sync_char_width = m_sync_width & 0x0f;
	uint8_t vert_sync_pix_width = m_supports_vert_sync_width ? (m_sync_width >> 4) & 0x0f : 0x10;

	if (horiz_sync_char_width == 0)
		horiz_sync_char_width = 0x10;

	if (vert_sync_pix_width == 0)
		vert_sync_pix_width = 0x10;

	/* determine the transparent update cycle time, 1 update every 4 character clocks */
	m_upd_time = cclks_to_attotime(4 * m_hpixels_per_column);

	hsync_on_pos = m_horiz_sync_pos * m_hpixels_per_column;
	hsync_off_pos = hsync_on_pos + (horiz_sync_char_width * m_hpixels_per_column);
	vsync_on_pos = m_vert_sync_pos * video_char_height;
	vsync_off_pos = vsync_on_pos + vert_sync_pix_width;

	// the Commodore PET computers have a non-standard 20kHz monitor which
	// requires a wider HSYNC pulse that extends past the scanline width
	if (hsync_off_pos > horiz_pix_total)
		hsync_off_pos = horiz_pix_total;

	if (vsync_on_pos > vert_pix_total)
		vsync_on_pos = vert_pix_total;

	if (vsync_off_pos > vert_pix_total)
		vsync_off_pos = vert_pix_total;

	/* update only if screen parameters changed, unless we are coming here after loading the saved state */
	if (postload ||
		(horiz_pix_total != m_horiz_pix_total) || (vert_pix_total != m_vert_pix_total) ||
		(max_visible_x != m_max_visible_x) || (max_visible_y != m_max_visible_y) ||
		(hsync_on_pos != m_hsync_on_pos) || (vsync_on_pos != m_vsync_on_pos) ||
		(hsync_off_pos != m_hsync_off_pos) || (vsync_off_pos != m_vsync_off_pos))
	{
		/* update the screen if we have valid data */
		if ((horiz_pix_total > 0) && (max_visible_x < horiz_pix_total) &&
			(vert_pix_total > 0) && (max_visible_y < vert_pix_total) &&
			(hsync_on_pos <= horiz_pix_total) && (vsync_on_pos <= vert_pix_total) &&
			(hsync_on_pos != hsync_off_pos))
		{
			rectangle visarea;

			attotime refresh = cclks_to_attotime((m_horiz_char_total + 1) * vert_pix_total);

			// This doubles the vertical resolution, required for 'interlace and video' mode support.
			// Tested and works for super80v, which was designed with this in mind (choose green or monochrome colour in config switches).
			// However it breaks some other drivers (apricot,a6809,victor9k,bbc(mode7)).
			// So, it is commented out for now.
			// Also, the mode-register change needs to be added to the changed-parameter tests above.
			if (MODE_INTERLACE_AND_VIDEO)
			{
				//max_visible_y *= 2;
				//vert_pix_total *= 2;
			}

			if(m_show_border_area)
				visarea.set(0, horiz_pix_total-2, 0, vert_pix_total-2);
			else
				visarea.set(0 + m_visarea_adjust_min_x, max_visible_x + m_visarea_adjust_max_x, 0 + m_visarea_adjust_min_y, max_visible_y + m_visarea_adjust_max_y);

			LOGCONF("M6845 config screen: HTOTAL: %d  VTOTAL: %d  MAX_X: %d  MAX_Y: %d  HSYNC: %d-%d  VSYNC: %d-%d  Freq: %ffps\n",
				 horiz_pix_total, vert_pix_total, max_visible_x, max_visible_y, hsync_on_pos, hsync_off_pos - 1, vsync_on_pos, vsync_off_pos - 1, refresh.as_hz());

			if (has_screen())
				screen().configure(horiz_pix_total, vert_pix_total, visarea, refresh.as_attoseconds());

			if(!m_reconfigure_cb.isnull())
				m_reconfigure_cb(horiz_pix_total, vert_pix_total, visarea, refresh.as_attoseconds());

			m_has_valid_parameters = true;
		}
		else
			m_has_valid_parameters = false;

		m_horiz_pix_total = horiz_pix_total;
		m_vert_pix_total = vert_pix_total;
		m_max_visible_x = max_visible_x;
		m_max_visible_y = max_visible_y;
		m_hsync_on_pos = hsync_on_pos;
		m_hsync_off_pos = hsync_off_pos;
		m_vsync_on_pos = vsync_on_pos;
		m_vsync_off_pos = vsync_off_pos;
		if (m_line_timer && !m_line_timer->enabled() && m_has_valid_parameters)
		{
			m_line_timer->adjust(cclks_to_attotime(m_horiz_char_total + 1));
		}
		if ( (!m_reconfigure_cb.isnull()) && (!postload) )
			m_line_counter = 0;
	}
}


void mc6845_device::update_counters()
{
	m_character_counter = attotime_to_cclks(m_line_timer->elapsed());

	if (m_hsync_off_timer->enabled())
	{
		m_hsync_width_counter = attotime_to_cclks(m_hsync_off_timer->elapsed());
	}
}


void mc6845_device::set_de(int state)
{
	if (m_de != state)
	{
		m_de = state;

		if (m_de)
		{
			/* If the upd_adr_timer was running, cancel it */
			m_upd_adr_timer->adjust(attotime::never);
		}
		else
		{
			/* if transparent update was requested fire the update timer */
			if(!m_update_ready_bit)
				update_upd_adr_timer();
		}

		m_out_de_cb(m_de);
	}
}


void mc6845_device::set_hsync(int state)
{
	if (m_hsync != state)
	{
		m_hsync = state;
		m_out_hsync_cb(m_hsync);
	}
}


void mc6845_device::set_vsync(int state)
{
	if (m_vsync != state)
	{
		m_vsync = state;
		m_out_vsync_cb(m_vsync);
	}
}


void mc6845_device::set_cur(int state)
{
	if (m_cur != state)
	{
		m_cur = state;
		m_out_cur_cb(m_cur);
	}
}


void mc6845_device::update_upd_adr_timer()
{
	if (! m_de && m_supports_transparent)
		m_upd_adr_timer->adjust(m_upd_time);
}


bool mc6845_device::match_line()
{
	/* Check if we've reached the end of active display */
	if ( m_line_counter == m_vert_disp )
	{
		m_line_enable_ff = false;
		m_current_disp_addr = m_disp_start_addr;
	}

	/* Check if VSYNC should be enabled */
	if ( m_line_counter == m_vert_sync_pos )
	{
		m_vsync_width_counter = 0;
		m_vsync_ff = 1;

		return true;
	}

	return false;
}


bool mc6845_device::check_cursor_visible(uint16_t ra, uint16_t line_addr)
{
	if (!m_cursor_state)
		return false;

	if ((m_cursor_addr < line_addr) ||
		(m_cursor_addr >= (line_addr + m_horiz_disp)))
	{
		// Not a cursor character line.
		return false;
	}

	uint16_t cursor_start_ras = m_cursor_start_ras & 0x1f;
	uint16_t max_ras_addr = m_max_ras_addr + (MODE_INTERLACE_AND_VIDEO ? m_interlace_adjust : m_noninterlace_adjust) - 1;

	if (cursor_start_ras > max_ras_addr)
	{
		// No cursor.
		return false;
	}

	// TODO explore the edge cases in the 'interlace and video' mode.

	if (cursor_start_ras <= m_cursor_end_ras)
	{
		if (m_cursor_end_ras > max_ras_addr)
		{
			// Wraps to produce a full cursor.
			return true;
		}
		// Cursor from start to end inclusive.
		return (ra >= cursor_start_ras) && (ra <= m_cursor_end_ras);
	}

	// Otherwise cursor_start_ras > m_cursor_end_ras giving a split cursor.
	return (ra <= m_cursor_end_ras) || (ra >= cursor_start_ras);
}

// The HD6845 cursor does not wrap as it does for the MC6845.
bool hd6845s_device::check_cursor_visible(uint16_t ra, uint16_t line_addr)
{
	if (!m_cursor_state)
		return false;

	if ((m_cursor_addr < line_addr) ||
		(m_cursor_addr >= (line_addr + m_horiz_disp)))
	{
		// Not a cursor character line.
		return false;
	}

	uint16_t cursor_start_ras = m_cursor_start_ras & 0x1f;
	uint16_t max_ras_addr = m_max_ras_addr + (MODE_INTERLACE_AND_VIDEO ? m_interlace_adjust : m_noninterlace_adjust) - 1;

	if (cursor_start_ras > max_ras_addr || cursor_start_ras > m_cursor_end_ras)
	{
		// No cursor.
		return false;
	}

	// Cursor from start to end inclusive.
	return (ra >= cursor_start_ras) && (ra <= m_cursor_end_ras);
}


TIMER_CALLBACK_MEMBER(mc6845_device::handle_line_timer)
{
	bool new_vsync = m_vsync;

	m_character_counter = 0;
	m_cursor_x = -1;

	/* Check if VSYNC is active */
	if ( m_vsync_ff )
	{
		uint8_t vsync_width = m_supports_vert_sync_width ? (m_sync_width >> 4) & 0x0f : 0;

		m_vsync_width_counter = ( m_vsync_width_counter + 1 ) & 0x0F;

		/* Check if we've reached end of VSYNC */
		if ( m_vsync_width_counter == vsync_width )
		{
			m_vsync_ff = 0;

			new_vsync = false;
		}
	}

	// For rudimentary 'interlace and video' support, m_raster_counter increments by 1 rather than the correct 2.
	// The correct test would be:
	// if ( m_raster_counter == (MODE_INTERLACE_AND_VIDEO ? m_max_ras_addr + 1 : m_max_ras_addr) )
	if ( m_raster_counter == m_max_ras_addr + (MODE_INTERLACE_AND_VIDEO ? m_interlace_adjust : m_noninterlace_adjust) - 1 )
	{
		/* Check if we have reached the end of the vertical area */
		if ( m_line_counter == m_vert_char_total )
		{
			m_adjust_counter = 0;
			m_adjust_active = 1;
		}

		m_raster_counter = 0;
		m_line_counter = ( m_line_counter + 1 ) & 0x7F;
		m_line_address = ( m_line_address + m_horiz_disp ) & 0x3fff;

		if (match_line())
			new_vsync = true;
	}
	else
	{
		// For rudimentary 'interlace and video' support, m_raster_counter increments by 1 rather than the correct 2.
		// m_raster_counter = ( m_raster_counter + (MODE_INTERLACE_AND_VIDEO ? 2 : 1) ) & 0x1F;
		m_raster_counter = ( m_raster_counter + 1 ) & 0x1F;
	}

	if ( m_adjust_active )
	{
		/* Check if we have reached the end of a full cycle */
		if ( m_adjust_counter == m_vert_total_adj )
		{
			m_adjust_active = 0;
			m_raster_counter = 0;
			m_line_counter = 0;
			m_line_address = m_disp_start_addr;
			m_line_enable_ff = true;

			if (m_supports_vert_sync_width)
			{
				if (match_line())
					new_vsync = true;
			}

			/* also update the cursor state now */
			update_cursor_state();

			if (has_screen())
				screen().reset_origin();
		}
		else
		{
			m_adjust_counter = ( m_adjust_counter + 1 ) & 0x1F;
		}
	}

	if ( m_line_enable_ff )
	{
		/* Schedule DE off signal change */
		m_de_off_timer->adjust(cclks_to_attotime(m_horiz_disp));

		/* Is cursor visible on this line? */
		if (check_cursor_visible(m_raster_counter, m_line_address))
		{
			m_cursor_x = m_cursor_addr - m_line_address;

			/* Schedule CURSOR ON signal */
			m_cursor_on_timer->adjust(cclks_to_attotime(m_cursor_x));
		}
	}

	/* Schedule HSYNC on signal */
	m_hsync_on_timer->adjust(cclks_to_attotime(m_horiz_sync_pos));

	/* Schedule our next callback */
	m_line_timer->adjust(cclks_to_attotime(m_horiz_char_total + 1));

	/* Set VSYNC and DE signals */
	set_vsync( new_vsync );
	set_de( m_line_enable_ff ? true : false );
}


TIMER_CALLBACK_MEMBER(mc6845_device::de_off_tick)
{
	set_de( false );
}

TIMER_CALLBACK_MEMBER(mc6845_device::cursor_on)
{
	set_cur(true);

	/* Schedule CURSOR off signal */
	m_cursor_off_timer->adjust(cclks_to_attotime(1));
}

TIMER_CALLBACK_MEMBER(mc6845_device::cursor_off)
{
	set_cur(false);
}

TIMER_CALLBACK_MEMBER(mc6845_device::hsync_on)
{
	uint8_t hsync_width = ( m_sync_width & 0x0f ) ? ( m_sync_width & 0x0f ) : 0x10;

	m_hsync_width_counter = 0;
	set_hsync( true );

	/* Schedule HSYNC off signal */
	m_hsync_off_timer->adjust(cclks_to_attotime(hsync_width));
}

TIMER_CALLBACK_MEMBER(mc6845_device::hsync_off)
{
	set_hsync( false );
}

TIMER_CALLBACK_MEMBER(mc6845_device::latch_light_pen)
{
	m_light_pen_addr = get_ma();
	m_light_pen_latched = true;
}

TIMER_CALLBACK_MEMBER(mc6845_device::adr_update_tick)
{
	/* fire a update address strobe */
	call_on_update_address(MODE_UPDATE_STROBE);
}

TIMER_CALLBACK_MEMBER(mc6845_device::transparent_update_tick)
{
	int addr = (param >> 8);
	int strobe = (param & 0xff);

	/* call the callback function -- we know it exists */
	m_on_update_addr_changed_cb(addr, strobe);

	if(!m_update_ready_bit && MODE_TRANSPARENT_BLANK)
	{
		m_update_addr++;
		m_update_addr &= 0x3fff;
		m_update_ready_bit = true;
	}
}


uint16_t mc6845_device::get_ma()
{
	update_counters();

	return ( m_line_address + m_character_counter ) & 0x3fff;
}


uint8_t mc6845_device::get_ra()
{
	return m_raster_counter;
}


void mc6845_device::assert_light_pen_input()
{
	/* compute the pixel coordinate of the NEXT character -- this is when the light pen latches */
	/* set the timer that will latch the display address into the light pen registers */
	m_light_pen_latch_timer->adjust(cclks_to_attotime(1));
}


// Microbee requires precise supplied light-pen address
void mc6845_device::assert_light_pen_input(u16 ma)
{
	m_light_pen_addr = ma;
	m_light_pen_latched = true;
}


void mc6845_device::set_hpixels_per_column(int hpixels_per_column)
{
	/* validate arguments */
	assert(hpixels_per_column > 0);

	if (hpixels_per_column != m_hpixels_per_column)
	{
		m_hpixels_per_column = hpixels_per_column;
		recompute_parameters(false);
	}
}


void mc6845_device::update_cursor_state()
{
	/* save and increment cursor counter */
	uint8_t last_cursor_blink_count = m_cursor_blink_count;
	m_cursor_blink_count = m_cursor_blink_count + 1;

	/* switch on cursor blinking mode */
	switch (m_cursor_start_ras & 0x60)
	{
		/* always on */
		case 0x00: m_cursor_state = true; break;

		/* always off */
		default:
		case 0x20: m_cursor_state = false; break;

		/* fast blink */
		case 0x40:
			if ((last_cursor_blink_count & 0x10) != (m_cursor_blink_count & 0x10))
				m_cursor_state = !m_cursor_state;
			break;

		/* slow blink */
		case 0x60:
			if ((last_cursor_blink_count & 0x20) != (m_cursor_blink_count & 0x20))
				m_cursor_state = !m_cursor_state;
			break;
	}
}


uint8_t mc6845_device::draw_scanline(int y, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	/* compute the current raster line */
	uint8_t ra = y % (m_max_ras_addr + (MODE_INTERLACE_AND_VIDEO ? m_interlace_adjust : m_noninterlace_adjust));

	// Check if the cursor is visible and is on this scanline.
	int cursor_visible = check_cursor_visible(ra, m_current_disp_addr);

	// Compute the cursor X position, or -1 if not visible. This position
	// is in units of characters and is relative to the start of the
	// displayable area, not relative to the screen bitmap origin.
	int8_t cursor_x = cursor_visible ? (m_cursor_addr - m_current_disp_addr) : -1;
	int de = (y <= m_max_visible_y) ? 1 : 0;
	int vbp = m_vert_pix_total - m_vsync_off_pos;
	if (vbp < 0) vbp = 0;
	int hbp = m_horiz_pix_total - m_hsync_off_pos;
	if (hbp < 0) hbp = 0;

	/* call the external system to draw it */
	if (MODE_ROW_COLUMN_ADDRESSING)
	{
		uint8_t cc = 0;
		uint8_t cr = y / (m_max_ras_addr + (MODE_INTERLACE_AND_VIDEO ? m_interlace_adjust : m_noninterlace_adjust));
		uint16_t ma = (cr << 8) | cc;

		m_update_row_cb(bitmap, cliprect, ma + m_disp_start_addr, ra, y, m_horiz_disp, cursor_x, de, hbp, vbp);
	}
	else
	{
		m_update_row_cb(bitmap, cliprect, m_current_disp_addr, ra, y, m_horiz_disp, cursor_x, de, hbp, vbp);
	}

	/* update MA if the last raster address */
	if (ra == m_max_ras_addr + (MODE_INTERLACE_AND_VIDEO ? m_interlace_adjust : m_noninterlace_adjust) - 1)
		m_current_disp_addr = (m_current_disp_addr + m_horiz_disp) & 0x3fff;

	return ra;
}


uint8_t hd6345_device::draw_scanline(int y, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint8_t ra = hd6845s_device::draw_scanline(y, bitmap, cliprect);

	/* update MA for screen split */
	if (ra == m_max_ras_addr + (MODE_INTERLACE_AND_VIDEO ? m_interlace_adjust : m_noninterlace_adjust) - 1)
	{
		int y_pos = y / (m_max_ras_addr + (MODE_INTERLACE_AND_VIDEO ? m_interlace_adjust : m_noninterlace_adjust));
		if ((m_control1 & 0x03) > 0 && y_pos == m_disp2_pos && m_disp2_pos != m_disp3_pos && m_disp2_pos != m_disp4_pos)
			m_current_disp_addr = m_disp2_start_addr;
		if ((m_control1 & 0x03) > 1 && y_pos == m_disp3_pos && m_disp3_pos != m_disp2_pos && m_disp3_pos != m_disp4_pos)
			m_current_disp_addr = m_disp3_start_addr;
		if ((m_control1 & 0x03) > 2 && y_pos == m_disp4_pos && m_disp4_pos != m_disp2_pos && m_disp4_pos != m_disp3_pos)
			m_current_disp_addr = m_disp4_start_addr;
	}

	return ra;
}


uint32_t mc6845_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	assert(bitmap.valid());

	if (m_has_valid_parameters)
	{
		if (m_display_disabled_msg_shown == true)
		{
			logerror("M6845: Valid screen parameters - display reenabled!!!\n");
			m_display_disabled_msg_shown = false;
		}

		/* call the set up function if any */
		m_begin_update_cb(bitmap, cliprect);

		if (cliprect.min_y == 0)
		{
			/* read the start address at the beginning of the frame */
			m_current_disp_addr = m_disp_start_addr;
		}

		/* for each row in the visible region */
		for (uint16_t y = cliprect.min_y; y <= cliprect.max_y; y++)
		{
			this->draw_scanline(y, bitmap, cliprect);
		}

		/* call the tear down function if any */
		m_end_update_cb(bitmap, cliprect);
	}
	else
	{
		if (m_display_disabled_msg_shown == false)
		{
			logerror("M6845: Invalid screen parameters - display disabled!!!\n");
			m_display_disabled_msg_shown = true;
		}
	}

	return 0;
}


void mc6845_device::device_start()
{
	assert(clock() > 0);
	assert(m_hpixels_per_column > 0);

	/* bind delegates */
	m_reconfigure_cb.resolve();
	m_begin_update_cb.resolve_safe();
	m_update_row_cb.resolve_safe();
	m_end_update_cb.resolve_safe();
	m_on_update_addr_changed_cb.resolve();

	/* create the timers */
	m_line_timer = timer_alloc(FUNC(mc6845_device::handle_line_timer), this);
	m_de_off_timer = timer_alloc(FUNC(mc6845_device::de_off_tick), this);
	m_cursor_on_timer = timer_alloc(FUNC(mc6845_device::cursor_on), this);
	m_cursor_off_timer = timer_alloc(FUNC(mc6845_device::cursor_off), this);
	m_hsync_on_timer = timer_alloc(FUNC(mc6845_device::hsync_on), this);
	m_hsync_off_timer = timer_alloc(FUNC(mc6845_device::hsync_off), this);
	m_light_pen_latch_timer = timer_alloc(FUNC(mc6845_device::latch_light_pen), this);
	m_upd_adr_timer = timer_alloc(FUNC(mc6845_device::adr_update_tick), this);
	m_upd_trans_timer = timer_alloc(FUNC(mc6845_device::transparent_update_tick), this);

	/* Use some large startup values */
	m_horiz_char_total = 0xff;
	m_max_ras_addr = 0x1f;
	m_vert_char_total = 0x7f;
	m_mode_control = 0x00;

	m_supports_disp_start_addr_r = false;  // MC6845 can not read Display Start (double checked on datasheet)
	m_supports_vert_sync_width = false;
	m_supports_status_reg_d5 = false;
	m_supports_status_reg_d6 = false;
	m_supports_status_reg_d7 = false;
	m_supports_transparent = false;
	m_has_valid_parameters = false;
	m_display_disabled_msg_shown = false;
	m_line_enable_ff = false;
	m_vsync_ff = 0;
	m_raster_counter = 0;
	m_adjust_active = 0;
	m_horiz_sync_pos = 1;
	m_vert_sync_pos = 1;
	m_de = 0;
	m_sync_width = 1;
	m_horiz_pix_total = m_vert_pix_total = 0;
	m_max_visible_x = m_max_visible_y = 0;
	m_hsync_on_pos = m_vsync_on_pos = 0;
	m_hsync_off_pos = m_vsync_off_pos = 0;
	m_vsync = m_hsync = 0;
	m_cur = 0;
	m_line_counter = 0;
	m_horiz_disp = m_vert_disp = 0;
	m_vert_sync_pos = 0;
	m_vert_total_adj = 0;
	m_cursor_start_ras = m_cursor_end_ras = m_cursor_addr = 0;
	m_cursor_blink_count = 0;
	m_cursor_state = 0;
	m_update_ready_bit = 0;
	m_line_address = 0;
	m_current_disp_addr = 0;
	m_disp_start_addr = 0;
	m_noninterlace_adjust = 1;
	m_interlace_adjust = 1;

	save_item(NAME(m_show_border_area));
	save_item(NAME(m_visarea_adjust_min_x));
	save_item(NAME(m_visarea_adjust_max_x));
	save_item(NAME(m_visarea_adjust_min_y));
	save_item(NAME(m_visarea_adjust_max_y));
	save_item(NAME(m_hpixels_per_column));
	save_item(NAME(m_register_address_latch));
	save_item(NAME(m_horiz_char_total));
	save_item(NAME(m_horiz_disp));
	save_item(NAME(m_horiz_sync_pos));
	save_item(NAME(m_sync_width));
	save_item(NAME(m_vert_char_total));
	save_item(NAME(m_vert_total_adj));
	save_item(NAME(m_vert_disp));
	save_item(NAME(m_vert_sync_pos));
	save_item(NAME(m_mode_control));
	save_item(NAME(m_max_ras_addr));
	save_item(NAME(m_cursor_start_ras));
	save_item(NAME(m_cursor_end_ras));
	save_item(NAME(m_disp_start_addr));
	save_item(NAME(m_cursor_addr));
	save_item(NAME(m_light_pen_addr));
	save_item(NAME(m_light_pen_latched));
	save_item(NAME(m_cursor_state));
	save_item(NAME(m_cursor_blink_count));
	save_item(NAME(m_update_addr));
	save_item(NAME(m_update_ready_bit));
	save_item(NAME(m_cur));
	save_item(NAME(m_hsync));
	save_item(NAME(m_vsync));
	save_item(NAME(m_de));
	save_item(NAME(m_character_counter));
	save_item(NAME(m_hsync_width_counter));
	save_item(NAME(m_line_counter));
	save_item(NAME(m_raster_counter));
	save_item(NAME(m_adjust_counter));
	save_item(NAME(m_vsync_width_counter));
	save_item(NAME(m_line_enable_ff));
	save_item(NAME(m_vsync_ff));
	save_item(NAME(m_adjust_active));
	save_item(NAME(m_line_address));
	save_item(NAME(m_cursor_x));
	save_item(NAME(m_has_valid_parameters));
}


void mc6845_1_device::device_start()
{
	mc6845_device::device_start();

	m_supports_disp_start_addr_r = true;
	m_supports_vert_sync_width = true;
	m_supports_status_reg_d5 = false;
	m_supports_status_reg_d6 = false;
	m_supports_status_reg_d7 = false;
	m_supports_transparent = false;
}


void c6545_1_device::device_start()
{
	mc6845_device::device_start();

	m_supports_disp_start_addr_r = false;
	m_supports_vert_sync_width = true;
	m_supports_status_reg_d5 = true;
	m_supports_status_reg_d6 = true;
	m_supports_status_reg_d7 = false;
	m_supports_transparent = false;
}


void r6545_1_device::device_start()
{
	mc6845_device::device_start();

	m_supports_disp_start_addr_r = false;
	m_supports_vert_sync_width = true;
	m_supports_status_reg_d5 = true;
	m_supports_status_reg_d6 = true;
	m_supports_status_reg_d7 = true;
	m_supports_transparent = true;
}


void hd6845s_device::device_start()
{
	mc6845_device::device_start();

	m_supports_disp_start_addr_r = true;  // HD6845S can definitely read Display Start (double checked on datasheet)
	m_supports_vert_sync_width = true;
	m_supports_status_reg_d5 = false;
	m_supports_status_reg_d6 = false;
	m_supports_status_reg_d7 = false;
	m_supports_transparent = false;

	// Non-interlace Mode, Interlace Sync Mode - When total number of rasters is RN, RN-1 shall be programmed.
	m_noninterlace_adjust = 1;
	// Interlace Sync & Video Mode - When total number of rasters is RN, RN-2 shall be programmed.
	m_interlace_adjust = 2;
}


void sy6545_1_device::device_start()
{
	mc6845_device::device_start();

	m_supports_disp_start_addr_r = false;
	m_supports_vert_sync_width = true;
	m_supports_status_reg_d5 = true;
	m_supports_status_reg_d6 = true;
	m_supports_status_reg_d7 = true;
	m_supports_transparent = true;
}


void sy6845e_device::device_start()
{
	mc6845_device::device_start();

	m_supports_disp_start_addr_r = false;
	m_supports_vert_sync_width = true;
	m_supports_status_reg_d5 = true;
	m_supports_status_reg_d6 = true;
	m_supports_status_reg_d7 = true;
	m_supports_transparent = true;
}


void hd6345_device::device_start()
{
	hd6845s_device::device_start();

	m_disp2_pos = 0;
	m_disp3_pos = 0;
	m_disp4_pos = 0;
	m_disp2_start_addr = 0;
	m_disp3_start_addr = 0;
	m_disp4_start_addr = 0;
	m_vert_sync_pos_adj = 0;
	m_smooth_scroll_ras = 0;
	m_mem_width_offs = 0;
	m_cursor2_start_ras = 0;
	m_cursor2_end_ras = 0;
	m_cursor2_addr = 0;
	m_cursor_width = 0;
	m_cursor2_width = 0;

	save_item(NAME(m_disp2_pos));
	save_item(NAME(m_disp2_start_addr));
	save_item(NAME(m_disp3_pos));
	save_item(NAME(m_disp3_start_addr));
	save_item(NAME(m_disp4_pos));
	save_item(NAME(m_disp4_start_addr));
	save_item(NAME(m_vert_sync_pos_adj));
	save_item(NAME(m_smooth_scroll_ras));
	save_item(NAME(m_control1));
	save_item(NAME(m_control2));
	save_item(NAME(m_control3));
	save_item(NAME(m_mem_width_offs));
	save_item(NAME(m_cursor2_start_ras));
	save_item(NAME(m_cursor2_end_ras));
	save_item(NAME(m_cursor2_addr));
	save_item(NAME(m_cursor_width));
	save_item(NAME(m_cursor2_width));
}


void ams40489_device::device_start()
{
	mc6845_device::device_start();

	m_supports_disp_start_addr_r = true;
	m_supports_vert_sync_width = false;
	m_supports_status_reg_d5 = false;
	m_supports_status_reg_d6 = false;
	m_supports_status_reg_d7 = false;
	m_supports_transparent = false;
}


void mc6845_device::device_reset()
{
	/* internal registers other than status remain unchanged, all outputs go low */
	m_out_de_cb(false);

	m_out_hsync_cb(false);

	m_out_vsync_cb(false);

	if (!m_line_timer->enabled() && m_has_valid_parameters)
	{
		m_line_timer->adjust(cclks_to_attotime(m_horiz_char_total + 1));
	}

	m_light_pen_latched = false;

	// TODO: as per the note above, none of these should reset unless otherwise proven.
	m_cursor_addr = 0;
	m_line_address = 0;
	// bml3 in particular disagrees with these two.
	//m_horiz_disp = 0;
	//m_mode_control = 0;
	m_cursor_x = 0;
	m_register_address_latch = 0;
	m_update_addr = 0;
	m_light_pen_addr = 0;
}


void r6545_1_device::device_reset() { mc6845_device::device_reset(); }
void mc6845_1_device::device_reset() { mc6845_device::device_reset(); }
void hd6845s_device::device_reset() { mc6845_device::device_reset(); }
void c6545_1_device::device_reset() { mc6845_device::device_reset(); }
void sy6545_1_device::device_reset() { mc6845_device::device_reset(); }
void sy6845e_device::device_reset() { mc6845_device::device_reset(); }

void hd6345_device::device_reset()
{
	hd6845s_device::device_reset();

	m_control1 = 0;
	m_control2 = 0;
	m_control3 = 0;
}

void ams40489_device::device_reset() { mc6845_device::device_reset(); }


r6545_1_device::r6545_1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mc6845_device(mconfig, R6545_1, tag, owner, clock)
{
}


mc6845_1_device::mc6845_1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mc6845_device(mconfig, MC6845_1, tag, owner, clock)
{
}


hd6845s_device::hd6845s_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: mc6845_device(mconfig, type, tag, owner, clock)
{
}


hd6845s_device::hd6845s_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mc6845_device(mconfig, HD6845S, tag, owner, clock)
{
}


c6545_1_device::c6545_1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mc6845_device(mconfig, C6545_1, tag, owner, clock)
{
}


sy6545_1_device::sy6545_1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mc6845_device(mconfig, SY6545_1, tag, owner, clock)
{
}


sy6845e_device::sy6845e_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mc6845_device(mconfig, SY6845E, tag, owner, clock)
{
}


hd6345_device::hd6345_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hd6845s_device(mconfig, HD6345, tag, owner, clock)
{
}


ams40489_device::ams40489_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mc6845_device(mconfig, AMS40489, tag, owner, clock)
{
}
