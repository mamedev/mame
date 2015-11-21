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

/***************************************************************************

    Local variables

***************************************************************************/

//#define VERBOSE_OUT(x) printf x
#define VERBOSE_OUT(x)

/***************************************************************************

    Fixed frequency monitor

***************************************************************************/
// device type definition
const device_type FIXFREQ = &device_creator<fixedfreq_device>;

fixedfreq_device::fixedfreq_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_video_interface(mconfig, *this, false), m_htotal(0), m_vtotal(0), m_vid(0), m_last_x(0), m_last_y(0), m_cur_bm(0),
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
		m_gain(1.0 / 3.7), m_vint(0), m_int_trig(0), m_mult(0), m_sig_vsync(0), m_sig_composite(0), m_sig_field(0)
{
}

fixedfreq_device::fixedfreq_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, FIXFREQ, "Fixed Frequency Monochrome Monitor", tag, owner, clock, "fixfreq", __FILE__),
		device_video_interface(mconfig, *this, false),
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
		m_gain(1.0 / 3.7)
{
}

void fixedfreq_device::device_start()
{
	m_htotal = 0;
	m_vtotal = 0;

	m_vid = 0.0;
	m_last_x = 0;
	m_last_y = 0;
	m_last_time = attotime::zero;
	m_line_time = attotime::zero;
	m_last_hsync_time = attotime::zero;
	m_last_vsync_time = attotime::zero;
	m_refresh = attotime::zero;
	m_clock_period = attotime::zero;
	//bitmap_rgb32 *m_bitmap[2];
	m_cur_bm = 0;

	/* sync separator */
	m_vint = 0.0;
	m_int_trig = 0.0;
	m_mult = 0.0;

	m_sig_vsync = 0;
	m_sig_composite = 0;
	m_sig_field = 0;

	m_bitmap[0] = NULL;
	m_bitmap[1] = NULL;
	//m_vblank_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(vga_device::vblank_timer_cb),this));
	recompute_parameters(false);

	save_item(NAME(m_vid));
	save_item(NAME(m_last_x));
	save_item(NAME(m_last_y));
	save_item(NAME(m_last_time));
	save_item(NAME(m_line_time));
	save_item(NAME(m_last_hsync_time));
	save_item(NAME(m_last_vsync_time));
	save_item(NAME(m_refresh));
	save_item(NAME(m_clock_period));
	//save_item(NAME(m_bitmap[0]));
	//save_item(NAME(m_bitmap[1]));
	save_item(NAME(m_cur_bm));

	/* sync separator */
	save_item(NAME(m_vint));
	save_item(NAME(m_int_trig));
	save_item(NAME(m_mult));

	save_item(NAME(m_sig_vsync));
	save_item(NAME(m_sig_composite));
	save_item(NAME(m_sig_field));



}

void fixedfreq_device::device_reset()
{
	m_last_time = attotime::zero;
	m_line_time = attotime::zero;
	m_last_hsync_time = attotime::zero;
	m_last_vsync_time = attotime::zero;
	m_vint = 0;

}


void fixedfreq_device::device_post_load()
{
	//recompute_parameters(true);
}

void fixedfreq_device::recompute_parameters(bool postload)
{
	bool needs_realloc = (m_htotal != m_hbackporch) && (m_vtotal != m_vbackporch);

	if (m_bitmap[0] != NULL || needs_realloc)
		auto_free(machine(), m_bitmap[0]);
	if (m_bitmap[1] != NULL || needs_realloc)
		auto_free(machine(), m_bitmap[0]);

	m_htotal = m_hbackporch;
	m_vtotal = m_vbackporch;

	/* sync separator */

	m_int_trig = (exp(- 3.0/(3.0+3.0))) - exp(-1.0);
	m_mult = (double) (m_monitor_clock) / (double) m_htotal * 1.0; // / (3.0 + 3.0);
	VERBOSE_OUT(("trigger %f with len %f\n", m_int_trig, 1e6 / m_mult));

	m_bitmap[0] = auto_bitmap_rgb32_alloc(machine(),m_htotal, m_vtotal);
	m_bitmap[1] = auto_bitmap_rgb32_alloc(machine(),m_htotal, m_vtotal);

	rectangle visarea(
			m_hbackporch - m_hfrontporch,
			m_hbackporch - m_hfrontporch + m_hvisible - 1,
			m_vbackporch - m_vfrontporch,
			m_vbackporch - m_vfrontporch + m_vvisible - 1);

	m_clock_period = attotime::from_hz(m_monitor_clock);

	m_refresh = attotime::from_hz(m_monitor_clock) * m_vtotal * m_htotal;
	screen().configure(m_htotal, m_vtotal, visarea, m_refresh.as_attoseconds());
}

