// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Subsino SS9602 & SS9802 I/O emulation

    These are QFP100 gate arrays custom-programmed as GPI/O port
    expanders. Outputs are ULN2003A compatible; inputs may be 74LS244
    buffered due to electrical requirements. Much like Sega 315-5296,
    some weak protection is provided by making several bytes spell out
    the game manufacturer's name.

    One port on each IC does not appear to have a separate direction
    register, and often has the same value written into both nibbles.
    The implementation here is a bit conjectural.

    TBD: are the separate direction registers write-only, or can they
    be read back?

**********************************************************************/

#include "emu.h"
#include "subsino_io.h"

#define LOG_SETUP (1U << 1)

//#define VERBOSE (LOG_SETUP)
#include "logmacro.h"

// device type definitions
DEFINE_DEVICE_TYPE(SS9602, ss9602_device, "ss9602", "Subsino SS9602 I/O")
DEFINE_DEVICE_TYPE(SS9802, ss9802_device, "ss9802", "Subsino SS9802 I/O")

subsino_io_device::subsino_io_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_in_port_callback(*this, 0xff)
	, m_out_port_callback(*this)
	, m_port_data{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
	, m_port_dir{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
{
}

ss9602_device::ss9602_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: subsino_io_device(mconfig, SS9602, tag, owner, clock)
{
}

ss9802_device::ss9802_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: subsino_io_device(mconfig, SS9802, tag, owner, clock)
{
}

void subsino_io_device::device_start()
{
	// save state
	save_item(NAME(m_port_data));
	save_item(NAME(m_port_dir));
}

void subsino_io_device::device_reset()
{
	std::fill(std::begin(m_port_data), std::end(m_port_data), 0);
	std::fill(std::begin(m_port_dir), std::end(m_port_dir), 0);
}

u8 subsino_io_device::read_port_data(unsigned port)
{
	const u8 dir = m_port_dir[port];
	u8 data = m_port_data[port] & dir;
	if (dir != 0xff)
	{
		if (!m_in_port_callback[port].isunset())
			data |= m_in_port_callback[port]() & ~dir;
		else if (!machine().side_effects_disabled())
			logerror("%s: Reading from undefined P%u input\n", machine().describe_context(), port);
	}
	return data;
}

void subsino_io_device::write_port_data(unsigned port, u8 data)
{
	const u8 dir = m_port_dir[port];
	if ((dir & (data ^ m_port_data[port])) != 0)
	{
		LOG("%s: Writing %02Xh to P%u output\n", machine().describe_context(), data, port);
		m_out_port_callback[port](data & dir);
	}
	m_port_data[port] = data;
}

void subsino_io_device::write_port_dir(unsigned port, u8 dir)
{
	LOGMASKED(LOG_SETUP, "%s: Writing %02Xh to P%u direction register (previous data = %02Xh)\n", machine().describe_context(), dir, port, m_port_data[port]);
	if (m_port_dir[port] != dir)
	{
		m_port_dir[port] = dir;
		m_out_port_callback[port](m_port_data[port] & dir);
	}
}

void subsino_io_device::write_port_data_and_dir(unsigned port, u8 data, u8 dir)
{
	if (m_port_dir[port] != dir || (dir & (data ^ m_port_data[port])) != 0)
	{
		LOG("%s: Writing %02Xh & %02Xh to P%u output\n", machine().describe_context(), data, dir, port);
		m_port_dir[port] = dir;
		m_out_port_callback[port](data & dir);
	}
	m_port_data[port] = data;
}

u8 ss9602_device::read(offs_t offset)
{
	offset &= 0x1f;
	if (offset <= 0x08)
		return read_port_data(offset);
	else if (offset == 0x12)
		return read_port_data(9) & 0x0f;
	else if (offset >= 0x18 && offset <= 0x1e)
		return "SUBSION"[offset - 0x18];
	else
	{
		if (!machine().side_effects_disabled())
			logerror("%s: Reading data from unknown/write-only register %02Xh\n", machine().describe_context(), offset);
		return 0;
	}
}

void ss9602_device::write(offs_t offset, u8 data)
{
	offset &= 0x1f;
	if (offset <= 0x08)
		write_port_data(offset, data);
	else if (offset <= 0x11)
		write_port_dir(offset - 0x09, data);
	else if (offset == 0x12)
		write_port_data_and_dir(9, data & 0x0f, (data & 0xf0) >> 4);
	else if (offset == 0x13)
		LOGMASKED(LOG_SETUP, "%s: Writing %02Xh to reset register\n", machine().describe_context(), data);
	else
		logerror("%s: Writing %02Xh to unknown register %02Xh\n", machine().describe_context(), data, offset);
}

u8 ss9802_device::read(offs_t offset)
{
	offset &= 0x1f;
	if (offset == 0x00)
		return (read_port_data(0) & 0xf0) | (m_port_dir[0] >> 4);
	else if (offset <= 0x09)
		return read_port_data(offset);
	else if (offset >= 0x13 && offset <= 0x19)
		return "SUBSINO"[offset - 0x13]; // for xtrain
	else
	{
		if (!machine().side_effects_disabled())
			logerror("%s: Reading data from unknown/write-only register %02Xh\n", machine().describe_context(), offset);
		return 0;
	}
}

void ss9802_device::write(offs_t offset, u8 data)
{
	offset &= 0x1f;
	if (offset == 0x00)
		write_port_data_and_dir(0, data & 0xf0, (data & 0x0f) << 4);
	else if (offset <= 0x09)
		write_port_data(offset, data);
	else if (offset <= 0x12)
		write_port_dir(offset - 0x09, data);
	else
		logerror("%s: Writing %02Xh to unknown register %02Xh\n", machine().describe_context(), data, offset);
}
