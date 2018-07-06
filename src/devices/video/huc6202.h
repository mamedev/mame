// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    Hudson/NEC HuC6202 interface

**********************************************************************/

#ifndef MAME_VIDEO_HUC6202_H
#define MAME_VIDEO_HUC6202_H

#pragma once


#define MCFG_HUC6202_NEXT_PIXEL_0_CB(_devcb) \
	downcast<huc6202_device &>(*device).set_next_pixel_0_callback(DEVCB_##_devcb);

#define MCFG_HUC6202_TIME_TIL_NEXT_EVENT_0_CB(_devcb) \
	downcast<huc6202_device &>(*device).set_time_til_next_event_0_callback(DEVCB_##_devcb);

#define MCFG_HUC6202_VSYNC_CHANGED_0_CB(_devcb) \
	downcast<huc6202_device &>(*device).set_vsync_changed_0_callback(DEVCB_##_devcb);

#define MCFG_HUC6202_HSYNC_CHANGED_0_CB(_devcb) \
	downcast<huc6202_device &>(*device).set_hsync_changed_0_callback(DEVCB_##_devcb);

#define MCFG_HUC6202_READ_0_CB(_devcb) \
	downcast<huc6202_device &>(*device).set_read_0_callback(DEVCB_##_devcb);

#define MCFG_HUC6202_WRITE_0_CB(_devcb) \
	downcast<huc6202_device &>(*device).set_write_0_callback(DEVCB_##_devcb);

#define MCFG_HUC6202_NEXT_PIXEL_1_CB(_devcb) \
	downcast<huc6202_device &>(*device).set_next_pixel_1_callback(DEVCB_##_devcb);

#define MCFG_HUC6202_TIME_TIL_NEXT_EVENT_1_CB(_devcb) \
	downcast<huc6202_device &>(*device).set_time_til_next_event_1_callback(DEVCB_##_devcb);

#define MCFG_HUC6202_VSYNC_CHANGED_1_CB(_devcb) \
	downcast<huc6202_device &>(*device).set_vsync_changed_1_callback(DEVCB_##_devcb);

#define MCFG_HUC6202_HSYNC_CHANGED_1_CB(_devcb) \
	downcast<huc6202_device &>(*device).set_hsync_changed_1_callback(DEVCB_##_devcb);

#define MCFG_HUC6202_READ_1_CB(_devcb) \
	downcast<huc6202_device &>(*device).set_read_1_callback(DEVCB_##_devcb);

#define MCFG_HUC6202_WRITE_1_CB(_devcb) \
	downcast<huc6202_device &>(*device).set_write_1_callback(DEVCB_##_devcb);


class huc6202_device : public device_t
{
public:
	// construction/destruction
	huc6202_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> devcb_base &set_next_pixel_0_callback(Object &&cb) { return m_next_pixel_0_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_time_til_next_event_0_callback(Object &&cb) { return m_time_til_next_event_0_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_vsync_changed_0_callback(Object &&cb) { return m_vsync_changed_0_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_hsync_changed_0_callback(Object &&cb) { return m_hsync_changed_0_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_read_0_callback(Object &&cb) { return m_read_0_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_write_0_callback(Object &&cb) { return m_write_0_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_next_pixel_1_callback(Object &&cb) { return m_next_pixel_1_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_time_til_next_event_1_callback(Object &&cb) { return m_time_til_next_event_1_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_vsync_changed_1_callback(Object &&cb) { return m_vsync_changed_1_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_hsync_changed_1_callback(Object &&cb) { return m_hsync_changed_1_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_read_1_callback(Object &&cb) { return m_read_1_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_write_1_callback(Object &&cb) { return m_write_1_cb.set_callback(std::forward<Object>(cb)); }

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
		uint8_t   prio_type;
		uint8_t   dev0_enabled;
		uint8_t   dev1_enabled;
	} m_prio[4];
	uint16_t  m_window1;
	uint16_t  m_window2;
	int     m_io_device;
	int     m_map_index;
	int     m_map_dirty;
	uint8_t   m_prio_map[512];

};


DECLARE_DEVICE_TYPE(HUC6202, huc6202_device)

#endif // MAME_VIDEO_HUC6202_H