void fixedfreq_device::update_screen_parameters(const attotime &refresh)
{
	rectangle visarea(
//          m_hsync - m_hvisible,
//          m_hsync - 1 ,
			m_hbackporch - m_hfrontporch,
			m_hbackporch - m_hfrontporch + m_hvisible - 1,
			m_vbackporch - m_vfrontporch,
			m_vbackporch - m_vfrontporch + m_vvisible - 1);

	m_refresh = refresh;
	screen().configure(m_htotal, m_vtotal, visarea, m_refresh.as_attoseconds());
}

int fixedfreq_device::sync_separator(const attotime &time, double newval)
{
	int last_vsync = m_sig_vsync;
	int last_comp = m_sig_composite;
	int ret = 0;

	m_vint += ((double) last_comp - m_vint) * (1.0 - exp(-time.as_double() * m_mult));
	m_sig_composite = (newval < m_sync_threshold) ? 1 : 0 ;

	m_sig_vsync = (m_vint > m_int_trig) ? 1 : 0;

	if (!last_vsync && m_sig_vsync)
	{
		/* TODO - time since last hsync and field detection */
		ret |= 1;
	}
	if (last_vsync && !m_sig_vsync)
	{
		m_sig_field = last_comp;   /* force false-progressive */
		m_sig_field = (m_sig_field ^ 1) ^ last_comp;   /* if there is no field switch, auto switch */
		VERBOSE_OUT(("Field: %d\n", m_sig_field));
	}
	if (!last_comp && m_sig_composite)
	{
		/* TODO - time since last hsync and field detection */
		ret |= 2;
	}
	if (last_comp && !m_sig_composite)
	{
		/* falling composite */
		ret |= 4;
	}
	return ret;
}

UINT32 fixedfreq_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, *m_bitmap[!m_cur_bm], 0, 0, 0, 0, cliprect);

	return 0;
}

NETDEV_ANALOG_CALLBACK_MEMBER(fixedfreq_device::update_vid)
{
	bitmap_rgb32 *bm = m_bitmap[m_cur_bm];
	const int has_fields = (m_fieldcount > 1) ? 1: 0;

	int pixels = round((time - m_line_time).as_double() / m_clock_period.as_double());
	attotime delta_time = (time - m_last_time);

	if (data == m_vid)
		return;

	ATTR_UNUSED int sync = sync_separator(delta_time, data);

	if (m_last_y < bm->height())
	{
		rgb_t col;

		if (m_vid < m_sync_threshold)
			// Mark sync areas
			col = rgb_t(255, 0, 0);
		else
		{
			int colv = (int) ((m_vid - m_sync_threshold) * m_gain * 255.0);
			if (colv > 255)
				colv = 255;
			col = rgb_t(colv, colv, colv);
		}

		bm->plot_box(m_last_x, m_last_y + m_sig_field * has_fields, pixels - m_last_x, 1, col);
		m_last_x = pixels;
	}
	if (sync & 1)
	{
		VERBOSE_OUT(("VSYNC %d %d\n", pixels, m_last_y + m_sig_field));
	}
	if (sync & 2)
	{
		VERBOSE_OUT(("HSYNC up %d\n", pixels));
		//if (m_last_y == 27) printf("HSYNC up %d %d\n", m_last_y, pixels);
	}
	if (sync & 4)
	{
		VERBOSE_OUT(("HSYNC down %f %d %f\n", time.as_double()* 1e6, pixels, m_vid));
	}

	if (sync & 1)
	{
		m_last_y = m_vbackporch - m_vsync;
		// toggle bitmap
		m_cur_bm ^= 1;
		update_screen_parameters(time - m_last_vsync_time);
		m_last_vsync_time = time;
	}

	// FIXME: pixels > 50 filters some spurious hysnc on line 27 in breakout
	if ((sync & 2) && !m_sig_vsync && (pixels > 100))
	{
		m_last_y += m_fieldcount;
		m_last_x = 0;
		m_line_time = time;
	}

	m_last_time = time;
	m_vid = data;

}


/***************************************************************************/
