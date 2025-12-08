// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

FDD 2HD bridge for 1st gen HW

**************************************************************************************************/

#include "emu.h"
#include "fdd_2hd.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(FDD_2HD_BRIDGE, fdd_2hd_bridge_device, "pc98_fdd_2hd", "NEC PC-98 2HD FDD bridge")

fdd_2hd_bridge_device::fdd_2hd_bridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, FDD_2HD_BRIDGE, tag, owner, clock)
	, device_pc98_cbus_slot_interface(mconfig, *this)
	, m_bios(*this, "bios")
{
}

ROM_START( fdd_2hd )
	ROM_REGION( 0x8000, "bios", ROMREGION_ERASEFF )
	// from CSCP package
	ROM_LOAD( "2hdif.rom", 0x00000, 0x1000, BAD_DUMP CRC(9652011b) SHA1(b607707d74b5a7d3ba211825de31a8f32aec8146) )
ROM_END

const tiny_rom_entry *fdd_2hd_bridge_device::device_rom_region() const
{
	return ROM_NAME( fdd_2hd );
}

void fdd_2hd_bridge_device::device_add_mconfig(machine_config &config)
{
	// TODO: move from base driver
}


void fdd_2hd_bridge_device::device_start()
{
}

void fdd_2hd_bridge_device::device_reset()
{
}

void fdd_2hd_bridge_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_PROGRAM)
	{
		// TODO: any option to disconnect the ROM?
		logerror("map ROM at 0xd7000-0xd7fff\n");
		m_bus->space(AS_PROGRAM).install_rom(
			0xd7000,
			0xd7fff,
			m_bios->base()
		);
	}
	else if (space_id == AS_IO)
	{
		m_bus->install_device(0x0000, 0xffff, *this, &fdd_2hd_bridge_device::io_map);
	}
}

void fdd_2hd_bridge_device::io_map(address_map &map)
{
	// TODO: map me
}

