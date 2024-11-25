// license:BSD-3-Clause
// copyright-holders:AJR
// thanks-to:Aaron Giles
/**********************************************************************

    Sony CXD1095 CMOS I/O Port Expander

    Based on Sega X Board driver by Aaron Giles

    This device provides four 8-bit ports (PA-PD) and one 4-bit port
    (PE or PX). All these ports can be configured for input or for
    output, entirely or in parts. The upper and lower halves of ports
    A-D can be separately configured by the first of the two
    write-only control registers. The second control register
    determines the direction of individual PE/PX bits.

**********************************************************************/

#include "emu.h"
#include "cxd1095.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(CXD1095, cxd1095_device, "cxd1095", "CXD1095 I/O Expander")

//**************************************************************************
//  DEVICE DEFINITION
//**************************************************************************

//-------------------------------------------------
//  cxd1095_device - constructor
//-------------------------------------------------

cxd1095_device::cxd1095_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, CXD1095, tag, owner, clock)
	, m_input_cb(*this, 0)
	, m_output_cb(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cxd1095_device::device_start()
{
	std::fill(std::begin(m_data_latch), std::end(m_data_latch), 0);

	// save state
	save_item(NAME(m_data_latch));
	save_item(NAME(m_data_dir));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cxd1095_device::device_reset()
{
	std::fill(std::begin(m_data_dir), std::end(m_data_dir), 0xff);
}

//-------------------------------------------------
//  read - read from an input port
//-------------------------------------------------

u8 cxd1095_device::read(offs_t offset)
{
	if (offset < 5)
	{
		u8 input_data = 0;
		u8 input_mask = m_data_dir[offset];
		if (offset == 4)
			input_mask &= 0x0f;

		// read through callback if port not configured entirely for output
		if (input_mask != 0 && !m_input_cb[offset].isunset())
			input_data = m_input_cb[offset](0, input_mask) & input_mask;
		else if (m_data_dir[offset] == 0xff)
			logerror("Reading from undefined input port %c\n", 'A' + offset);

		// combine live inputs with latched data
		return input_data | (m_data_latch[offset] & ~m_data_dir[offset]);
	}

	logerror("Reading from nonexistent port %c\n", 'A' + offset);
	return 0;
}

//-------------------------------------------------
//  write - write to an output port or one of two
//  control registers
//-------------------------------------------------

void cxd1095_device::write(offs_t offset, u8 data)
{
	if (offset < 5)
	{
		// port E is only 4 bits wide
		if (offset == 4)
			data &= 0x0f;

		// update our latched data
		m_data_latch[offset] = data;

		// send output through callback
		u8 dataout = data & ~m_data_dir[offset];
		if (!m_output_cb[offset].isunset())
			m_output_cb[offset](0, dataout, ~m_data_dir[offset]);
		else
			logerror("Writing %02X to undefined output port %c\n", dataout, 'A' + offset);
	}
	else if (offset == 6)
	{
		// control register 1 determines direction of each half of ports A-D
		for (int port = 0; port < 4; port++)
		{
			m_data_dir[port] = (BIT(data, 0) ? 0x0f : 0) | (BIT(data, 1) ? 0xf0 : 0);

			if (m_data_dir[port] != 0)
				logerror("Port %c & %02X configured for input\n", 'A' + port, m_data_dir[port]);
			if (m_data_dir[port] != 0xff)
				logerror("Port %c & %02X configured for output\n", 'A' + port, 0xff ^ m_data_dir[port]);

			data >>= 2;
		}
	}
	else if (offset == 7)
	{
		// control register 2 determines direction of the four port E bits
		m_data_dir[4] = (data & 0x0f) | 0xf0;

		if (m_data_dir[4] != 0xf0)
			logerror("Port E & %X configured for input\n", 0x0f & m_data_dir[4]);
		if (m_data_dir[4] != 0xff)
			logerror("Port E & %X configured for output\n", 0xff ^ m_data_dir[4]);
	}
}
