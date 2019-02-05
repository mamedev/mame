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

fixedfreq_device::fixedfreq_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock),
		device_video_interface(mconfig, *this, false),
		m_htotal(0),
		m_vtotal(0),
		m_hscale(1),    // FIXME: this should be modified by static initialization
		m_sync_signal(0),
		m_last_x(0),
		m_last_y(0),
		m_cur_bm(0),
		// default to NTSC "704x480@30i"
		m_monitor_clock(13500000),
		m_hvisible(704),
		m_hfrontporch(728),
		m_hsync(791),
		m_hbackporch(858),
		m_vvisible(480),
		m_vfrontporch(486),
		m_vsync(492),
		m_vbackporch(525),
		m_fieldcount(2),
		m_sync_threshold(0.3),
		m_gain(1.0 / 3.7),
		m_vsync_filter(0), m_vsync_threshold(0), m_vsync_filter_timeconst(0), m_sig_vsync(0), m_sig_composite(0), m_sig_field(0)
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
		screen().set_raw(m_monitor_clock, m_hbackporch, 0, m_hbackporch, m_vbackporch, 0, m_vbackporch);

	if (!screen().has_screen_update())
		screen().set_screen_update(screen_update_rgb32_delegate(FUNC(fixedfreq_device::screen_update), this));
}

void fixedfreq_device::device_start()
{
	m_htotal = 0;
	m_vtotal = 0;

	m_sync_signal = 0.0;
	m_col = rgb_t(0,0,0);
	m_last_x = 0;
	m_last_y = 0;
	m_last_sync_time = time_type(0);
	m_line_time = time_type(0);
	m_last_hsync_time = time_type(0);
	m_last_vsync_time = time_type(0);
	m_refresh_period = time_type(0);
	m_clock_period = time_type(0);
	//bitmap_rgb32 *m_bitmap[2];
	m_cur_bm = 0;

	/* sync separator */
	m_vsync_filter = 0.0;
	m_vsync_threshold = 0.0;
	m_vsync_filter_timeconst = 0.0;

	m_sig_vsync = 0;
	m_sig_composite = 0;
	m_sig_field = 0;

	m_bitmap[0] = nullptr;
	m_bitmap[1] = nullptr;
	//m_vblank_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(vga_device::vblank_timer_cb),this));
	recompute_parameters();

	save_item(NAME(m_sync_signal));
	save_item(NAME(m_last_x));
	save_item(NAME(m_last_y));
	save_item(NAME(m_last_sync_time));
	save_item(NAME(m_line_time));
	save_item(NAME(m_last_hsync_time));
	save_item(NAME(m_last_vsync_time));
	save_item(NAME(m_refresh_period));
	save_item(NAME(m_clock_period));
	//save_item(NAME(m_bitmap[0]));
	//save_item(NAME(m_bitmap[1]));
	save_item(NAME(m_cur_bm));

	/* sync separator */
	save_item(NAME(m_vsync_filter));
	save_item(NAME(m_vsync_threshold));
	save_item(NAME(m_vsync_filter_timeconst));

	save_item(NAME(m_sig_vsync));
	save_item(NAME(m_sig_composite));
	save_item(NAME(m_sig_field));



}

void fixedfreq_device::device_reset()
{
	m_last_sync_time = time_type(0);
	m_line_time = time_type(0);
	m_last_hsync_time = time_type(0);
	m_last_vsync_time = time_type(0);
	m_vsync_filter = 0;

}


void fixedfreq_device::device_post_load()
{
	//recompute_parameters();
}

void fixedfreq_device::recompute_parameters()
{
	bool needs_realloc = (m_htotal != m_hbackporch) && (m_vtotal != m_vbackporch);

	if (m_bitmap[0] != nullptr || needs_realloc)
		m_bitmap[0] = nullptr;
	if (m_bitmap[1] != nullptr || needs_realloc)
		m_bitmap[1] = nullptr;

	m_htotal = m_hbackporch;
	m_vtotal = m_vbackporch;

	/* sync separator */

	m_vsync_threshold = (exp(- 3.0/(3.0+3.0))) - exp(-1.0);
	m_vsync_filter_timeconst = (double) (m_monitor_clock) / (double) m_htotal * 1.0; // / (3.0 + 3.0);
	LOG("trigger %f with len %f\n", m_vsync_threshold, 1e6 / m_vsync_filter_timeconst);

	m_bitmap[0] = std::make_unique<bitmap_rgb32>(m_htotal * m_hscale, m_vtotal);
	m_bitmap[1] = std::make_unique<bitmap_rgb32>(m_htotal * m_hscale, m_vtotal);

	m_clock_period = 1.0 / m_monitor_clock;
	update_screen_parameters(m_clock_period * m_vtotal * m_htotal);
}

void fixedfreq_device::update_screen_parameters(const time_type &refresh)
{
	rectangle visarea(
			(m_hbackporch - m_hfrontporch) * m_hscale,
			(m_hbackporch - m_hfrontporch + m_hvisible) * m_hscale - 1,
			m_vbackporch - m_vfrontporch,
			m_vbackporch - m_vfrontporch + m_vvisible - 1);

	m_refresh_period = refresh;
	screen().configure(m_htotal * m_hscale, m_vtotal, visarea, DOUBLE_TO_ATTOSECONDS(m_refresh_period));
}

