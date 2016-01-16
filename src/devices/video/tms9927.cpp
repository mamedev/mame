// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/**********************************************************************

    TI TMS9927 and compatible CRT controller emulation

**********************************************************************/

#include "emu.h"
#include "video/tms9927.h"


static const UINT8 chars_per_row_value[8] = { 20, 32, 40, 64, 72, 80, 96, 132 };
static const UINT8 skew_bits_value[4] = { 0, 1, 2, 2 };


#define HCOUNT               (m_reg[0] + 1)
#define INTERLACED           ((m_reg[1] >> 7) & 0x01)
#define HSYNC_WIDTH          ((m_reg[1] >> 4) & 0x0f)
#define HSYNC_DELAY          ((m_reg[1] >> 0) & 0x07)
#define SCANS_PER_DATA_ROW   (((m_reg[2] >> 3) & 0x0f) + 1)
#define CHARS_PER_DATA_ROW   (chars_per_row_value[(m_reg[2] >> 0) & 0x07])
#define SKEW_BITS            (skew_bits_value[(m_reg[3] >> 6) & 0x03])
#define DATA_ROWS_PER_FRAME  (((m_reg[3] >> 0) & 0x3f) + 1)
#define SCAN_LINES_PER_FRAME ((m_reg[4] * 2) + 256)
#define VERTICAL_DATA_START  (m_reg[5])
#define LAST_DISP_DATA_ROW   (m_reg[6] & 0x3f)
#define CURSOR_CHAR_ADDRESS  (m_reg[7])
#define CURSOR_ROW_ADDRESS   (m_reg[8] & 0x3f)


const device_type TMS9927 = &device_creator<tms9927_device>;
const device_type CRT5027 = &device_creator<crt5027_device>;
const device_type CRT5037 = &device_creator<crt5037_device>;
const device_type CRT5057 = &device_creator<crt5057_device>;

tms9927_device::tms9927_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
				: device_t(mconfig, TMS9927, "TMS9927 VTC", tag, owner, clock, "tms9927", __FILE__),
					device_video_interface(mconfig, *this),
					m_write_vsyn(*this),
					m_hpixels_per_column(0),
					m_reset(0)
{
	memset(m_reg, 0x00, sizeof(m_reg));
}

tms9927_device::tms9927_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source)
				: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
					device_video_interface(mconfig, *this),
					m_write_vsyn(*this),
					m_hpixels_per_column(0),
					m_reset(0)
{
	memset(m_reg, 0x00, sizeof(m_reg));
}

crt5027_device::crt5027_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
				: tms9927_device(mconfig, CRT5027, "CRT5027", tag, owner, clock, "crt5027", __FILE__)
{
}

crt5037_device::crt5037_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
				: tms9927_device(mconfig, CRT5037, "CRT5037", tag, owner, clock, "crt5037", __FILE__)
{
}

