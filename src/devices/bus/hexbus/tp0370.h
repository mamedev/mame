// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/****************************************************************************

    Intelligent peripheral bus controller (IBC)

    This is a simple circuit used to connect to the Hexbus infrastructure.

    See tp0370.cpp for documentation

    Michael Zapf

*****************************************************************************/
#ifndef MAME_BUS_HEXBUS_TP0370_H
#define MAME_BUS_HEXBUS_TP0370_H

#pragma once

namespace bus::hexbus {

class ibc_device : public device_t
{
public:
	ibc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	// Callbacks
	auto int_cb() { return m_int.bind(); }
	auto hexbus_cb() { return m_hexout.bind(); }
	auto hsklatch_cb() { return m_latch.bind(); }

	// INT line
	devcb_write_line m_int;

	// Outgoing connection to Hexbus
	devcb_write8 m_hexout;

	// Callback to set the HSK latch
	devcb_write_line m_latch;

	// Incoming connection
	void from_hexbus(uint8_t val);

	// Update the lines from the Hexbus
	void update_lines(bool bav, bool hsk);

private:
	bool m_inhibit;
	bool m_disable;
	bool m_bav;
	bool m_hsk;
	bool m_bavold;
	bool m_hskold;
	bool m_int_pending;
	bool m_incoming_message;
	bool m_message_started;
	bool m_latch_inhibit;

	uint8_t m_data;
	uint8_t m_transmit;

	uint8_t m_last_status;

	void set_disable_inhibit(bool dis, bool inh);
	void set_lines(bool bav, bool hsk);
};

} // namespace bus::hexbus

DECLARE_DEVICE_TYPE_NS(IBC, bus::hexbus, ibc_device)
#endif
