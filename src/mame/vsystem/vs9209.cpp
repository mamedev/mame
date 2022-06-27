// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    VS9209 (4L01F1429) custom QFP80 I/O

    This chip, which appears on various Video System PCBs from 1992
    to 1995, provides a programmable interface for up to eight ports.

    There are at least four configuration registers that control the
    directions of individual bits on some of the ports (low for input
    and high for output). However, the game programs always write
    zero to the first register, except for Super Slams which doesn't
    write to it at all.

    No program attempts to write to Ports A, B, C or D, with the
    dubious exception of Tao Taido writing to the Port C offset on
    its second VS9209 (whose inputs are mostly unused) at
    initialization time. It seems possible that only the latter four
    ports may be configured for output.

    Much like CXD1095, the last port is apparently only half width.

**********************************************************************/

#include "emu.h"
#include "vs9209.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(VS9209, vs9209_device, "vs9209", "VS9209 I/O")

//**************************************************************************
//  DEVICE DEFINITION
//**************************************************************************

//-------------------------------------------------
//  vs9209_device - constructor
//-------------------------------------------------

vs9209_device::vs9209_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, VS9209, tag, owner, clock)
	, m_input_cb(*this)
	, m_output_cb(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vs9209_device::device_start()
{
	// resolve callbacks
	m_input_cb.resolve_all();
	m_output_cb.resolve_all();

	std::fill(std::begin(m_data_latch), std::end(m_data_latch), 0);

	// save state
	save_item(NAME(m_data_latch));
	save_item(NAME(m_data_dir));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vs9209_device::device_reset()
{
	std::fill(std::begin(m_data_dir), std::end(m_data_dir), 0);
}

//-------------------------------------------------
//  read - read from an input port
//-------------------------------------------------

u8 vs9209_device::read(address_space &space, offs_t offset)
{
	int port = offset & 7;

	if ((offset & 8) == 0)
	{
		u8 input_data = 0;
		u8 input_mask = ~m_data_dir[port];
		if (port == 7)
			input_mask &= 0x0f;

		// read through callback if port not configured entirely for output
		if (input_mask != 0 && !m_input_cb[port].isnull())
			input_data = m_input_cb[port](0, input_mask) & input_mask;
		else if (m_data_dir[port] == 0)
			logerror("%s: Read from undefined input port %c\n", machine().describe_context(), 'A' + port);

		// combine live inputs with latched data
		return input_data | (m_data_latch[port] & m_data_dir[port]);
	}

	//logerror("%s: Read from write-only/nonexistent register %d\n", machine().describe_context(), offset);
	return space.unmap();
}

//-------------------------------------------------
//  write - write to an output port or one of two
//  control registers
//-------------------------------------------------

void vs9209_device::write(offs_t offset, u8 data)
{
	// port H is probably only 4 bits wide
	int port = offset & 7;
	if (port == 7 && (data & 0xf0) != 0)
	{
		logerror("%s: Attempt to write %02X to port H%s", machine().describe_context(), data, (offset & 8) != 0 ? " direction register" : "");
		data &= 0x0f;
	}

	if ((offset & 8) == 0)
	{
		// update our latched data
		m_data_latch[port] = data;

		if (m_data_dir[port] != 0)
		{
			u8 dataout = data & m_data_dir[port];

			// send output through callback
			if (!m_output_cb[port].isnull())
				m_output_cb[port](0, dataout, m_data_dir[port]);
			else
				logerror("%s: Writing %02X to undefined output port %c\n", machine().describe_context(), dataout, 'A' + port);
		}
		else if (m_output_cb[port].isnull())
			logerror("%s: Writing %02X to input-only port %c\n", machine().describe_context(), data, 'A' + port);
	}
	else
	{
		u8 old_data_dir = m_data_dir[port];
		m_data_dir[port] = data;

		u8 all_port_bits = (port == 7) ? 0x0f : 0xff;
		if (data != all_port_bits)
			logerror("Port %c & %02X configured for input\n", 'A' + port, all_port_bits ^ data);
		if (data != 0)
		{
			logerror("Port %c & %02X configured for output\n", 'A' + port, data);

			// if direction changed to output, begin output from latch
			if ((data & ~old_data_dir) != 0 && !m_output_cb[port].isnull())
				m_output_cb[port](0, m_data_latch[port], data);
		}
	}
}
