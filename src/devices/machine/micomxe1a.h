// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/**********************************************************************

    Dempa Micom Soft Analog/Digital Controller emulation

**********************************************************************/
#ifndef MAME_MACHINE_MICOMXE1A_H
#define MAME_MACHINE_MICOMXE1A_H

#pragma once


class micom_xe_1a_device : public device_t
{
public:
	micom_xe_1a_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0) ATTR_COLD;
	virtual ~micom_xe_1a_device();

	auto buttons_handler() { return m_buttons_callback.bind(); }
	auto analog_handler() { return m_analog_callback.bind(); }

	u8 out_r();

	void req_w(int state);
	void mode_w(int state); // 0 = digital, 1 = analog
	void interface_w(int state); // 0 = PC, 1 = MD

protected:
	virtual void device_start() override ATTR_COLD;

private:
	TIMER_CALLBACK_MEMBER(step_output);

	devcb_read16 m_buttons_callback;
	devcb_read8 m_analog_callback;

	emu_timer *m_output_timer;

	u8 m_req;
	u8 m_mode;
	u8 m_interface;
	u8 m_data[6];
	u8 m_out;
};


DECLARE_DEVICE_TYPE(MICOM_XE_1A, micom_xe_1a_device)

#endif // MAME_MACHINE_MICOMXE1A_H
