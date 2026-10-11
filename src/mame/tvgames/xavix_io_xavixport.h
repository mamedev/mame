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
	template <unsigned Sensor> auto sensor_callback() { static_assert(Sensor < 2); return m_sensor_cb[Sensor].bind(); }
	auto power_callback() { return m_power_cb.bind(); }
	auto hub_callback() { return m_hub_cb.bind(); }
	auto event_callback() { return m_event_cb.bind(); }
	void set_report_type(u8 report_type) { m_report_type = report_type; }
	void set_receiver_present(bool present) { m_receiver_present = present; }

	u8 pio_r(offs_t offset, u8 mem_mask);
	void pio_w(offs_t offset, u8 data, u8 mem_mask);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	TIMER_CALLBACK_MEMBER(event_timer);
	void latch_report();
	void latch_hub_report();
	void hub_command_bit_w(int state);

	devcb_read8 m_panel_cb;
	devcb_read8::array<2> m_sensor_cb;
	devcb_read8 m_power_cb;
	devcb_read8 m_hub_cb;
	devcb_write_line m_event_cb;
	emu_timer *m_event_timer;

	std::array<u8, 5> m_report;
	std::array<u8, 5> m_hub_report;
	std::array<u8, 3> m_hub_command;
	u8 m_report_type;
	bool m_receiver_present;
	u8 m_bit;
	u8 m_hub_bit;
	u8 m_hub_command_bits;
	u8 m_hub_report_bytes;
	bool m_selected;
	bool m_clock;
	bool m_request;
	bool m_request_output;
	bool m_hub_clock;
	bool m_host_transmit;
	bool m_hub_active;
	bool m_hub_transfer;
};

DECLARE_DEVICE_TYPE(XAVIXPORT_IO, xavixport_io_device)

#endif // MAME_TVGAMES_XAVIX_IO_XAVIXPORT_H
