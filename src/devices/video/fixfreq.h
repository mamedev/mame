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
	m_fieldcount(2),
	m_sync_threshold(0.3),
	m_gain(1.0 / 3.7),
	m_hscale(1),
	m_vsync_threshold(0.600), // trigger at 91% of vsync length 1-exp(-0.6)
	m_hvisible(704),
	m_hfrontporch(728),
	m_hsync(791),
	m_hbackporch(858),
	m_vvisible(480),
	m_vfrontporch(486),
	m_vsync(492),
	m_vbackporch(525)
	{}

	uint32_t monitor_clock() const noexcept { return m_monitor_clock; }
	double clock_period() const noexcept { return 1.0 / (double) m_monitor_clock; }

	int minh() const noexcept { return (m_hbackporch - m_hsync) * m_hscale; }
	int maxh() const noexcept { return (m_hbackporch - m_hsync + m_hvisible) * m_hscale - 1; }
	int minv() const noexcept { return m_vbackporch - m_vsync; }
	int maxv() const noexcept { return m_vbackporch - m_vsync + m_vvisible - 1; }

	int htotal_scaled() const noexcept { return m_hbackporch * m_hscale; }

	int vbackporch_width() const noexcept { return m_vbackporch - m_vsync; }
	int vsync_width() const noexcept { return m_vsync - m_vfrontporch; }
	int vfrontporch_width() const noexcept { return m_vfrontporch - m_vvisible; }
	int vvisible_width() const noexcept { return m_vvisible; }
	int vtotal() const noexcept { return m_vbackporch; }

	int hbackporch_width() const noexcept { return m_hbackporch - m_hsync; }
	int hsync_width() const noexcept { return m_hsync - m_hfrontporch; }
	int hfrontporch_width() const noexcept { return m_hfrontporch - m_hvisible; }
	int hvisible_width() const noexcept { return m_hvisible; }
	int htotal() const noexcept { return m_hbackporch; }

	void set_h_rel(int vw, int fpw, int sw, int bpw)
	{
		m_hvisible = vw;
		m_hfrontporch = m_hvisible + fpw;
		m_hsync = m_hfrontporch + sw;
		m_hbackporch = m_hsync + bpw;
	}

	void set_v_rel(int vw, int fpw, int sw, int bpw)
	{
		m_vvisible = vw;
		m_vfrontporch = m_vvisible + fpw;
		m_vsync = m_vfrontporch + sw;
		m_vbackporch = m_vsync + bpw;
	}

	double vsync_filter_timeconst() const noexcept
	{
		return (double) (m_monitor_clock) / ((double) m_hbackporch * vsync_width());
	}

	double hsync_filter_timeconst() const noexcept
	{
		return (double) m_monitor_clock /  (double) hsync_width();
	}

	uint32_t m_monitor_clock;
	int m_fieldcount;
	double m_sync_threshold;
	double m_gain;
	int m_hscale;
	double m_vsync_threshold;
private:
	int m_hvisible;
	int m_hfrontporch;
	int m_hsync;
	int m_hbackporch;
	int m_vvisible;
	int m_vfrontporch;
	int m_vsync;
	int m_vbackporch;
};

struct fixedfreq_monitor_intf
{
	virtual ~fixedfreq_monitor_intf() = default;
	virtual void vsync_end_cb(double refresh_time) = 0;
};

struct fixedfreq_monitor_line
{
	float y;
	float x;
	float xr;
	uint32_t col;
};

struct fixedfreq_monitor_state
{
	using time_type = double;

	fixedfreq_monitor_state(fixedfreq_monitor_desc &desc, fixedfreq_monitor_intf &intf)
	: m_desc(desc),
	m_intf(intf),
	m_last_sync_val(0),
	m_col(0),
	m_last_x(0),
	m_last_y(0),
	m_last_sync_time(time_type(0)),
	m_line_time(time_type(0)),
	m_last_hsync_time(time_type(0)),
	m_last_vsync_time(time_type(0)),
	m_last_line_duration(time_type(0)),
	m_last_field_time(time_type(0)),
	m_vsync_filter(0),
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

		m_last_sync_val = 0.0;
		m_col = rgb_t(0,0,0);
		m_last_x = 0;
		m_last_y = 0;
		m_last_sync_time = time_type(0);
		m_line_time = time_type(0);
		m_last_hsync_time = time_type(0);
		m_last_vsync_time = time_type(0);

		/* sync separator */
		m_vsync_filter = 0.0;

		m_sig_vsync = 0;
		m_sig_composite = 0;
		m_sig_field = 0;

