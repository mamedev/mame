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

#define MCFG_FIXFREQ_ADD(_tag, _screen_tag, _config) \
	MCFG_SCREEN_ADD(_screen_tag, RASTER) \
	MCFG_SCREEN_RAW_PARAMS(13500000, 858, 0, 858, 525, 0, 525) \
	MCFG_SCREEN_UPDATE_DEVICE(_tag, fixedfreq_device, screen_update) \
	MCFG_DEVICE_ADD(_tag, FIXFREQ, 0) \
	MCFG_VIDEO_SET_SCREEN(_screen_tag) \
	MCFG_DEVICE_CONFIG(_config)

struct fixedfreq_interface {
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
};

extern fixedfreq_interface fixedfreq_mode_ntsc704;
extern fixedfreq_interface fixedfreq_mode_ntsc720;

// ======================> vga_device

class fixedfreq_device :  public device_t,
							public device_video_interface,
							public fixedfreq_interface
{
public:
	// construction/destruction
	fixedfreq_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	fixedfreq_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);


	virtual UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	NETDEV_ANALOG_CALLBACK_MEMBER(update_vid);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	virtual void device_post_load();
	//virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	void recompute_parameters(bool postload);
	void update_screen_parameters(attotime refresh);

private:

	int sync_separator(attotime time, double newval);

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

	/* sync separator */
	double m_vint;
	double m_int_trig;
	double m_mult;

	int m_sig_vsync;
	int m_sig_composite;
	int m_sig_field;

protected:

};


// device type definition
extern const device_type FIXFREQ;

#endif /* FIXFREQ_H */
