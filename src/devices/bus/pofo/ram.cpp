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
	m_slot->memspace().install_readwrite_tap(0xc0000, 0xdffff, "ccm_ram",
		[this] (offs_t offset, u8 &data, u8) { if (m_selected) data = m_nvram[offset & 0x1ffff]; },
		[this] (offs_t offset, u8 &data, u8) { if (m_selected) m_nvram[offset & 0x1ffff] = data; });
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
