// license:BSD-3-Clause
// copyright-holders:Couriersud
/***************************************************************************

    fixfreq.h

    2013 Couriersud

    Fixed frequency monochrome monitor emulation

    The driver is intended for drivers which provide an analog video signal.
    VSYNC and HSYNC levels are used to create the bitmap.

***************************************************************************/

#include "emu.h"
#include "fixfreq.h"

//#define VERBOSE 1
#include "logmacro.h"

/***************************************************************************

    Fixed frequency monitor

***************************************************************************/



// device type definition
DEFINE_DEVICE_TYPE(FIXFREQ, fixedfreq_device, "fixfreq", "Fixed-Frequency Monochrome Monitor")

void fixedfreq_monitor_state::update_sync_channel(const time_type &time, const double newval)
{
	const time_type delta_time = time - m_last_sync_time;

	const int last_vsync = m_sig_vsync;
	const int last_comp = m_sig_composite;

	m_vsync_filter += ((double) last_comp - m_vsync_filter) * (1.0 - exp(-delta_time * m_vsync_filter_timeconst));
	m_sig_composite = (newval < m_desc.m_sync_threshold) ? 1 : 0 ;

	m_sig_vsync = (m_vsync_filter > m_vsync_threshold) ? 1 : 0;

	if (!last_vsync && m_sig_vsync)
	{
		//LOG("VSYNC %d %d\n", m_last_x, m_last_y + m_sig_field);
		m_last_y = m_desc.m_vbackporch - m_desc.m_vsync;
		m_intf.vsync_start_cb(time - m_last_vsync_time);
		m_last_vsync_time = time;
	}
	else if (last_vsync && !m_sig_vsync)
	{
		m_sig_field = last_comp;   /* force false-progressive */
		m_sig_field = (m_sig_field ^ 1) ^ last_comp;   /* if there is no field switch, auto switch */
		//LOG("Field: %d\n", m_sig_field);
	}

	if (!last_comp && m_sig_composite)
	{
		/* TODO - time since last hsync and field detection */
		//LOG("HSYNC up %d\n", m_last_x);
		// FIXME: pixels > 50 filters some spurious hysnc on line 27 in breakout
		if (!m_sig_vsync && (m_last_x > m_desc.m_hscale * 100))
		{
			m_last_y += m_desc.m_fieldcount;
			m_last_x = 0;
			m_line_time = time;
		}
		//if (m_last_y == 27) printf("HSYNC up %d %d\n", m_last_y, pixels);
	}
	else if (last_comp && !m_sig_composite)
	{
		/* falling composite */
		//LOG("HSYNC down %f %d %f\n", time * 1e6, m_last_x, m_sync_signal);
	}

	m_sync_signal = newval;
	m_last_sync_time = time;
}

void fixedfreq_monitor_state::update_bm(const time_type &time)
{
	const int pixels = round((time - m_line_time) * m_desc.m_hscale / m_clock_period);
	const int has_fields = (m_desc.m_fieldcount > 1) ? 1: 0;

	uint32_t col(0xffff0000); // Mark sync areas

	if (m_sync_signal >= m_desc.m_sync_threshold)
		col = m_col;

	m_intf.plot_hline(m_last_x, m_last_y + m_sig_field * has_fields, pixels - m_last_x, col);
	m_last_x = pixels;
}

void fixedfreq_monitor_state::update_composite_monochrome(const time_type &time, const double data)
{
	update_bm(time);
	update_sync_channel(time, data);

	int colv = (int) ((data - m_desc.m_sync_threshold) * m_desc.m_gain * 255.0);
	if (colv > 255)
		colv = 255;
	if (colv < 0)
		m_col = 0xffff0000;
	else
		m_col = 0xff000000 | (colv<<16) | (colv<<8) | colv;
}

void fixedfreq_monitor_state::update_red(const time_type &time, const double data)
{
	update_bm(time);

	int colv = (int) ((data - m_desc.m_sync_threshold) * m_desc.m_gain * 255.0);
	if (colv > 255)
		colv = 255;
	if (colv < 0)
		colv = 0;
	m_col = (m_col & 0xff00ffff) | (colv<<16);
}

void fixedfreq_monitor_state::update_green(const time_type &time, const double data)
{
	update_bm(time);
	//update_sync_channel(ctime, data);

	int colv = (int) ((data - m_desc.m_sync_threshold) * m_desc.m_gain * 255.0);
	if (colv > 255)
		colv = 255;
	if (colv < 0)
		colv = 0;
	m_col = (m_col & 0xffff00ff) | (colv<<8);
}

void fixedfreq_monitor_state::update_blue(const time_type &time, const double data)
{
	update_bm(time);
	//update_sync_channel(ctime, data);

	int colv = (int) ((data - m_desc.m_sync_threshold) * m_desc.m_gain * 255.0);
	if (colv > 255)
		colv = 255;
	if (colv < 0)
		colv = 0;
	m_col = (m_col & 0xffffff00) | colv;

}

