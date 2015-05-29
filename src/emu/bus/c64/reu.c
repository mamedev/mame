// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 1700/1750/1764 RAM Expansion Unit emulation

**********************************************************************/

#include "reu.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define MOS8726R1_TAG   "u1"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_REU1700 = &device_creator<c64_reu1700_cartridge_device>;
const device_type C64_REU1750 = &device_creator<c64_reu1750_cartridge_device>;
const device_type C64_REU1764 = &device_creator<c64_reu1764_cartridge_device>;


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( c64_reu )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c64_reu )
	MCFG_MOS8726_ADD(MOS8726R1_TAG)

	MCFG_GENERIC_SOCKET_ADD("rom", generic_linear_slot, NULL)
	MCFG_GENERIC_EXTENSIONS("bin,rom")
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor c64_reu_cartridge_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( c64_reu );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_reu_cartridge_device - constructor
//-------------------------------------------------

c64_reu_cartridge_device::c64_reu_cartridge_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT32 variant, int jp1, size_t ram_size, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_c64_expansion_card_interface(mconfig, *this),
	m_dmac(*this, MOS8726R1_TAG),
	m_eprom(*this, "rom"),
	m_ram(*this, "ram"),
	m_variant(variant),
	m_jp1(jp1),
	m_ram_size(ram_size)
{
}

c64_reu1700_cartridge_device::c64_reu1700_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: c64_reu_cartridge_device(mconfig, C64_REU1700, "1700 REU", tag, owner, clock, TYPE_1700, 0, 128 * 1024, "c64_1700reu", __FILE__) { }

c64_reu1750_cartridge_device::c64_reu1750_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: c64_reu_cartridge_device(mconfig, C64_REU1750, "1750 REU", tag, owner, clock, TYPE_1750, 1, 256 * 1024, "c64_1750reu", __FILE__) { }

c64_reu1764_cartridge_device::c64_reu1764_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: c64_reu_cartridge_device(mconfig, C64_REU1764, "1764 REU", tag, owner, clock, TYPE_1764, 1, 512 * 1024, "c64_1764reu", __FILE__) { }


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

UINT8 c64_reu_cartridge_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!m_dmac->romsel_r(roml, romh))
	{
		data = m_eprom->read_rom(space, offset & 0x7fff);
	}
	else if (!io2)
	{
		data = m_dmac->read(space, offset);
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_reu_cartridge_device::c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io2)
	{
		m_dmac->write(space, offset, data);
	}
}
