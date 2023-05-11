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

DEFINE_DEVICE_TYPE(ABC_MEMORY_CARD, abc_memory_card_device, "abc_memcard", "ABC Memory Card")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void abc_memory_card_device::device_add_mconfig(machine_config & config)
{
	GENERIC_SOCKET(config, m_dos_rom, generic_plain_slot, "abc80_dos", "bin");
	GENERIC_SOCKET(config, m_iec_rom, generic_plain_slot, "abc80_iec", "bin");
	GENERIC_SOCKET(config, m_opt_rom, generic_plain_slot, "abc80_opt", "bin");
	GENERIC_SOCKET(config, m_prn_rom, generic_plain_slot, "abc80_prn", "bin");
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
		data = m_dos_rom->read_rom(offset & 0xfff);
	}
	if (offset >= 0x7000 && offset < 0x7400)
	{
		data = m_iec_rom->read_rom(offset & 0x3ff);
	}
	if (offset >= 0x7400 && offset < 0x7800)
	{
		data = m_opt_rom->read_rom(offset & 0x3ff);
	}
	if (offset >= 0x7800 && offset < 0x7c00)
	{
		data = m_prn_rom->read_rom(offset & 0x3ff);
	}

	return data;
}
