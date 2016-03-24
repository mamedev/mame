// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    Hudson/NEC HuC6202 interface

**********************************************************************/

#ifndef __HUC6202_H_
#define __HUC6202_H_

#include "emu.h"


#define MCFG_HUC6202_NEXT_PIXEL_0_CB(_devcb) \
	devcb = &huc6202_device::set_next_pixel_0_callback(*device, DEVCB_##_devcb);

#define MCFG_HUC6202_TIME_TIL_NEXT_EVENT_0_CB(_devcb) \
	devcb = &huc6202_device::set_time_til_next_event_0_callback(*device, DEVCB_##_devcb);

#define MCFG_HUC6202_VSYNC_CHANGED_0_CB(_devcb) \
	devcb = &huc6202_device::set_vsync_changed_0_callback(*device, DEVCB_##_devcb);

#define MCFG_HUC6202_HSYNC_CHANGED_0_CB(_devcb) \
	devcb = &huc6202_device::set_hsync_changed_0_callback(*device, DEVCB_##_devcb);

#define MCFG_HUC6202_READ_0_CB(_devcb) \
	devcb = &huc6202_device::set_read_0_callback(*device, DEVCB_##_devcb);

#define MCFG_HUC6202_WRITE_0_CB(_devcb) \
	devcb = &huc6202_device::set_write_0_callback(*device, DEVCB_##_devcb);

#define MCFG_HUC6202_NEXT_PIXEL_1_CB(_devcb) \
	devcb = &huc6202_device::set_next_pixel_1_callback(*device, DEVCB_##_devcb);

#define MCFG_HUC6202_TIME_TIL_NEXT_EVENT_1_CB(_devcb) \
	devcb = &huc6202_device::set_time_til_next_event_1_callback(*device, DEVCB_##_devcb);

#define MCFG_HUC6202_VSYNC_CHANGED_1_CB(_devcb) \
	devcb = &huc6202_device::set_vsync_changed_1_callback(*device, DEVCB_##_devcb);

#define MCFG_HUC6202_HSYNC_CHANGED_1_CB(_devcb) \
	devcb = &huc6202_device::set_hsync_changed_1_callback(*device, DEVCB_##_devcb);

#define MCFG_HUC6202_READ_1_CB(_devcb) \
	devcb = &huc6202_device::set_read_1_callback(*device, DEVCB_##_devcb);

#define MCFG_HUC6202_WRITE_1_CB(_devcb) \
	devcb = &huc6202_device::set_write_1_callback(*device, DEVCB_##_devcb);


class huc6202_device : public device_t
{
public:
	// construction/destruction
	huc6202_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_next_pixel_0_callback(device_t &device, _Object object) { return downcast<huc6202_device &>(device).m_next_pixel_0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_time_til_next_event_0_callback(device_t &device, _Object object) { return downcast<huc6202_device &>(device).m_time_til_next_event_0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_vsync_changed_0_callback(device_t &device, _Object object) { return downcast<huc6202_device &>(device).m_vsync_changed_0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_hsync_changed_0_callback(device_t &device, _Object object) { return downcast<huc6202_device &>(device).m_hsync_changed_0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_read_0_callback(device_t &device, _Object object) { return downcast<huc6202_device &>(device).m_read_0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_write_0_callback(device_t &device, _Object object) { return downcast<huc6202_device &>(device).m_write_0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_next_pixel_1_callback(device_t &device, _Object object) { return downcast<huc6202_device &>(device).m_next_pixel_1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_time_til_next_event_1_callback(device_t &device, _Object object) { return downcast<huc6202_device &>(device).m_time_til_next_event_1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_vsync_changed_1_callback(device_t &device, _Object object) { return downcast<huc6202_device &>(device).m_vsync_changed_1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_hsync_changed_1_callback(device_t &device, _Object object) { return downcast<huc6202_device &>(device).m_hsync_changed_1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_read_1_callback(device_t &device, _Object object) { return downcast<huc6202_device &>(device).m_read_1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_write_1_callback(device_t &device, _Object object) { return downcast<huc6202_device &>(device).m_write_1_cb.set_callback(object); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( io_read );
	DECLARE_WRITE8_MEMBER( io_write );
	DECLARE_READ16_MEMBER( next_pixel );
	DECLARE_READ16_MEMBER( time_until_next_event );
	DECLARE_WRITE_LINE_MEMBER( vsync_changed );
	DECLARE_WRITE_LINE_MEMBER( hsync_changed );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	/* callbacks */
	/* First gfx input device */
	devcb_read16                m_next_pixel_0_cb;

	/* TODO: Choose proper types */
	/* Callback function to get time until next event */
	devcb_read16                m_time_til_next_event_0_cb;

	devcb_write_line            m_vsync_changed_0_cb;
	devcb_write_line            m_hsync_changed_0_cb;
	devcb_read8                 m_read_0_cb;
	devcb_write8                m_write_0_cb;


	/* Second gfx input device */
	devcb_read16                m_next_pixel_1_cb;

	/* TODO: Choose proper types */
	/* Callback function to get time until next event */
	devcb_read16                m_time_til_next_event_1_cb;

	devcb_write_line            m_vsync_changed_1_cb;
	devcb_write_line            m_hsync_changed_1_cb;
	devcb_read8                 m_read_1_cb;
	devcb_write8                m_write_1_cb;

	struct {
		UINT8   prio_type;
		UINT8   dev0_enabled;
		UINT8   dev1_enabled;
	} m_prio[4];
	UINT16  m_window1;
	UINT16  m_window2;
	int     m_io_device;
	int     m_map_index;
	int     m_map_dirty;
	UINT8   m_prio_map[512];

};


extern const device_type HUC6202;


#endif
