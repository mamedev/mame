// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Portfolio 128KB RAM card emulation

**********************************************************************/

#include "emu.h"
#include "ram.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PORTFOLIO_RAM_CARD, portfolio_ram_card_device, "portfolio_ram_card", "Atari Portfolio RAM card")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  portfolio_ram_card_device - constructor
//-------------------------------------------------

portfolio_ram_card_device::portfolio_ram_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PORTFOLIO_RAM_CARD, tag, owner, clock),
	device_portfolio_memory_card_slot_interface(mconfig, *this),
	device_nvram_interface(mconfig, *this),
	m_nvram(*this, "nvram", 0x20000, ENDIANNESS_LITTLE)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void portfolio_ram_card_device::device_start()
{
}


void portfolio_ram_card_device::nvram_default()
{
}


bool portfolio_ram_card_device::nvram_read(util::read_stream &file)
{
	auto const [err, actual] = read(file, m_nvram, m_nvram.bytes());
	return !err && (actual == m_nvram.bytes());
}


bool portfolio_ram_card_device::nvram_write(util::write_stream &file)
{
	auto const [err, actual] = write(file, m_nvram, m_nvram.bytes());
	return !err;
}


//-------------------------------------------------
//  nrdi_r - read
//-------------------------------------------------

uint8_t portfolio_ram_card_device::nrdi_r(offs_t offset)
{
	return m_nvram[offset];
}


//-------------------------------------------------
//  nwri_w - write
//-------------------------------------------------

void portfolio_ram_card_device::nwri_w(offs_t offset, uint8_t data)
{
	m_nvram[offset] = data;
}
