// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

FDD 2DD bridge for 1st gen HW

**************************************************************************************************/

#include "emu.h"
#include "fdd_2dd.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(FDD_2DD_BRIDGE, fdd_2dd_bridge_device, "pc98_fdd_2dd", "NEC PC-98 2DD FDD bridge")

fdd_2dd_bridge_device::fdd_2dd_bridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, FDD_2DD_BRIDGE, tag, owner, clock)
	, device_pc98_cbus_slot_interface(mconfig, *this)
	, m_bios(*this, "bios")
{
}

ROM_START( fdd_2dd )
	ROM_REGION( 0x8000, "bios", ROMREGION_ERASEFF )
	// from an onboard pc9801f
	ROM_LOAD16_BYTE( "urf01-01.bin", 0x00000, 0x4000, CRC(2f5ae147) SHA1(69eb264d520a8fc826310b4fce3c8323867520ee) )
	ROM_LOAD16_BYTE( "urf02-01.bin", 0x00001, 0x4000, CRC(62a86928) SHA1(4160a6db096dbeff18e50cbee98f5d5c1a29e2d1) )
ROM_END

const tiny_rom_entry *fdd_2dd_bridge_device::device_rom_region() const
{
	return ROM_NAME( fdd_2dd );
}

void fdd_2dd_bridge_device::device_add_mconfig(machine_config &config)
{
	// TODO: move from base driver
}


void fdd_2dd_bridge_device::device_start()
{
}

void fdd_2dd_bridge_device::device_reset()
{
}

void fdd_2dd_bridge_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_PROGRAM)
	{
		// TODO: any option to disconnect the ROM?
		logerror("map ROM at 0xd6000-0xd6fff\n");
		m_bus->space(AS_PROGRAM).install_rom(
			0xd6000,
			0xd6fff,
			m_bios->base()
		);

	}
	else if (space_id == AS_IO)
	{
		m_bus->install_device(0x0000, 0xffff, *this, &fdd_2dd_bridge_device::io_map);
	}
}

void fdd_2dd_bridge_device::io_map(address_map &map)
{
	// TODO: map me
}

