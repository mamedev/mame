// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Intel 8355 - 16,384-Bit ROM with I/O emulation

**********************************************************************/

#include "emu.h"
#include "i8355.h"

//#define VERBOSE 1
#include "logmacro.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

enum
{
	REGISTER_PORT_A = 0,
	REGISTER_PORT_B,
	REGISTER_PORT_A_DDR,
	REGISTER_PORT_B_DDR
};

enum
{
	PORT_A = 0,
	PORT_B,
	PORT_COUNT
};



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(I8355, i8355_device, "i8355", "Intel 8355")



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  read_port - read from input port
//-------------------------------------------------

inline u8 i8355_device::read_port(int port)
{
	u8 data = m_output[port] & m_ddr[port];

	if (m_ddr[port] != 0xff)
	{
		if (port == 0) { data |= m_in_pa_cb(0) & ~m_ddr[port]; }
		else { data |= m_in_pb_cb(0) & ~m_ddr[port]; }
	}

	return data;
}


//-------------------------------------------------
//  write_port - write to output port
//-------------------------------------------------

inline void i8355_device::write_port(int port, u8 data)
{
	m_output[port] = data;

	if (port == 0) {m_out_pa_cb((offs_t)0, m_output[port] & m_ddr[port]);}
	else {m_out_pb_cb((offs_t)0, m_output[port] & m_ddr[port]);}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  i8355_device - constructor
//-------------------------------------------------

i8355_device::i8355_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, I8355, tag, owner, clock),
		m_in_pa_cb(*this),
		m_out_pa_cb(*this),
		m_in_pb_cb(*this),
		m_out_pb_cb(*this),
		m_rom(*this, DEVICE_SELF)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i8355_device::device_start()
{
	// resolve callbacks
	m_in_pa_cb.resolve_safe(0);
	m_in_pb_cb.resolve_safe(0);
	m_out_pa_cb.resolve_safe();
	m_out_pb_cb.resolve_safe();

	// register for state saving
	save_item(NAME(m_output));
	save_item(NAME(m_ddr));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i8355_device::device_reset()
{
	// set ports to input mode
	m_ddr[PORT_A] = 0;
	m_ddr[PORT_B] = 0;
}


//-------------------------------------------------
//  io_r - register read
//-------------------------------------------------

u8 i8355_device::io_r(offs_t offset)
{
	int port = offset & 0x01;

	u8 data = 0;

	switch (offset & 0x03)
	{
	case REGISTER_PORT_A:
	case REGISTER_PORT_B:
		data = read_port(port);
		break;

	case REGISTER_PORT_A_DDR:
	case REGISTER_PORT_B_DDR:
		// write only
		break;
	}

	return data;
}


//-------------------------------------------------
//  io_w - register write
//-------------------------------------------------

void i8355_device::io_w(offs_t offset, u8 data)
{
	int port = offset & 0x01;

	switch (offset & 0x03)
	{
	case REGISTER_PORT_A:
	case REGISTER_PORT_B:
		LOG("I8355 Port %c Write %02x\n", 'A' + port, data);

		write_port(port, data);
		break;

	case REGISTER_PORT_A_DDR:
	case REGISTER_PORT_B_DDR:
		LOG("I8355 Port %c DDR: %02x\n", 'A' + port, data);

		m_ddr[port] = data;
		write_port(port, data);
		break;
	}
}


//-------------------------------------------------
//  memory_r - internal ROM read
//-------------------------------------------------

u8 i8355_device::memory_r(offs_t offset)
{
	return m_rom[offset];
}
