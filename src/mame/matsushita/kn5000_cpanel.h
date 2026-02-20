// license:GPL2+
// copyright-holders:Felipe Sanches
/***************************************************************************

	KN5000 control panel

***************************************************************************/

#ifndef MAME_MATSUSHITA_KN5000_CPANEL_H
#define MAME_MATSUSHITA_KN5000_CPANEL_H

#pragma once

class kn5000_cpanel_device :
	public device_t
{
public:
	kn5000_cpanel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto sck_out() { return m_sck_out_cb.bind(); }
	void sck_in(int state);
	void serial_in(int state);
	auto serial_out() { return m_serial_out_cb.bind(); }


protected:
	// device_t
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	devcb_write_line m_sck_out_cb;
	devcb_write_line m_serial_out_cb;

	int8_t m_serial_in;
	int8_t m_sck_in;
	int8_t m_sck_out;
	uint8_t m_clock_count;
	uint8_t m_tx_shift_register;
	uint8_t m_rx_shift_register;
};

DECLARE_DEVICE_TYPE(KN5000_CPANEL, kn5000_cpanel_device)

#endif // MAME_MATSUSHITA_KN5000_CPANEL_H
