// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/**********************************************************************

    TI TMS9927 and compatible CRT controller emulation

**********************************************************************/

#include "emu.h"
#include "video/tms9927.h"

#include "screen.h"


static constexpr uint8_t chars_per_row_value[8] = { 20, 32, 40, 64, 72, 80, 96, 132 };
static constexpr uint8_t skew_bits_value[4] = { 0, 1, 2, 2 };


#define HCOUNT               (m_reg[0] + 1)
#define INTERLACED           ((m_reg[1] >> 7) & 0x01)
#define HSYNC_WIDTH          (((m_reg[1] >> 3) & 0x0f) + 1)
#define HSYNC_DELAY          (((m_reg[1] >> 0) & 0x07) + 1)
#define SCANS_PER_DATA_ROW   (((m_reg[2] >> 3) & 0x0f) + 1)
#define CHARS_PER_DATA_ROW   (chars_per_row_value[(m_reg[2] >> 0) & 0x07])
#define SKEW_BITS            (skew_bits_value[(m_reg[3] >> 6) & 0x03])
#define DATA_ROWS_PER_FRAME  ((m_reg[3] & 0x3f) + 1)
#define SCAN_LINES_PER_FRAME ((m_reg[4] * 2) + 256 + (((m_reg[1] >> 7) & 0x01) * 257))
#define VERTICAL_DATA_START  (m_reg[5])
#define LAST_DISP_DATA_ROW   (m_reg[6] & 0x3f)
#define CURSOR_CHAR_ADDRESS  (m_reg[7])
#define CURSOR_ROW_ADDRESS   (m_reg[8] & 0x3f)


DEFINE_DEVICE_TYPE(TMS9927, tms9927_device, "tms9927", "TMS9927 VTC")
DEFINE_DEVICE_TYPE(CRT5027, crt5027_device, "crt5027", "CRT5027 VTAC")
DEFINE_DEVICE_TYPE(CRT5037, crt5037_device, "crt5037", "CRT5037 VTAC")
DEFINE_DEVICE_TYPE(CRT5057, crt5057_device, "crt5057", "CRT5057 VTAC")

tms9927_device::tms9927_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tms9927_device(mconfig, TMS9927, tag, owner, clock)
{
}

tms9927_device::tms9927_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, m_write_vsyn(*this)
	, m_write_hsyn(*this)
	, m_hpixels_per_column(0)
	, m_overscan_left(0)
	, m_overscan_right(0)
	, m_overscan_top(0)
	, m_overscan_bottom(0)
	, m_selfload(*this, finder_base::DUMMY_TAG)
	, m_reset(false)
	, m_valid_config(false)
{
	std::fill(std::begin(m_reg), std::end(m_reg), 0x00);
}

crt5027_device::crt5027_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tms9927_device(mconfig, CRT5027, tag, owner, clock)
{
}

crt5037_device::crt5037_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tms9927_device(mconfig, CRT5037, tag, owner, clock)
{
}