void fixedfreq_device::update_sync_channel(const time_type &time, const double newval)
{
	const time_type delta_time = time - m_last_sync_time;

	const int last_vsync = m_sig_vsync;
	const int last_comp = m_sig_composite;

	m_vsync_filter += ((double) last_comp - m_vsync_filter) * (1.0 - exp(-delta_time * m_vsync_filter_timeconst));
	m_sig_composite = (newval < m_sync_threshold) ? 1 : 0 ;

	m_sig_vsync = (m_vsync_filter > m_vsync_threshold) ? 1 : 0;

	if (!last_vsync && m_sig_vsync)
	{
		LOG("VSYNC %d %d\n", m_last_x, m_last_y + m_sig_field);
		m_last_y = m_vbackporch - m_vsync;
		// toggle bitmap
		m_cur_bm ^= 1;
		update_screen_parameters(time - m_last_vsync_time);
		m_last_vsync_time = time;
	}
	else if (last_vsync && !m_sig_vsync)
	{
		m_sig_field = last_comp;   /* force false-progressive */
		m_sig_field = (m_sig_field ^ 1) ^ last_comp;   /* if there is no field switch, auto switch */
		LOG("Field: %d\n", m_sig_field);
	}

	if (!last_comp && m_sig_composite)
	{
		/* TODO - time since last hsync and field detection */
		LOG("HSYNC up %d\n", m_last_x);
		// FIXME: pixels > 50 filters some spurious hysnc on line 27 in breakout
		if (!m_sig_vsync && (m_last_x > m_hscale * 100))
		{
			m_last_y += m_fieldcount;
			m_last_x = 0;
			m_line_time = time;
		}
		//if (m_last_y == 27) printf("HSYNC up %d %d\n", m_last_y, pixels);
	}
	else if (last_comp && !m_sig_composite)
	{
		/* falling composite */
		LOG("HSYNC down %f %d %f\n", time * 1e6, m_last_x, m_sync_signal);
	}

	m_sync_signal = newval;
	m_last_sync_time = time;
}

void fixedfreq_device::update_bm(const time_type &time)
{
	const int pixels = round((time - m_line_time) * m_hscale / m_clock_period);
	const int has_fields = (m_fieldcount > 1) ? 1: 0;

	bitmap_rgb32 *bm = m_bitmap[m_cur_bm].get();

	if (m_last_y < bm->height())
	{
		rgb_t col(255, 0, 0); // Mark sync areas

		if (m_sync_signal >= m_sync_threshold)
		{
			col = m_col;
		}
		bm->plot_box(m_last_x, m_last_y + m_sig_field * has_fields, pixels - m_last_x, 1, col);
		m_last_x = pixels;
	}
}

uint32_t fixedfreq_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, *m_bitmap[!m_cur_bm], 0, 0, 0, 0, cliprect);

	return 0;
}

NETDEV_ANALOG_CALLBACK_MEMBER(fixedfreq_device::update_composite_monochrome)
{
	// double is good enough for this exercise;

	const time_type ctime = time.as_double();
	update_bm(ctime);
	update_sync_channel(ctime, data);

	int colv = (int) ((data - m_sync_threshold) * m_gain * 255.0);
	if (colv > 255)
		colv = 255;
	m_col = rgb_t(colv, colv, colv);
}

NETDEV_ANALOG_CALLBACK_MEMBER(fixedfreq_device::update_red)
{
	// double is good enough for this exercise;

	const time_type ctime = time.as_double();
	update_bm(ctime);
	//update_sync_channel(ctime, data);

	int colv = (int) ((data - m_sync_threshold) * m_gain * 255.0);
	if (colv > 255)
		colv = 255;
	m_col.set_r(colv);
}

NETDEV_ANALOG_CALLBACK_MEMBER(fixedfreq_device::update_green)
{
	// double is good enough for this exercise;

	const time_type ctime = time.as_double();
	update_bm(ctime);
	//update_sync_channel(ctime, data);

	int colv = (int) ((data - m_sync_threshold) * m_gain * 255.0);
	if (colv > 255)
		colv = 255;
	m_col.set_g(colv);
}

NETDEV_ANALOG_CALLBACK_MEMBER(fixedfreq_device::update_blue)
{
	// double is good enough for this exercise;

	const time_type ctime = time.as_double();
	update_bm(ctime);
	//update_sync_channel(ctime, data);

	int colv = (int) ((data - m_sync_threshold) * m_gain * 255.0);
	if (colv > 255)
		colv = 255;
	m_col.set_b(colv);
}

NETDEV_ANALOG_CALLBACK_MEMBER(fixedfreq_device::update_sync)
{
	// double is good enough for this exercise;

	const time_type ctime = time.as_double();
	update_bm(ctime);
	update_sync_channel(ctime, data);
}

/***************************************************************************/