crt5057_device::crt5057_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
				: tms9927_device(mconfig, CRT5057, "CRT5057", tag, owner, clock, "crt5057", __FILE__)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tms9927_device::device_start()
{
	assert(clock() > 0);
	assert(m_hpixels_per_column > 0);

	/* copy the initial parameters */
	m_clock = clock();

	/* get the self-load PROM */
	if (!m_selfload_region.empty())
	{
		m_selfload = machine().root_device().memregion(m_selfload_region)->base();
		assert(m_selfload != nullptr);
	}

	// resolve callbacks
	m_write_vsyn.resolve_safe();

	// allocate timers
	m_vsync_timer = timer_alloc(TIMER_VSYNC);

	/* register for state saving */
	machine().save().register_postload(save_prepost_delegate(FUNC(tms9927_device::state_postload), this));

	save_item(NAME(m_reg));
	save_item(NAME(m_start_datarow));
	save_item(NAME(m_reset));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tms9927_device::device_reset()
{
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void tms9927_device::device_stop()
{
	osd_printf_debug("TMS9937: Final params: (%d, %d, %d, %d, %d, %d, %d)\n",
						m_clock,
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

		m_write_vsyn(m_vsyn);

		if (m_vsyn)
		{
			m_vsync_timer->adjust(m_screen->time_until_pos(3));
		}
		else
		{
			m_vsync_timer->adjust(m_screen->time_until_pos(0));
		}
		break;
	}
}

void tms9927_device::state_postload()
{
	recompute_parameters(TRUE);
}


void tms9927_device::generic_access(address_space &space, offs_t offset)
{
	switch (offset)
	{
		case 0x07:  /* Processor Self Load */
		case 0x0f:  /* Non-processor self-load */
			if (m_selfload != nullptr)
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
				m_screen->update_now();
				m_reset = TRUE;
			}
			break;

		case 0x0b:  /* Up scroll */
osd_printf_debug("Up scroll\n");
			m_screen->update_now();
			m_start_datarow = (m_start_datarow + 1) % DATA_ROWS_PER_FRAME;
			break;

		case 0x0e:  /* Start timing chain */
			if (m_reset)
			{
				m_screen->update_now();
				m_reset = FALSE;
				recompute_parameters(FALSE);
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
		case 0x06:  /* LAST DISPLAYED DATA ROW */
			m_reg[offset] = data;
			recompute_parameters(FALSE);
			break;

		case 0x0c:  /* LOAD CURSOR CHARACTER ADDRESS */
		case 0x0d:  /* LOAD CURSOR ROW ADDRESS */
osd_printf_debug("Cursor address changed\n");
			m_reg[offset - 0x0c + 7] = data;
			recompute_parameters(FALSE);
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
			generic_access(space, offset);
			break;
	}
	return 0xff;
}


int tms9927_device::screen_reset()
{
	return m_reset;
}


int tms9927_device::upscroll_offset()
{
	return m_start_datarow;
}


int tms9927_device::cursor_bounds(rectangle &bounds)
{
	int cursorx = CURSOR_CHAR_ADDRESS;
	int cursory = CURSOR_ROW_ADDRESS;

	bounds.min_x = cursorx * m_hpixels_per_column;
	bounds.max_x = bounds.min_x + m_hpixels_per_column - 1;
	bounds.min_y = cursory * SCANS_PER_DATA_ROW;
	bounds.max_y = bounds.min_y + SCANS_PER_DATA_ROW - 1;

	return (cursorx < HCOUNT && cursory <= LAST_DISP_DATA_ROW);
}


void tms9927_device::recompute_parameters(int postload)
{
	UINT16 offset_hpix, offset_vpix;
	attoseconds_t refresh;
	rectangle visarea;

	if (m_reset)
		return;

	/* compute the screen sizes */
	m_total_hpix = HCOUNT * m_hpixels_per_column;
	m_total_vpix = SCAN_LINES_PER_FRAME;

	/* determine the visible area, avoid division by 0 */
	m_visible_hpix = CHARS_PER_DATA_ROW * m_hpixels_per_column;
	m_visible_vpix = (LAST_DISP_DATA_ROW + 1) * SCANS_PER_DATA_ROW;

	/* determine the horizontal/vertical offsets */
	offset_hpix = HSYNC_DELAY * m_hpixels_per_column;
	offset_vpix = VERTICAL_DATA_START;

	osd_printf_debug("TMS9937: Total = %dx%d, Visible = %dx%d, Offset=%dx%d, Skew=%d\n", m_total_hpix, m_total_vpix, m_visible_hpix, m_visible_vpix, offset_hpix, offset_vpix, SKEW_BITS);

	/* see if it all makes sense */
	m_valid_config = TRUE;
	if (m_visible_hpix > m_total_hpix || m_visible_vpix > m_total_vpix)
	{
		m_valid_config = FALSE;
		logerror("tms9927: invalid visible size (%dx%d) versus total size (%dx%d)\n", m_visible_hpix, m_visible_vpix, m_total_hpix, m_total_vpix);
	}

	/* update */
	if (!m_valid_config)
		return;

	/* create a visible area */
	/* fix me: how do the offsets fit in here? */
	visarea.set(0, m_visible_hpix - 1, 0, m_visible_vpix - 1);

	refresh = HZ_TO_ATTOSECONDS(m_clock) * m_total_hpix * m_total_vpix;

	m_screen->configure(m_total_hpix, m_total_vpix, visarea, refresh);

	m_vsyn = 0;
	m_vsync_timer->adjust(m_screen->time_until_pos(0, 0));
}
