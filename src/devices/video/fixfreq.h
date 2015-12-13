// license:BSD-3-Clause
// copyright-holders:Couriersud
/***************************************************************************

    fixfreq.h

    Fixed frequency monochrome monitor emulation

    The driver is intended for drivers which provide an analog video signal.
    VSYNC and HSYNC levels are used to create the bitmap.

***************************************************************************/

#ifndef FIXFREQ_H
#define FIXFREQ_H

#include "emu.h"
#include "machine/netlist.h"

#define FIXFREQ_INTERFACE(name) \
	const fixedfreq_interface (name) =

#define MCFG_FIXFREQ_ADD(_tag, _screen_tag) \
	MCFG_SCREEN_ADD(_screen_tag, RASTER) \
	MCFG_SCREEN_RAW_PARAMS(13500000, 858, 0, 858, 525, 0, 525) \
	MCFG_SCREEN_UPDATE_DEVICE(_tag, fixedfreq_device, screen_update) \
	MCFG_DEVICE_ADD(_tag, FIXFREQ, 0) \
	MCFG_VIDEO_SET_SCREEN(_screen_tag)

#define MCFG_FIXFREQ_MONITOR_CLOCK(_clock) \
	fixedfreq_device::set_minitor_clock(*device, _clock);

#define MCFG_FIXFREQ_HORZ_PARAMS(_visible, _frontporch, _sync, _backporch) \
	fixedfreq_device::set_horz_params(*device, _visible, _frontporch, _sync, _backporch);

#define MCFG_FIXFREQ_VERT_PARAMS(_visible, _frontporch, _sync, _backporch) \
	fixedfreq_device::set_vert_params(*device, _visible, _frontporch, _sync, _backporch);

#define MCFG_FIXFREQ_FIELDCOUNT(_count) \
	fixedfreq_device::set_fieldcount(*device, _count);

#define MCFG_FIXFREQ_SYNC_THRESHOLD(_threshold) \
	fixedfreq_device::set_threshold(*device, _threshold);

#define MCFG_FIXFREQ_GAIN(_gain) \
	fixedfreq_device::set_gain(*device, _gain);

// pre-defined configurations

//ModeLine "720x480@30i" 13.5 720 736 799 858 480 486 492 525 interlace -hsync -vsync
#define MCFG_FIXFREQ_MODE_NTSC720 \
	MCFG_FIXFREQ_MONITOR_CLOCK(13500000) \
	MCFG_FIXFREQ_HORZ_PARAMS(720, 736, 799, 858) \
	MCFG_FIXFREQ_VERT_PARAMS(480, 486, 492, 525) \
	MCFG_FIXFREQ_FIELDCOUNT(2) \
	MCFG_FIXFREQ_SYNC_THRESHOLD(0.3)

//ModeLine "704x480@30i" 13.5 704 728 791 858 480 486 492 525
#define MCFG_FIXFREQ_MODE_NTSC704 \
	MCFG_FIXFREQ_MONITOR_CLOCK(13500000) \
	MCFG_FIXFREQ_HORZ_PARAMS(704, 728, 791, 858) \
	MCFG_FIXFREQ_VERT_PARAMS(480, 486, 492, 525) \
	MCFG_FIXFREQ_FIELDCOUNT(2) \
	MCFG_FIXFREQ_SYNC_THRESHOLD(0.3)


// ======================> vga_device

class fixedfreq_device :  public device_t,
							public device_video_interface
{
public:
	// construction/destruction
	fixedfreq_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	fixedfreq_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	// inline configuration helpers
	static void set_minitor_clock(device_t &device, UINT32 clock) { downcast<fixedfreq_device &>(device).m_monitor_clock = clock; }
	static void set_fieldcount(device_t &device, int count) { downcast<fixedfreq_device &>(device).m_fieldcount = count; }
	static void set_threshold(device_t &device, double threshold) { downcast<fixedfreq_device &>(device).m_sync_threshold = threshold; }
	static void set_gain(device_t &device, double gain) { downcast<fixedfreq_device &>(device).m_gain = gain; }
	static void set_horz_params(device_t &device, int visible, int frontporch, int sync, int backporch)
	{
		fixedfreq_device &dev = downcast<fixedfreq_device &>(device);
		dev.m_hvisible = visible;
		dev.m_hfrontporch = frontporch;
		dev.m_hsync = sync;
		dev.m_hbackporch = backporch;
	}
	static void set_vert_params(device_t &device, int visible, int frontporch, int sync, int backporch)
	{
		fixedfreq_device &dev = downcast<fixedfreq_device &>(device);
		dev.m_vvisible = visible;
		dev.m_vfrontporch = frontporch;
		dev.m_vsync = sync;
		dev.m_vbackporch = backporch;
	}

	virtual UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	NETDEV_ANALOG_CALLBACK_MEMBER(update_vid);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;
	//virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	void recompute_parameters(bool postload);
	void update_screen_parameters(const attotime &refresh);

private:

	int sync_separator(const attotime &time, double newval);

	int m_htotal;
	int m_vtotal;

	double m_vid;
	int m_last_x;
	int m_last_y;
	attotime m_last_time;
	attotime m_line_time;
	attotime m_last_hsync_time;
	attotime m_last_vsync_time;
	attotime m_refresh;
	attotime  m_clock_period;
	bitmap_rgb32 *m_bitmap[2];
	int m_cur_bm;

	/* adjustable by drivers */
	UINT32 m_monitor_clock;
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
	double m_vint;
	double m_int_trig;
	double m_mult;

	int m_sig_vsync;
	int m_sig_composite;
	int m_sig_field;
};


// device type definition
extern const device_type FIXFREQ;

#endif /* FIXFREQ_H */
