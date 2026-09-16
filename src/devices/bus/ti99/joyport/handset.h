// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    TI-99/4 handset
    See handset.c for documentation.

    This file also contains the implementation of the twin joystick;
    actually, no big deal, as it contains no logic but only switches.

    Michael Zapf, October 2010
    February 2012: Rewritten as class
    June 2012: Added joystick

*****************************************************************************/

#ifndef MAME_BUS_TI99_JOYPORT_HANDSET_H
#define MAME_BUS_TI99_JOYPORT_HANDSET_H

#pragma once

#include "joyport.h"

namespace bus::ti99::joyport {

class ti99_handset_device : public device_t, public device_ti99_joyport_interface
{
public:
	ti99_handset_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t read_dev() override;
	void  write_dev(uint8_t data) override;

	void pulse_clock() override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(delayed_data_ack);

private:
	static constexpr unsigned MAX_HANDSETS = 4;

	void do_task();
	void post_message(int message);
	bool poll_keyboard(int num);
	bool poll_joystick(int num);
	void set_acknowledge(int data);

	required_ioport_array<4> m_joyx;
	required_ioport_array<4> m_joyy;
	required_ioport_array<5> m_keys;

	int     m_ack;
	bool    m_clock_high;
	int     m_buf;
	int     m_buflen;
	uint8_t   previous_joy[MAX_HANDSETS];
	uint8_t   previous_key[MAX_HANDSETS];

	emu_timer *m_delay_timer;
};

/****************************************************************************/

class ti99_twin_joystick_device : public device_t, public device_ti99_joyport_interface
{
public:
	ti99_twin_joystick_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void device_start() override ATTR_COLD;

	uint8_t read_dev() override;
	void  write_dev(uint8_t data) override;

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	// Which joystick is selected?
	// In reality this is no latch but GND is put on one of the selector lines
	// and then routed back to the port via the joystick
	int m_joystick;

	required_ioport_array<2> m_joys;
};

} // end namespace bus::ti99::joyport

DECLARE_DEVICE_TYPE_NS(TI99_HANDSET, bus::ti99::joyport, ti99_handset_device)
DECLARE_DEVICE_TYPE_NS(TI99_JOYSTICK, bus::ti99::joyport, ti99_twin_joystick_device)

#endif // MAME_BUS_TI99_JOYPORT_HANDSET_H