		// htotal = m_desc.m_hbackporch;
		// vtotal = m_desc.m_vbackporch;

		/* sync separator */

		//m_vsync_threshold = (exp(- 3.0/(3.0+3.0))) - exp(-1.0);
		//printf("trigger %f with len %f\n", m_vsync_threshold, 1e6 / m_vsync_filter_timeconst);
		// Minimum frame period to be passed to video system ?

		m_fragments.clear();

		m_intf.vsync_end_cb(m_desc.clock_period() * m_desc.vtotal() * m_desc.htotal());
	}

	void reset()
	{
		m_last_sync_val = 0;
		m_col = 0;
		m_last_x = 0;
		m_last_y = 0;
		m_vsync_filter = 0;
		m_sig_vsync = 0;
		m_sig_composite = 0;
		m_sig_field = 0;
		m_fragments.clear();
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

	double m_last_sync_val;
	uint32_t m_col;
	float m_last_x;
	int m_last_y;
	time_type m_last_sync_time;
	time_type m_line_time;
	time_type m_last_hsync_time;
	time_type m_last_vsync_time;

	time_type m_last_line_duration;
	time_type m_last_field_time;

	/* sync separator */
	double m_vsync_filter;

	int m_sig_vsync;
	int m_sig_composite;
	int m_sig_field;
	std::vector<fixedfreq_monitor_line> m_fragments;
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
	fixedfreq_device &set_monitor_clock(uint32_t clock) { m_monitor.m_monitor_clock = clock; return *this;}
	fixedfreq_device &set_fieldcount(int count) { m_monitor.m_fieldcount = count; return *this; }
	fixedfreq_device &set_threshold(double threshold) { m_monitor.m_sync_threshold = threshold; return *this; }
	fixedfreq_device &set_vsync_threshold(double threshold) { m_monitor.m_vsync_threshold = threshold; return *this; }
	fixedfreq_device &set_gain(double gain) { m_monitor.m_gain = gain; return *this; }
	fixedfreq_device &set_horz_params(int visible, int frontporch, int sync, int backporch)
	{
		m_monitor.set_h_rel(
			visible,
			frontporch - visible,
			sync - frontporch,
			backporch - sync);
		return *this;
	}
	fixedfreq_device &set_vert_params(int visible, int frontporch, int sync, int backporch)
	{
		m_monitor.set_v_rel(
			visible,
			frontporch - visible,
			sync - frontporch,
			backporch - sync);
		return *this;
	}
	fixedfreq_device &set_horz_scale(int hscale) { m_monitor.m_hscale = hscale;  return *this;}

	// pre-defined configurations
	fixedfreq_device &set_mode_ntsc720() //ModeLine "720x480@30i" 13.5 720 736 799 858 480 486 492 525 interlace -hsync -vsync
	{
		set_monitor_clock(13500000);
		set_horz_params(720, 736, 799, 858);
		set_vert_params(480, 486, 492, 525);
		set_fieldcount(2);
		set_threshold(0.3);
		return *this;
	}
	fixedfreq_device &set_mode_ntsc704() //ModeLine "704x480@30i" 13.5 704 728 791 858 480 486 492 525
	{
		set_monitor_clock(13500000);
		set_horz_params(704, 728, 791, 858);
		set_vert_params(480, 486, 492, 525);
		set_fieldcount(2);
		set_threshold(0.3);
		return *this;
	}

	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	NETDEV_ANALOG_CALLBACK_MEMBER(update_composite_monochrome);
	NETDEV_ANALOG_CALLBACK_MEMBER(update_red);
	NETDEV_ANALOG_CALLBACK_MEMBER(update_green);
	NETDEV_ANALOG_CALLBACK_MEMBER(update_blue);
	NETDEV_ANALOG_CALLBACK_MEMBER(update_sync);

	INPUT_CHANGED_MEMBER(port_changed);

	unsigned monitor_val(unsigned param) const;

protected:

	fixedfreq_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;
	//virtual void device_timer(emu_timer &timer, device_timer_id id, int param);

	virtual ioport_constructor device_input_ports() const override;

	void vsync_end_cb(double refresh_time) override;

private:
	required_ioport m_enable;
	required_ioport m_vector;
	float m_scanline_height;

	/* adjustable by drivers */
	fixedfreq_monitor_desc m_monitor;
	fixedfreq_monitor_state m_state;

};


// device type definition
DECLARE_DEVICE_TYPE(FIXFREQ, fixedfreq_device)

#endif // MAME_VIDEO_FIXFREQ_H
