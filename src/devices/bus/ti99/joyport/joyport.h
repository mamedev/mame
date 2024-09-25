// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Joystick port

    Now explicitly implemented as a slot device
    A joystick port allows for plugging in a single or twin joystick, or
    a Mechatronics mouse.

    The TI-99/4 also offers an infrared handset, connected to this port. For
    this reason we also need an interrupt line.

    Michael Zapf

    June 2012

*****************************************************************************/

#ifndef MAME_BUS_TI99_JOYPORT_JOYPORT_H
#define MAME_BUS_TI99_JOYPORT_JOYPORT_H

#pragma once

namespace bus::ti99::joyport {

enum
{
	PLAIN=0,
	MOUSE,
	HANDSET
};

#define TI_JOYPORT_TAG     "joyport"

class joyport_device;

/********************************************************************
    Common parent class of all devices attached to the joystick port
********************************************************************/
class device_ti99_joyport_interface : public device_interface
{
public:
	virtual uint8_t read_dev() = 0;
	virtual void write_dev(uint8_t data) = 0;
	virtual void pulse_clock() { }

protected:
	device_ti99_joyport_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_config_complete() override;
	joyport_device* m_joyport;
};

/********************************************************************
    Joystick port
********************************************************************/
class joyport_device : public device_t, public device_single_card_slot_interface<device_ti99_joyport_interface>
{
public:
	template <typename U>
	joyport_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, U &&opts, const char *dflt)
		: joyport_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	joyport_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	uint8_t   read_port();
	void    write_port(int data);
	void    set_interrupt(int state);
	void    pulse_clock();
	auto    int_cb() { return m_interrupt.bind(); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_config_complete() override;

private:
	devcb_write_line           m_interrupt;
	device_ti99_joyport_interface*    m_connected;
};

} // end namespace bus::ti99::joyport

DECLARE_DEVICE_TYPE_NS(TI99_JOYPORT, bus::ti99::joyport, joyport_device)

void ti99_joyport_options_plain(device_slot_interface &device);
void ti99_joyport_options_mouse(device_slot_interface &device);
void ti99_joyport_options_994(device_slot_interface &device);

#endif // MAME_BUS_TI99_JOYPORT_JOYPORT_H
