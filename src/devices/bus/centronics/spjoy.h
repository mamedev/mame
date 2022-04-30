// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    The Serial Port / Vertical Twist Joystick Interface

***************************************************************************/

#ifndef MAME_BUS_CENTRONICS_SPJOY_H
#define MAME_BUS_CENTRONICS_SPJOY_H

#pragma once

#include "ctronics.h"

class serial_port_joystick_device : public device_t,
	public device_centronics_peripheral_interface
{
public:
	// construction/destruction
	serial_port_joystick_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual ioport_constructor device_input_ports() const override;

	virtual DECLARE_WRITE_LINE_MEMBER( input_data0 ) override { if (state) m_data |= 0x01; else m_data &= ~0x01; update_busy_ack(); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data1 ) override { if (state) m_data |= 0x02; else m_data &= ~0x02; update_busy_ack(); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data2 ) override { if (state) m_data |= 0x04; else m_data &= ~0x04; update_busy_ack(); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data3 ) override { if (state) m_data |= 0x08; else m_data &= ~0x08; update_busy_ack(); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data4 ) override { if (state) m_data |= 0x10; else m_data &= ~0x10; update_busy_ack(); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data5 ) override { if (state) m_data |= 0x20; else m_data &= ~0x20; update_busy_ack(); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data6 ) override { if (state) m_data |= 0x40; else m_data &= ~0x40; update_busy_ack(); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data7 ) override { if (state) m_data |= 0x80; else m_data &= ~0x80; update_busy_ack(); }

private:
	required_ioport_array<2> m_joy;

	void update_busy_ack();

	uint8_t m_data;
	int m_busy;
	int m_ack;
};

// device type definition
DECLARE_DEVICE_TYPE(SERIAL_PORT_JOYSTICK, serial_port_joystick_device)

#endif // MAME_BUS_CENTRONICS_SPJOY_H
