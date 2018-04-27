// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Sega 315-5338A

    I/O Controller

***************************************************************************/

#include "emu.h"
#include "315_5338a.h"

//#define VERBOSE 1
#include "logmacro.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SEGA_315_5338A, sega_315_5338a_device, "315_5338a", "Sega 315-5338A I/O Controller")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sega_315_5338a_device - constructor
//-------------------------------------------------

sega_315_5338a_device::sega_315_5338a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SEGA_315_5338A, tag, owner, clock),
	m_read_cb(*this), m_write_cb(*this),
	m_out0_cb(*this),
	m_in1_cb(*this),
	m_in2_cb(*this),
	m_in3_cb(*this),
	m_in4_cb(*this),
	m_out4_cb(*this),
	m_out5_cb(*this),
	m_in6_cb(*this),
	m_port0(0xff), m_config(0), m_serial_output(0), m_address(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sega_315_5338a_device::device_start()
{
	// resolve callbacks
	m_read_cb.resolve_safe(0xff);
	m_write_cb.resolve_safe();
	m_out0_cb.resolve_safe();
	m_in1_cb.resolve_safe(0xff);
	m_in2_cb.resolve_safe(0xff);
	m_in3_cb.resolve_safe(0xff);
	m_in4_cb.resolve_safe(0xff);
	m_out4_cb.resolve_safe();
	m_out5_cb.resolve_safe();
	m_in6_cb.resolve_safe(0xff);

	// register for save states
	save_item(NAME(m_port0));
	save_item(NAME(m_config));
	save_item(NAME(m_serial_output));
	save_item(NAME(m_address));
}


//**************************************************************************
//  INTERFACE
//**************************************************************************

READ8_MEMBER( sega_315_5338a_device::read )
{
	uint8_t data = 0xff;

	switch (offset)
	{
	// return output latch
	case 0x00: data = m_port0; break;

	// standard input ports
	case 0x01: data = m_in1_cb(0); break;
	case 0x02: data = m_in2_cb(0); break;
	case 0x03: data = m_in3_cb(0); break;

	// input/output port
	case 0x04: data = m_in4_cb(0); break;

	// input port
	case 0x06: data = m_in6_cb(0); break;

	// serial data read back?
	case 0x0a: data = m_serial_output; break;

	// serial data input
	case 0x0c: data = m_read_cb(m_address); break;

	// status register
	case 0x0d:
		// 7654----  unknown
		// ----3---  transfer finished?
		// -----21-  unknown
		// -------0  command acknowledged (0 = ack)
		data = 0x08;
		break;
	}

	LOG("RD %02x = %02x\n", offset, data);

	return data;
}

WRITE8_MEMBER( sega_315_5338a_device::write )
{
	LOG("WR %02x = %02x\n", offset, data);

	switch (offset)
	{
	// latched output port
	case 0x00: m_port0 = data; m_out0_cb(data); break;

	// input/output port
	case 0x04: m_out4_cb(data); break;

	// output port
	case 0x05: m_out5_cb(data); break;

	// config register?
	case 0x08:
		// 765-----  unknown
		// ---4----  port 4 direction (0 = output, 1 = input)
		// ----3210  unknown
		m_config = data;
		break;

	// command register
	case 0x09:
		switch (data)
		{
		case 0x00:
			m_address = (m_address & 0xff00) | (m_serial_output << 0);
			break;
		case 0x01:
			m_address = (m_address & 0x00ff) | (m_serial_output << 8);
			break;
		case 0x07:
			m_write_cb(m_address, m_serial_output, 0xff);
			break;
		case 0x87:
			// sent after setting up the address and when wanting to receive serial data
			break;
		default:
			logerror("Unknown command: %02x\n", data);
		}
		break;

	// serial data output
	case 0x0a: m_serial_output = data; break;
	}
}
