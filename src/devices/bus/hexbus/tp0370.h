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

namespace bus { namespace hexbus {

class ibc_device : public device_t
{
public:
	ibc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void device_start() override;
	void device_reset() override;

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	template <class Object> static devcb_base &set_ibc_int_callback(device_t &device, Object &&cb) { return downcast<ibc_device &>(device).m_int.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_hexbus_wr_callback(device_t &device, Object &&cb) { return downcast<ibc_device &>(device).m_hexout.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_hsklatch_wr_callback(device_t &device, Object &&cb) { return downcast<ibc_device &>(device).m_latch.set_callback(std::forward<Object>(cb)); }

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

	uint8_t m_data;
	uint8_t m_transmit;

	void set_disable_inhibit(bool dis, bool inh);
	void set_lines(bool bav, bool hsk);
};

}   }


/*
    Links to outside
*/

#define MCFG_IBC_HEXBUS_OUT_CALLBACK(_write) \
	devcb = &ibc_device::set_hexbus_wr_callback(*device, DEVCB_##_write);

#define MCFG_IBC_HSKLATCH_CALLBACK(_write) \
	devcb = &ibc_device::set_hsklatch_wr_callback(*device, DEVCB_##_write);

#define MCFG_IBC_INT_CALLBACK(_write) \
	devcb = &ibc_device::set_ibc_int_callback(*device, DEVCB_##_write);

DECLARE_DEVICE_TYPE_NS(IBC, bus::hexbus, ibc_device)
#endif
