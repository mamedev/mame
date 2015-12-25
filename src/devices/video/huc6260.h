// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    Hudson/NEC HuC6260 interface and definitions

**********************************************************************/


#ifndef __HUC6260_H_
#define __HUC6260_H_

#include "emu.h"


#define HUC6260_PALETTE_SIZE    1024

/* Screen timing stuff */
#define HUC6260_WPF         1365    /* width of a line in frame including blanking areas */
#define HUC6260_LPF         263     /* max number of lines in a single frame */


#define MCFG_HUC6260_NEXT_PIXEL_DATA_CB(_devcb) \
	devcb = &huc6260_device::set_next_pixel_data_callback(*device, DEVCB_##_devcb);

#define MCFG_HUC6260_TIME_TIL_NEXT_EVENT_CB(_devcb) \
	devcb = &huc6260_device::set_time_til_next_event_callback(*device, DEVCB_##_devcb);

#define MCFG_HUC6260_VSYNC_CHANGED_CB(_devcb) \
	devcb = &huc6260_device::set_vsync_changed_callback(*device, DEVCB_##_devcb);

#define MCFG_HUC6260_HSYNC_CHANGED_CB(_devcb) \
	devcb = &huc6260_device::set_hsync_changed_callback(*device, DEVCB_##_devcb);


class huc6260_device :  public device_t,
						public device_video_interface
{
public:
	// construction/destruction
	huc6260_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_next_pixel_data_callback(device_t &device, _Object object) { return downcast<huc6260_device &>(device).m_next_pixel_data_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_time_til_next_event_callback(device_t &device, _Object object) { return downcast<huc6260_device &>(device).m_time_til_next_event_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_vsync_changed_callback(device_t &device, _Object object) { return downcast<huc6260_device &>(device).m_vsync_changed_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_hsync_changed_callback(device_t &device, _Object object) { return downcast<huc6260_device &>(device).m_hsync_changed_cb.set_callback(object); }

	void video_update(bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_PALETTE_INIT(huc6260);

	READ8_MEMBER(palette_direct_read);
	WRITE8_MEMBER(palette_direct_write);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	int     m_last_h;
	int     m_last_v;
	int     m_height;

	/* callbacks */
	/* Callback function to retrieve pixel data */
	devcb_read16                    m_next_pixel_data_cb;

	/* TODO: Choose proper types */
	/* Callback function to get time until next event */
	devcb_read16                    m_time_til_next_event_cb;

	/* Callback function which gets called when vsync changes */
	devcb_write_line                m_vsync_changed_cb;

	/* Callback function which gets called when hsync changes */
	devcb_write_line                m_hsync_changed_cb;

	UINT16  m_palette[512];
	UINT16  m_address;
	UINT8   m_greyscales;       /* Should the HuC6260 output grey or color graphics */
	UINT8   m_blur;             /* Should the edges of graphics be blurred/Select screen height 0=262, 1=263 */
	UINT8   m_pixels_per_clock; /* Number of pixels to output per colour clock */
	UINT16  m_pixel_data;
	UINT8   m_pixel_clock;

	emu_timer   *m_timer;
	std::unique_ptr<bitmap_ind16>   m_bmp;
};


extern const device_type HUC6260;


#endif
