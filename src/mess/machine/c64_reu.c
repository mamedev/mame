/**********************************************************************

    Commodore 1700/1750/1764 RAM Expansion Unit emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "c64_reu.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define MOS8726R1_TAG	"u1"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_REU1700 = &device_creator<c64_reu1700_cartridge_device>;
const device_type C64_REU1750 = &device_creator<c64_reu1750_cartridge_device>;
const device_type C64_REU1764 = &device_creator<c64_reu1764_cartridge_device>;


//-------------------------------------------------
//  ROM( c64_reu )
//-------------------------------------------------

ROM_START( c64_reu )
	ROM_REGION( 0x8000, "roml", 0 )
	ROM_CART_LOAD( "rom", 0x0000, 0x8000, ROM_MIRROR )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *c64_reu_cartridge_device::device_rom_region() const
{
	return ROM_NAME( c64_reu );
}


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( c64_reu )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c64_reu )
	MCFG_MOS8726_ADD(MOS8726R1_TAG)

	MCFG_CARTSLOT_ADD("rom")
	MCFG_CARTSLOT_EXTENSION_LIST("rom,bin")
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

c64_reu_cartridge_device::c64_reu_cartridge_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT32 variant, int jp1, size_t ram_size) :
	device_t(mconfig, type, name, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this),
	m_dmac(*this, MOS8726R1_TAG),
	m_variant(variant),
	m_jp1(jp1),
	m_ram_size(ram_size)
{
}

c64_reu1700_cartridge_device::c64_reu1700_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: c64_reu_cartridge_device(mconfig, C64_REU1700, "1700 REU", tag, owner, clock, TYPE_1700, 0, 128 * 1024) { }

c64_reu1750_cartridge_device::c64_reu1750_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: c64_reu_cartridge_device(mconfig, C64_REU1750, "1750 REU", tag, owner, clock, TYPE_1750, 1, 256 * 1024) { }

c64_reu1764_cartridge_device::c64_reu1764_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: c64_reu_cartridge_device(mconfig, C64_REU1764, "1764 REU", tag, owner, clock, TYPE_1764, 1, 512 * 1024) { }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_reu_cartridge_device::device_start()
{
	// find memory region
	m_roml = memregion("roml")->base();

	// allocate memory
	c64_ram_pointer(machine(), m_ram_size);

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

UINT8 c64_reu_cartridge_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int ba, int roml, int romh, int io1, int io2)
{
	if (!m_dmac->romsel_r(roml, romh))
	{
		data = m_roml[offset & 0x7fff];
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

void c64_reu_cartridge_device::c64_cd_w(address_space &space, offs_t offset, UINT8 data, int ba, int roml, int romh, int io1, int io2)
{
	if (!io2)
	{
		m_dmac->write(space, offset, data);
	}
}
