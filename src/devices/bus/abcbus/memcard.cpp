// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC Memory Card 55 10762-01 emulation

*********************************************************************/

/*

PCB Layout
----------

55 10762-01

|-----------------------------------|
|                                   |
|                                   |
|                                   |
|                                   |
|   ROM3        ROM2                |
|                                   |
|                                   |
|                                   |
|                                   |
|                                   |
|               ROM1        ROM0    |
|                                   |
|                                   |
|                                   |
|                                   |
|                                   |
|                                   |
|           LS02            LS139   |
|                                   |
|                                   |
|                                   |
|   LS367   LS241   LS241           |
|                                   |
|                                   |
|                                   |
|                                   |
|--|-----------------------------|--|
   |------------CON1-------------|

Notes:
    All IC's shown.

    ROM0    - Synertek C55022 4Kx8 ROM "DOSDD80"
    ROM1    - Motorola MCM2708C 1Kx8 EPROM "9704"
    ROM2    - empty socket
    ROM3    - empty socket
    CON1    - ABC bus connector

*/

#include "emu.h"
#include "memcard.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ABC_MEMORY_CARD, abc_memory_card_device, "abc_mem", "ABC Memory Card")


//-------------------------------------------------
//  ROM( abc_dos )
//-------------------------------------------------

ROM_START( abc_dos )
	ROM_REGION( 0x1000, "dos", 0 )
	ROM_DEFAULT_BIOS("ufd20")
	ROM_SYSTEM_BIOS( 0, "abcdos", "ABC-DOS" ) // Scandia Metric FD2
	ROMX_LOAD( "abcdos.3d",   0x0000, 0x1000, CRC(2cb2192f) SHA1(a6b3a9587714f8db807c05bee6c71c0684363744), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "dosdd80", "ABC-DOS DD" ) // ABC 830
	ROMX_LOAD( "dosdd80.3d",  0x0000, 0x1000, CRC(36db4c15) SHA1(ae462633f3a9c142bb029beb14749a84681377fa), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "ufd20", "UFD-DOS v.20" ) // ABC 830
	ROMX_LOAD( "ufddos20.3d", 0x0000, 0x1000, CRC(69b09c0b) SHA1(403997a06cf6495b8fa13dc74eff6a64ef7aa53e), ROM_BIOS(2) )

	ROM_REGION( 0x400, "iec", 0 )
	ROM_LOAD( "iec.4b", 0x000, 0x400, NO_DUMP )

	ROM_REGION( 0x400, "opt", 0 )
	ROM_LOAD( "spare.4a", 0x000, 0x400, NO_DUMP )

	ROM_REGION( 0x400, "prn", 0 )
	ROM_LOAD( "printer.3b", 0x000, 0x400, NO_DUMP )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *abc_memory_card_device::device_rom_region() const
{
	return ROM_NAME( abc_dos );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  abc_memory_card_device - constructor
//-------------------------------------------------

abc_memory_card_device::abc_memory_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ABC_MEMORY_CARD, tag, owner, clock),
	device_abcbus_card_interface(mconfig, *this),
	m_dos_rom(*this, "dos"),
	m_iec_rom(*this, "iec"),
	m_opt_rom(*this, "opt"),
	m_prn_rom(*this, "prn")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void abc_memory_card_device::device_start()
{
}



//**************************************************************************
//  ABC BUS INTERFACE
//**************************************************************************

//-------------------------------------------------
//  abcbus_xmemfl -
//-------------------------------------------------

uint8_t abc_memory_card_device::abcbus_xmemfl(offs_t offset)
{
	uint8_t data = 0xff;

	if (offset >= 0x6000 && offset < 0x7000)
	{
		data = m_dos_rom->base()[offset & 0xfff];
	}
	if (offset >= 0x7000 && offset < 0x7400)
	{
		data = m_iec_rom->base()[offset & 0x3ff];
	}
	if (offset >= 0x7400 && offset < 0x7800)
	{
		data = m_opt_rom->base()[offset & 0x3ff];
	}
	if (offset >= 0x7800 && offset < 0x7c00)
	{
		data = m_prn_rom->base()[offset & 0x3ff];
	}

	return data;
}
