// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 1700/1750/1764 RAM Expansion Unit emulation

**********************************************************************/

#include "emu.h"
#include "reu.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define MOS8726R1_TAG   "u1"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_REU1700, c64_reu1700_cartridge_device, "c64_1700reu", "1700 REU")
DEFINE_DEVICE_TYPE(C64_REU1750, c64_reu1750_cartridge_device, "c64_1750reu", "1750 REU")
DEFINE_DEVICE_TYPE(C64_REU1764, c64_reu1764_cartridge_device, "c64_1764reu", "1764 REU")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c64_reu_cartridge_device::device_add_mconfig(machine_config &config)
{
	MOS8726(config, m_dmac, 1000000); // dummy clock

	GENERIC_SOCKET(config, m_eprom, generic_linear_slot, nullptr, "bin,rom");
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_reu_cartridge_device - constructor
//-------------------------------------------------

c64_reu_cartridge_device::c64_reu_cartridge_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t variant, int jp1, size_t ram_size) :
	device_t(mconfig, type, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this),
	m_dmac(*this, MOS8726R1_TAG),
	m_eprom(*this, "rom"),
	m_ram(*this, "ram"),
	m_variant(variant),
	m_jp1(jp1),
	m_ram_size(ram_size)
{
}

c64_reu1700_cartridge_device::c64_reu1700_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: c64_reu_cartridge_device(mconfig, C64_REU1700, tag, owner, clock, TYPE_1700, 0, 128 * 1024) { }

c64_reu1750_cartridge_device::c64_reu1750_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: c64_reu_cartridge_device(mconfig, C64_REU1750, tag, owner, clock, TYPE_1750, 1, 256 * 1024) { }

c64_reu1764_cartridge_device::c64_reu1764_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: c64_reu_cartridge_device(mconfig, C64_REU1764, tag, owner, clock, TYPE_1764, 1, 512 * 1024) { }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_reu_cartridge_device::device_start()
{
	// allocate memory
	m_ram.allocate(m_ram_size);

	// setup DMA controller
	m_dmac->set_unscaled_clock(m_slot->phi2());
	m_dmac->bs_w(m_jp1);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_reu_cartridge_device::device_reset()
{
	m_dmac->reset();
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

uint8_t c64_reu_cartridge_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!m_dmac->romsel_r(roml, romh))
	{
		data = m_eprom->read_rom(offset & 0x7fff);
	}
	else if (!io2)
	{
		data = m_dmac->read(offset);
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_reu_cartridge_device::c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io2)
	{
		m_dmac->write(offset, data);
	}
}
