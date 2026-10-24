// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_XAVIXPORT_H
#define MAME_TVGAMES_XAVIX_IO_XAVIXPORT_H

#pragma once

#include <array>

class xavixport_io_device : public device_t
{
public:
	xavixport_io_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	auto panel_callback() { return m_panel_cb.bind(); }
	// Decoded accessory bytes 1 and 2, in the low and high byte respectively.
	auto accessory_callback() { return m_accessory_cb.bind(); }
	void set_receiver_present(bool present) { m_receiver_present = present; }

	u8 pio_r(offs_t offset, u8 mem_mask);
	void pio_w(offs_t offset, u8 data, u8 mem_mask);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void latch_report();

	devcb_read8 m_panel_cb;
	devcb_read16 m_accessory_cb;

	std::array<u8, 5> m_report;
	bool m_receiver_present;
	u8 m_bit;
	bool m_selected;
	bool m_clock;
};

DECLARE_DEVICE_TYPE(XAVIXPORT_IO, xavixport_io_device)

#endif // MAME_TVGAMES_XAVIX_IO_XAVIXPORT_H
