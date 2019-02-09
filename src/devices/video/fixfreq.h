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


// ======================> fixedfreq_device

class fixedfreq_device : public device_t, public device_video_interface
{
public:
	// construction/destruction
	fixedfreq_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// inline configuration helpers
	void set_monitor_clock(uint32_t clock) { m_monitor_clock = clock; }
	void set_fieldcount(int count) { m_fieldcount = count; }
	void set_threshold(double threshold) { m_sync_threshold = threshold; }
	void set_gain(double gain) { m_gain = gain; }
	void set_horz_params(int visible, int frontporch, int sync, int backporch)
	{
		m_hvisible = visible;
		m_hfrontporch = frontporch;
		m_hsync = sync;
		m_hbackporch = backporch;
	}
	void set_vert_params(int visible, int frontporch, int sync, int backporch)
	{
		m_vvisible = visible;
		m_vfrontporch = frontporch;
		m_vsync = sync;
		m_vbackporch = backporch;
	}

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

	typedef double time_type;

	fixedfreq_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;
	//virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	void update_screen_parameters(const time_type &refresh);

private:

	void update_sync_channel(const time_type &time, const double newval);
	void update_bm(const time_type &time);
	void recompute_parameters();

	int m_htotal;
	int m_vtotal;
	int m_hscale;

	double m_sync_signal;
	rgb_t m_col;
	int m_last_x;
	int m_last_y;
	time_type m_last_sync_time;
	time_type m_line_time;
	time_type m_last_hsync_time;
	time_type m_last_vsync_time;
	time_type m_refresh_period;
	time_type m_clock_period;
	std::unique_ptr<bitmap_rgb32> m_bitmap[2];
	int m_cur_bm;

	/* adjustable by drivers */
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

	/* sync separator */
	double m_vsync_filter;
	double m_vsync_threshold;
	double m_vsync_filter_timeconst;

	int m_sig_vsync;
	int m_sig_composite;
	int m_sig_field;
};


// device type definition
DECLARE_DEVICE_TYPE(FIXFREQ, fixedfreq_device)

#endif // MAME_VIDEO_FIXFREQ_H
