// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    Hudson/NEC HuC6202 interface

**********************************************************************/

#ifndef MAME_VIDEO_HUC6202_H
#define MAME_VIDEO_HUC6202_H

#pragma once


class huc6202_device : public device_t
{
public:
	// construction/destruction
	huc6202_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto next_pixel_0_callback() { return m_next_pixel_0_cb.bind(); }
	auto time_til_next_event_0_callback() { return m_time_til_next_event_0_cb.bind(); }
	auto vsync_changed_0_callback() { return m_vsync_changed_0_cb.bind(); }
	auto hsync_changed_0_callback() { return m_hsync_changed_0_cb.bind(); }
	auto read_0_callback() { return m_read_0_cb.bind(); }
	auto write_0_callback() { return m_write_0_cb.bind(); }
	auto next_pixel_1_callback() { return m_next_pixel_1_cb.bind(); }
	auto time_til_next_event_1_callback() { return m_time_til_next_event_1_cb.bind(); }
	auto vsync_changed_1_callback() { return m_vsync_changed_1_cb.bind(); }
	auto hsync_changed_1_callback() { return m_hsync_changed_1_cb.bind(); }
	auto read_1_callback() { return m_read_1_cb.bind(); }
	auto write_1_callback() { return m_write_1_cb.bind(); }

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);
	u8 io_read(offs_t offset);
	void io_write(offs_t offset, u8 data);
	u16 next_pixel();
	u16 time_until_next_event();
	void vsync_changed(int state);
	void hsync_changed(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

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
