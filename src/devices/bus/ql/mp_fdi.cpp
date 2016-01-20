// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Micro Peripherals Floppy Disk Interface emulation

**********************************************************************/

#include "mp_fdi.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type MICRO_PERIPHERALS_FLOPPY_DISK_INTERFACE = &device_creator<micro_peripherals_floppy_disk_interface_t>;


//-------------------------------------------------
//  ROM( micro_peripherals_floppy_disk_interface )
//-------------------------------------------------

ROM_START( micro_peripherals_floppy_disk_interface )
	ROM_REGION( 0x2000, "rom", 0 )
	ROM_DEFAULT_BIOS("v53e")
	ROM_SYSTEM_BIOS( 0, "v53e", "v5.3E" )
	ROMX_LOAD( "microp_disk system_v5.3e_1985.rom", 0x0000, 0x2000, CRC(9a8d8fa7) SHA1(f9f5e5d55f3046f63b4eae59222b81290d626e72), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *micro_peripherals_floppy_disk_interface_t::device_rom_region() const
{
	return ROM_NAME( micro_peripherals_floppy_disk_interface );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  micro_peripherals_floppy_disk_interface_t - constructor
//-------------------------------------------------

micro_peripherals_floppy_disk_interface_t::micro_peripherals_floppy_disk_interface_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, MICRO_PERIPHERALS_FLOPPY_DISK_INTERFACE, "Micro Peripherals Floppy Disk Interface", tag, owner, clock, "ql_mpfdi", __FILE__),
	device_ql_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void micro_peripherals_floppy_disk_interface_t::device_start()
{
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

UINT8 micro_peripherals_floppy_disk_interface_t::read(address_space &space, offs_t offset, UINT8 data)
{
	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void micro_peripherals_floppy_disk_interface_t::write(address_space &space, offs_t offset, UINT8 data)
{
}