crt5057_device::crt5057_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tms9927_device(mconfig, CRT5057, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tms9927_device::device_start()
{
	assert(clock() > 0);
	if (!(m_hpixels_per_column > 0)) fatalerror("TMS9927: number of pixels per column must be explicitly set using MCFG_TMS9927_CHAR_WIDTH()!\n");

	// resolve callbacks
	m_write_vsyn.resolve_safe();
	m_write_hsyn.resolve();

	// allocate timers
	m_vsync_timer = timer_alloc(TIMER_VSYNC);
	m_hsync_timer = timer_alloc(TIMER_HSYNC);

	// register for state saving
	save_item(NAME(m_reg));
	save_item(NAME(m_start_datarow));
	save_item(NAME(m_reset));
	save_item(NAME(m_vsyn));
	save_item(NAME(m_hsyn));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tms9927_device::device_reset()
{
	m_start_datarow = 0;
}

//-------------------------------------------------
//  device_clock_changed - called when the
//  device clock is altered in any way
//-------------------------------------------------

void tms9927_device::device_clock_changed()
{
	if (m_valid_config && !m_reset)
		recompute_parameters(false);
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void tms9927_device::device_stop()
{
	osd_printf_debug("TMS9927: Final params: (%d, %d, %d, %d, %d, %d, %d)\n",
						clock() * m_hpixels_per_column,
						m_total_hpix,
						0, m_visible_hpix,
						m_total_vpix,
						0, m_visible_vpix);
}



//-------------------------------------------------
//  device_timer - handle timer events
//-------------------------------------------------

void tms9927_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_VSYNC:
		m_vsyn = !m_vsyn;
		m_write_vsyn(m_vsyn ? 1 : 0);

		if (m_vsyn)
		{
			m_vsync_timer->adjust(screen().time_until_pos(m_vsyn_end, m_hsyn_start));
		}
		else
		{
			m_vsync_timer->adjust(screen().time_until_pos(m_vsyn_start, m_hsyn_start));
		}
		break;

	case TIMER_HSYNC:
		m_hsyn = !m_hsyn;
		m_write_hsyn(m_hsyn ? 1 : 0);

		uint16_t vpos = screen().vpos();
		if (m_hsyn)
		{
			screen().update_now();
			if (screen().hpos() > m_hsyn_end)
				vpos = (vpos + 1) % m_total_vpix;
			m_hsync_timer->adjust(screen().time_until_pos(vpos, m_hsyn_end));
		}
		else
		{
			if (screen().hpos() > m_hsyn_start)
				vpos = (vpos + 1) % m_total_vpix;
			m_hsync_timer->adjust(screen().time_until_pos(vpos, m_hsyn_start));
		}
		break;
	}
}

void tms9927_device::device_post_load()
{
	recompute_parameters(true);
}


void tms9927_device::generic_access(address_space &space, offs_t offset)
{
	switch (offset)
	{
		case 0x07:  /* Processor Self Load */
		case 0x0f:  /* Non-processor self-load */
			if (m_selfload.found())
			{
				for (int cur = 0; cur < 7; cur++)
					write(space, cur, m_selfload[cur]);
				for (int cur = 0; cur < 1; cur++)
					write(space, cur + 0xc, m_selfload[cur + 7]);
			}
			else
				popmessage("tms9927: self-load initiated with no PROM!");

			/* processor self-load waits with reset enabled;
			   non-processor just goes ahead */
			m_reset = (offset == 0x07);
			break;

		case 0x0a:  /* Reset */
			if (!m_reset)
			{
				screen().update_now();
				m_reset = true;
			}
			break;

		case 0x0b:  /* Up scroll */
			screen().update_now();
			m_start_datarow = (m_start_datarow + 1) % DATA_ROWS_PER_FRAME;
			break;

		case 0x0e:  /* Start timing chain */
			if (m_reset)
			{
				screen().update_now();
				m_reset = false;
				recompute_parameters(false);
			}
			break;
	}
}

WRITE8_MEMBER( tms9927_device::write )
{
	switch (offset)
	{
		case 0x00:  /* HORIZONTAL CHARACTER COUNT */
		case 0x01:  /* INTERLACED / HSYNC WIDTH / HSYNC DELAY */
		case 0x02:  /* SCANS PER DATA ROW / CHARACTERS PER DATA ROW */
		case 0x03:  /* SKEW BITS / DATA ROWS PER FRAME */
		case 0x04:  /* SCAN LINES / FRAME */
		case 0x05:  /* VERTICAL DATA START */
			m_reg[offset] = data;
			recompute_parameters(false);
			break;

		case 0x06:  /* LAST DISPLAYED DATA ROW */
			// TVI-912 writes to this register frequently
			if (m_reg[offset] != data)
			{
				m_reg[offset] = data;
				recompute_parameters(false);
			}
			break;

		case 0x0c:  /* LOAD CURSOR CHARACTER ADDRESS */
		case 0x0d:  /* LOAD CURSOR ROW ADDRESS */
			m_reg[offset - 0x0c + 7] = data;
			/* Recomputing parameters here will break the scrollup on the Attachè
			   and probably other machines due to m_start_datarow being reset ! */
			//recompute_parameters(false);
			break;

		default:
			generic_access(space, offset);
			break;
	}
}


READ8_MEMBER( tms9927_device::read )
{
	switch (offset)
	{
		case 0x08:  /* READ CURSOR CHARACTER ADDRESS */
		case 0x09:  /* READ CURSOR ROW ADDRESS */
			return m_reg[offset - 0x08 + 7];

		default:
			if (!machine().side_effects_disabled())
				generic_access(space, offset);
			break;
	}
	return 0xff;
}


READ_LINE_MEMBER(tms9927_device::bl_r)
{
	return (m_reset || screen().vblank() || screen().hblank()) ? 1 : 0;
}


bool tms9927_device::cursor_bounds(rectangle &bounds) const
{
	int cursorx = CURSOR_CHAR_ADDRESS;
	int cursory = (CURSOR_ROW_ADDRESS + DATA_ROWS_PER_FRAME - m_start_datarow) % DATA_ROWS_PER_FRAME;

	bounds.min_x = cursorx * m_hpixels_per_column;
	bounds.max_x = bounds.min_x + m_hpixels_per_column - 1;
	bounds.min_y = cursory * SCANS_PER_DATA_ROW;
	bounds.max_y = bounds.min_y + SCANS_PER_DATA_ROW - 1;

	return (cursorx < HCOUNT && cursory <= LAST_DISP_DATA_ROW);
}


void tms9927_device::recompute_parameters(bool postload)
{
	if (m_reset)
		return;

	/* compute the screen sizes */
	m_total_hpix = HCOUNT * m_hpixels_per_column;
	m_total_vpix = SCAN_LINES_PER_FRAME;

	/* determine the visible area, avoid division by 0 */
	m_visible_hpix = CHARS_PER_DATA_ROW * m_hpixels_per_column;
	m_visible_vpix = DATA_ROWS_PER_FRAME * SCANS_PER_DATA_ROW;

	m_start_datarow = (LAST_DISP_DATA_ROW + 1) % DATA_ROWS_PER_FRAME;

	m_hsyn_start = (m_visible_hpix + m_overscan_left + HSYNC_DELAY * m_hpixels_per_column) % m_total_hpix;
	m_hsyn_end = (m_hsyn_start + HSYNC_WIDTH * m_hpixels_per_column) % m_total_hpix;

	m_vsyn_start = (m_total_vpix + m_overscan_top - VERTICAL_DATA_START) % m_total_vpix;
	m_vsyn_end = (m_vsyn_start + 3) % m_total_vpix;

	/* see if it all makes sense */
	m_valid_config = true;
	if ( (m_visible_hpix > m_total_hpix || m_visible_vpix > m_total_vpix) || (((m_visible_hpix-1)<=0) || ((m_visible_vpix-1)<=0)) || ((m_total_hpix * m_total_vpix) == 0) )
	{
		m_valid_config = false;
		logerror("tms9927: invalid visible size (%dx%d) versus total size (%dx%d)\n", m_visible_hpix, m_visible_vpix, m_total_hpix, m_total_vpix);
	}

	if (clock() == 0)
	{
		m_valid_config = false;
		// TODO: make the screen refresh never, and disable the vblank and odd/even interrupts here!
		logerror("tms9927: invalid clock rate of zero defined!\n");
	}

	/* update */
	if (!m_valid_config)
		return;

	/* create a visible area */
	rectangle visarea(0, m_overscan_left + m_visible_hpix + m_overscan_right - 1,
				0, m_overscan_top + m_visible_vpix + m_overscan_bottom - 1);

	attotime refresh = clocks_to_attotime(HCOUNT * m_total_vpix);

	osd_printf_debug("TMS9927: Total = %dx%d, Visible = %dx%d, HSync = %d-%d, VSync = %d-%d, Skew=%d, Upscroll=%d, Period=%f Hz\n", m_total_hpix, m_total_vpix, m_visible_hpix, m_visible_vpix, m_hsyn_start, m_hsyn_end, m_vsyn_start, m_vsyn_end, SKEW_BITS, m_start_datarow, refresh.as_hz());

	screen().configure(m_total_hpix, m_total_vpix, visarea, refresh.as_attoseconds());

	m_hsyn = false;
	if (!m_write_hsyn.isnull())
	{
		m_write_hsyn(0);
		m_hsync_timer->adjust(screen().time_until_pos(m_vsyn_start, m_hsyn_start));
	}

	m_vsyn = false;
	m_write_vsyn(0);
	m_vsync_timer->adjust(screen().time_until_pos(m_vsyn_start, m_hsyn_start));
}
