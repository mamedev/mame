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

namespace bus { namespace ti99 { namespace joyport {

class joyport_device;

/********************************************************************
    Common parent class of all devices attached to the joystick port
********************************************************************/
class device_ti99_joyport_interface : public device_slot_card_interface
{
public:
	virtual uint8_t read_dev() = 0;
	virtual void write_dev(uint8_t data) = 0;
	virtual void pulse_clock() { }

protected:
	using device_slot_card_interface::device_slot_card_interface;

	virtual void interface_config_complete() override;
	joyport_device* m_joyport = nullptr;
};

/********************************************************************
    Joystick port
********************************************************************/
class joyport_device : public device_t, public device_slot_interface
{
public:
	joyport_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	uint8_t   read_port();
	void    write_port(int data);
	void    set_interrupt(int state);
	void    pulse_clock();

	template <class Object> devcb_base &set_int_callback(Object &&cb) { return m_interrupt.set_callback(std::forward<Object>(cb)); }

protected:
	void device_start() override;
	void device_config_complete() override;

private:
	devcb_write_line           m_interrupt;
	device_ti99_joyport_interface*    m_connected;
};

} } } // end namespace bus::ti99::joyport

void ti99_joystick_port(device_slot_interface &device);
void ti99_joystick_port_994(device_slot_interface &device);
void ti99_joystick_port_gen(device_slot_interface &device);

DECLARE_DEVICE_TYPE_NS(TI99_JOYPORT, bus::ti99::joyport, joyport_device)

#define MCFG_JOYPORT_INT_HANDLER( _intcallb ) \
	devcb = &downcast<bus::ti99::joyport::joyport_device &>(*device).set_int_callback(DEVCB_##_intcallb);

#define MCFG_GENEVE_JOYPORT_ADD( _tag )  \
	MCFG_DEVICE_ADD(_tag, TI99_JOYPORT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(ti99_joystick_port_gen, "twinjoy", false)

#define MCFG_TI_JOYPORT4A_ADD( _tag )    \
	MCFG_DEVICE_ADD(_tag, TI99_JOYPORT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(ti99_joystick_port, "twinjoy", false)

#define MCFG_TI_JOYPORT4_ADD( _tag ) \
	MCFG_DEVICE_ADD(_tag, TI99_JOYPORT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(ti99_joystick_port_994, "twinjoy", false)


#endif // MAME_BUS_TI99_JOYPORT_JOYPORT_H
