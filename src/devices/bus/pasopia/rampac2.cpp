// license:BSD-3-Clause
// copyright-holders:AJR, Angelo Salese
/****************************************************************************

    Toshiba Pasopia RAM PAC2 emulation

    This emulates the later, larger cartridges, but not the original 4K
    model which used a 8255 PPI to latch the RAM address.

****************************************************************************/

#include "emu.h"
#include "rampac2.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definitions
DEFINE_DEVICE_TYPE(PASOPIA_PA7243, pasopia_pa7243_device, "pa7243", "PA7243 Pasopia RAM PAC2 (16KB)")
DEFINE_DEVICE_TYPE(PASOPIA_PA7245, pasopia_pa7245_device, "pa7245", "PA7245 Pasopia RAM PAC2 (32KB)")
DEFINE_DEVICE_TYPE(PASOPIA_PA7248, pasopia_pa7248_device, "pa7248", "PA7248 Pasopia RAM PAC2 (64KB)")

//**************************************************************************
//  DEVICE DEFINITION
//**************************************************************************

//-------------------------------------------------
//  pasopia_rampac2_device - construction
//-------------------------------------------------

pasopia_rampac2_device::pasopia_rampac2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 ram_size)
	: device_t(mconfig, type, tag, owner, clock)
	, pac2_card_interface(mconfig, *this)
	, device_nvram_interface(mconfig, *this)
	, m_ram_size(ram_size)
	, m_ram_index(0)
{
}


//-------------------------------------------------
//  pasopia_pa7243_device - construction
//-------------------------------------------------

pasopia_pa7243_device::pasopia_pa7243_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: pasopia_rampac2_device(mconfig, PASOPIA_PA7243, tag, owner, clock, 0x4000)
{
}


//-------------------------------------------------
//  pasopia_pa7245_device - construction
//-------------------------------------------------

pasopia_pa7245_device::pasopia_pa7245_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: pasopia_rampac2_device(mconfig, PASOPIA_PA7245, tag, owner, clock, 0x8000)
{
}


//-------------------------------------------------
//  pasopia_pa7248_device - construction
//-------------------------------------------------

pasopia_pa7248_device::pasopia_pa7248_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: pasopia_rampac2_device(mconfig, PASOPIA_PA7248, tag, owner, clock, 0x10000)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pasopia_rampac2_device::device_start()
{
	m_ram = std::make_unique<u8[]>(m_ram_size);

	save_pointer(NAME(m_ram), m_ram_size);
	save_item(NAME(m_ram_index));
}


//-------------------------------------------------
//  nvram_read - read NVRAM from the file
//-------------------------------------------------

bool pasopia_rampac2_device::nvram_read(util::read_stream &file)
{
	size_t actual;
	return !file.read(&m_ram[0], m_ram_size, actual) && actual == m_ram_size;
}


//-------------------------------------------------
//  nvram_write - write NVRAM to the file
//-------------------------------------------------

bool pasopia_rampac2_device::nvram_write(util::write_stream &file)
{
	size_t actual;
	return !file.write(&m_ram[0], m_ram_size, actual) && actual == m_ram_size;
}


//-------------------------------------------------
//  nvram_default - initialize NVRAM to its
//  default state
//-------------------------------------------------

void pasopia_rampac2_device::nvram_default()
{
	std::fill_n(&m_ram[0], m_ram_size, 0);
}

//**************************************************************************
//  PAC2 INTERFACE
//**************************************************************************

//-------------------------------------------------
//  pac2_read - I/O read access
//-------------------------------------------------

u8 pasopia_rampac2_device::pac2_read(offs_t offset)
{
	return m_ram[m_ram_index & (m_ram_size - 1)];
}


//-------------------------------------------------
//  pac2_write - I/O write access
//-------------------------------------------------

void pasopia_rampac2_device::pac2_write(offs_t offset, u8 data)
{
	switch (offset)
	{
	case 0:
		m_ram_index = (m_ram_index & 0xff00) | (data & 0xff);
		break;

	case 1:
		m_ram_index = (m_ram_index & 0xff) | ((data & 0xff) << 8);
		break;

	case 2: // PAC2 RAM write
		m_ram[m_ram_index & (m_ram_size - 1)] = data;
		break;
	}
}
