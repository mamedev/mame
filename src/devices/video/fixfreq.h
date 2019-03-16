// license:BSD-3-Clause
// copyright-holders:Couriersud
/***************************************************************************

    fixfreq.h

    Fixed frequency monochrome monitor emulation

    The driver is intended for drivers which provide an analog video signal.
    VSYNC and HSYNC levels are used to create the bitmap.

***************************************************************************/

#ifndef MAME_VIDEO_FIXFREQ_H
#define MAME_VIDEO_FIXFREQ_H

#include "machine/netlist.h"
#include "screen.h"

struct fixedfreq_monitor_desc
{
	fixedfreq_monitor_desc()
	// default to NTSC "704x480@30i"
	: m_monitor_clock(13500000),
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
	m_hscale(1)
	{}

	int minh() const { return (m_hbackporch - m_hfrontporch) * m_hscale; }
	int maxh() const { return (m_hbackporch - m_hfrontporch + m_hvisible) * m_hscale - 1; }
	int minv() const { return m_vbackporch - m_vfrontporch; }
	int maxv() const { return m_vbackporch - m_vfrontporch + m_vvisible - 1; }

	uint32_t m_monitor_clock;
	int m_hvisible;
	int m_hfrontporch;
	int m_hsync;
	int m_hbackporch;
	int m_vvisible;
	int m_vfrontporch;
	int m_vsync;
	int m_vbackporch;
	int m_fieldcount;
	double m_sync_threshold;
	double m_gain;
	int m_hscale;
};

struct fixedfreq_monitor_intf
{
	virtual ~fixedfreq_monitor_intf() = default;
	virtual void vsync_start_cb(double refresh_time) = 0;
	virtual void plot_hline(int x, int y, int w, uint32_t col) = 0;
};

struct fixedfreq_monitor_state
{
	using time_type = double;

	fixedfreq_monitor_state(fixedfreq_monitor_desc &desc, fixedfreq_monitor_intf &intf)
	: m_desc(desc),
	m_intf(intf),
	m_sync_signal(0),
	m_col(0),
	m_last_x(0),
	m_last_y(0),
	m_last_sync_time(time_type(0)),
	m_line_time(time_type(0)),
	m_last_hsync_time(time_type(0)),
	m_last_vsync_time(time_type(0)),
	m_clock_period(time_type(0)),
	m_vsync_filter(0),
	m_vsync_threshold(0),
	m_vsync_filter_timeconst(0),
	m_sig_vsync(0),
	m_sig_composite(0),
	m_sig_field(0)
	{}

	/***
	 * \brief To be called after monitor parameters are set
	 */
	void start()
	{
		// FIXME: once moved to netlist this may no longer be necessary.
		//        Only copies constructor init

		m_sync_signal = 0.0;
		m_col = rgb_t(0,0,0);
		m_last_x = 0;
		m_last_y = 0;
		m_last_sync_time = time_type(0);
		m_line_time = time_type(0);
		m_last_hsync_time = time_type(0);
		m_last_vsync_time = time_type(0);
		m_clock_period = time_type(0);

		/* sync separator */
		m_vsync_filter = 0.0;
		m_vsync_threshold = 0.0;
		m_vsync_filter_timeconst = 0.0;

		m_sig_vsync = 0;
		m_sig_composite = 0;
		m_sig_field = 0;

		// htotal = m_desc.m_hbackporch;
		// vtotal = m_desc.m_vbackporch;

		/* sync separator */

		m_vsync_threshold = (exp(- 3.0/(3.0+3.0))) - exp(-1.0);
		m_vsync_filter_timeconst = (double) (m_desc.m_monitor_clock) / (double) m_desc.m_hbackporch * 1.0; // / (3.0 + 3.0);
		//LOG("trigger %f with len %f\n", m_vsync_threshold, 1e6 / m_vsync_filter_timeconst);

		m_clock_period = 1.0 / m_desc.m_monitor_clock;
		m_intf.vsync_start_cb(m_clock_period * m_desc.m_vbackporch * m_desc.m_hbackporch);

	}

