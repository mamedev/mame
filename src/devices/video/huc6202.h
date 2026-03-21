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
	huc6202_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto next_pixel_0_callback() { return m_next_pixel_cb[0].bind(); }
	auto time_til_next_event_0_callback() { return m_time_til_next_event_cb[0].bind(); }
	auto read_0_callback() { return m_read_cb[0].bind(); }
	auto write_0_callback() { return m_write_cb[0].bind(); }
	auto next_pixel_1_callback() { return m_next_pixel_cb[1].bind(); }
	auto time_til_next_event_1_callback() { return m_time_til_next_event_cb[1].bind(); }
	auto read_1_callback() { return m_read_cb[1].bind(); }
	auto write_1_callback() { return m_write_cb[1].bind(); }

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);
	u8 io_read(offs_t offset);
	void io_write(offs_t offset, u8 data);
	u16 next_pixel();
	u16 time_until_next_event();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	/* callbacks */
	/* gfx input device */
	devcb_read16::array<2>                m_next_pixel_cb;

	/* TODO: Choose proper types */
	/* Callback function to get time until next event */
	devcb_read16::array<2>                m_time_til_next_event_cb;

	devcb_read8::array<2>                 m_read_cb;
	devcb_write8::array<2>                m_write_cb;

	struct {
		u8   prio_type;
		bool dev0_enabled;
		bool dev1_enabled;
	} m_prio[4];
	u16  m_window[2];
	bool m_io_device;
	u32  m_map_index;
	bool m_map_dirty;
	u8   m_prio_map[512];

};


DECLARE_DEVICE_TYPE(HUC6202, huc6202_device)

#endif // MAME_VIDEO_HUC6202_H