void fixedfreq_monitor_state::update_sync(const time_type &time, const double data)
{
	update_bm(time);
	update_sync_channel(time, data);
}

fixedfreq_device::fixedfreq_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock),
		device_video_interface(mconfig, *this, false),
		m_cur_bm(0),
		m_htotal(0),
		m_vtotal(0),
		m_refresh_period(time_type(0)),
		m_monitor(),
		m_state(m_monitor, *this)
{
}

fixedfreq_device::fixedfreq_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: fixedfreq_device(mconfig, FIXFREQ, tag, owner, clock)
{
}

void fixedfreq_device::device_config_complete()
{
	if (!has_screen())
		return;

	if (!screen().refresh_attoseconds())
		screen().set_raw(m_monitor.m_monitor_clock, m_monitor.m_hbackporch, 0,
			m_monitor.m_hbackporch, m_monitor.m_vbackporch, 0,
			m_monitor.m_vbackporch);

	if (!screen().has_screen_update())
		screen().set_screen_update(*this, FUNC(fixedfreq_device::screen_update));
}

void fixedfreq_device::device_start()
{

	m_refresh_period = time_type(0);

	m_cur_bm = 0;

	m_htotal = m_monitor.m_hbackporch;
	m_vtotal = m_monitor.m_vbackporch;
	m_bitmap[0] = std::make_unique<bitmap_rgb32>(m_htotal * m_monitor.m_hscale, m_vtotal);
	m_bitmap[1] = std::make_unique<bitmap_rgb32>(m_htotal * m_monitor.m_hscale, m_vtotal);

	m_state.start();

	// FIXME: will be done by netlist going forward
	save_item(NAME(m_state.m_sync_signal));
	save_item(NAME(m_state.m_last_x));
	save_item(NAME(m_state.m_last_y));
	save_item(NAME(m_state.m_last_sync_time));
	save_item(NAME(m_state.m_line_time));
	save_item(NAME(m_state.m_last_hsync_time));
	save_item(NAME(m_state.m_last_vsync_time));
	save_item(NAME(m_refresh_period));
	save_item(NAME(m_state.m_clock_period));
	//save_item(NAME(m_bitmap[0]));
	//save_item(NAME(m_bitmap[1]));
	save_item(NAME(m_cur_bm));

	/* sync separator */
	save_item(NAME(m_state.m_vsync_filter));
	save_item(NAME(m_state.m_vsync_threshold));
	save_item(NAME(m_state.m_vsync_filter_timeconst));

	save_item(NAME(m_state.m_sig_vsync));
	save_item(NAME(m_state.m_sig_composite));
	save_item(NAME(m_state.m_sig_field));
}

void fixedfreq_device::device_reset()
{
	m_state.reset();
}

void fixedfreq_device::device_post_load()
{
	//recompute_parameters();
}

uint32_t fixedfreq_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, *m_bitmap[!m_cur_bm], 0, 0, 0, 0, cliprect);

	return 0;
}

void fixedfreq_device::vsync_start_cb(double refresh_time)
{
	// toggle bitmap
	m_cur_bm ^= 1;

	rectangle visarea(m_monitor.minh(), m_monitor.maxh(), m_monitor.minv(), m_monitor.maxv());

	m_refresh_period = refresh_time;
	screen().configure(m_htotal * m_monitor.m_hscale, m_vtotal, visarea, DOUBLE_TO_ATTOSECONDS(m_refresh_period));
}

void fixedfreq_device::plot_hline(int x, int y, int w, uint32_t col)
{
	bitmap_rgb32 *bm = m_bitmap[m_cur_bm].get();
	if (y < bm->height())
		bm->plot_box(x, y, w, 1, col);
}

NETDEV_ANALOG_CALLBACK_MEMBER(fixedfreq_device::update_composite_monochrome)
{
	// double is good enough for this exercise;

	const time_type ctime = time.as_double();
	m_state.update_composite_monochrome(ctime, data);
}

NETDEV_ANALOG_CALLBACK_MEMBER(fixedfreq_device::update_red)
{
	// double is good enough for this exercise;

	const time_type ctime = time.as_double();
	m_state.update_red(ctime, data);
}

NETDEV_ANALOG_CALLBACK_MEMBER(fixedfreq_device::update_green)
{
	// double is good enough for this exercise;

	const time_type ctime = time.as_double();
	m_state.update_green(ctime, data);
}

NETDEV_ANALOG_CALLBACK_MEMBER(fixedfreq_device::update_blue)
{
	// double is good enough for this exercise;

	const time_type ctime = time.as_double();
	m_state.update_blue(ctime, data);
}

NETDEV_ANALOG_CALLBACK_MEMBER(fixedfreq_device::update_sync)
{
	// double is good enough for this exercise;

	const time_type ctime = time.as_double();
	m_state.update_sync(ctime, data);
}

/***************************************************************************/