	void reset()
	{
		m_last_sync_time = time_type(0);
		m_line_time = time_type(0);
		m_last_hsync_time = time_type(0);
		m_last_vsync_time = time_type(0);
		m_vsync_filter = 0;
	}

	void update_sync_channel(const time_type &time, const double newval);
	void update_bm(const time_type &time);
	void update_composite_monochrome(const time_type &time, const double newval);
	void update_red(const time_type &time, const double data);
	void update_green(const time_type &time, const double data);
	void update_blue(const time_type &time, const double data);
	void update_sync(const time_type &time, const double data);

	const fixedfreq_monitor_desc &m_desc;
	fixedfreq_monitor_intf &m_intf;

	double m_sync_signal;
	uint32_t m_col;
	int m_last_x;
	int m_last_y;
	time_type m_last_sync_time;
	time_type m_line_time;
	time_type m_last_hsync_time;
	time_type m_last_vsync_time;
	time_type m_clock_period;

	/* sync separator */
	double m_vsync_filter;
	double m_vsync_threshold;
	double m_vsync_filter_timeconst;

	int m_sig_vsync;
	int m_sig_composite;
	int m_sig_field;

};

// ======================> fixedfreq_device

class fixedfreq_device : public device_t, public device_video_interface,
						 public fixedfreq_monitor_intf
{
public:

	using time_type = fixedfreq_monitor_state::time_type;

	// construction/destruction
	fixedfreq_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// inline configuration helpers
	void set_monitor_clock(uint32_t clock) { m_monitor.m_monitor_clock = clock; }
	void set_fieldcount(int count) { m_monitor.m_fieldcount = count; }
	void set_threshold(double threshold) { m_monitor.m_sync_threshold = threshold; }
	void set_gain(double gain) { m_monitor.m_gain = gain; }
	void set_horz_params(int visible, int frontporch, int sync, int backporch)
	{
		m_monitor.m_hvisible = visible;
		m_monitor.m_hfrontporch = frontporch;
		m_monitor.m_hsync = sync;
		m_monitor.m_hbackporch = backporch;
	}
	void set_vert_params(int visible, int frontporch, int sync, int backporch)
	{
		m_monitor.m_vvisible = visible;
		m_monitor.m_vfrontporch = frontporch;
		m_monitor.m_vsync = sync;
		m_monitor.m_vbackporch = backporch;
	}
	void set_horz_scale(int hscale) { m_monitor.m_hscale = hscale; }

	// pre-defined configurations
	void set_mode_ntsc720() //ModeLine "720x480@30i" 13.5 720 736 799 858 480 486 492 525 interlace -hsync -vsync
	{
		set_monitor_clock(13500000);
		set_horz_params(720, 736, 799, 858);
		set_vert_params(480, 486, 492, 525);
		set_fieldcount(2);
		set_threshold(0.3);
	}
	void set_mode_ntsc704() //ModeLine "704x480@30i" 13.5 704 728 791 858 480 486 492 525
	{
		set_monitor_clock(13500000);
		set_horz_params(704, 728, 791, 858);
		set_vert_params(480, 486, 492, 525);
		set_fieldcount(2);
		set_threshold(0.3);
	}

	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	NETDEV_ANALOG_CALLBACK_MEMBER(update_composite_monochrome);
	NETDEV_ANALOG_CALLBACK_MEMBER(update_red);
	NETDEV_ANALOG_CALLBACK_MEMBER(update_green);
	NETDEV_ANALOG_CALLBACK_MEMBER(update_blue);
	NETDEV_ANALOG_CALLBACK_MEMBER(update_sync);


protected:

	fixedfreq_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;
	//virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	void vsync_start_cb(double refresh_time) override;
	void plot_hline(int x, int y, int w, uint32_t col) override;

private:

	std::unique_ptr<bitmap_rgb32> m_bitmap[2];
	int m_cur_bm;
	int m_htotal;
	int m_vtotal;

	time_type m_refresh_period;

	/* adjustable by drivers */
	fixedfreq_monitor_desc m_monitor;
	fixedfreq_monitor_state m_state;

};


// device type definition
DECLARE_DEVICE_TYPE(FIXFREQ, fixedfreq_device)

#endif // MAME_VIDEO_FIXFREQ_H
