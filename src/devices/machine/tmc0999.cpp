// license:BSD-3-Clause
// copyright-holders:hap
/*

Texas Instruments TMC0999 256x4 RAM

It's not a standard RAM chip, it has separate pins for input and output
and an internal address latch.

TODO:
- no official documentation is known to exist, unknown if anything is missing
- pin names are unknown

*/

#include "emu.h"
#include "machine/tmc0999.h"


DEFINE_DEVICE_TYPE(TMC0999, tmc0999_device, "tmc0999", "TI TMC0999 RAM")

//-------------------------------------------------
//  constructor
//-------------------------------------------------

tmc0999_device::tmc0999_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, TMC0999, tag, owner, clock)
{ }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tmc0999_device::device_start()
{
	// zerofill
	m_data = 0;
	m_wr = 0;
	m_rd = 0;
	m_adr_strobe = 0;
	m_ram_address = 0;
	memset(m_ram, 0, sizeof(m_ram));

	// register for savestates
	save_item(NAME(m_data));
	save_item(NAME(m_wr));
	save_item(NAME(m_rd));
	save_item(NAME(m_adr_strobe));
	save_item(NAME(m_ram_address));
	save_item(NAME(m_ram));
}


//-------------------------------------------------
//  handlers
//-------------------------------------------------

void tmc0999_device::di_w(u8 data)
{
	m_data = data & 0xf;
}

u8 tmc0999_device::do_r()
{
	u8 data = m_rd ? m_ram[m_ram_address] : 0;
	return data & 0xf;
}

void tmc0999_device::wr_w(int state)
{
	state = state ? 1 : 0;

	// write to RAM on rising edge
	if (state && !m_wr)
		m_ram[m_ram_address] = m_data;
	m_wr = state;
}

void tmc0999_device::rd_w(int state)
{
	// enable data outputs
	m_rd = state ? 1 : 0;
}

void tmc0999_device::adr_w(int state)
{
	state = state ? 1 : 0;

	// set RAM address lo on rising edge
	// set RAM address hi on falling edge
	if (state && !m_adr_strobe)
		m_ram_address = (m_ram_address & 0xf0) | m_data;
	else if (!state && m_adr_strobe)
		m_ram_address = (m_ram_address & 0x0f) | m_data << 4;
	m_adr_strobe = state;
}
