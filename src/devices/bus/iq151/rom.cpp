// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    IQ151 rom cartridge emulation

    Supported cart:
    - BASIC6
    - BASICG
    - AMOS OS (3 cart)

***************************************************************************/

#include "emu.h"
#include "rom.h"

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

ROM_START( iq151_rom )
	ROM_REGION(0x4000, "rom", ROMREGION_ERASEFF)
ROM_END


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(IQ151_BASIC6, iq151_basic6_device, "iq151_basic6", "IQ151 BASIC6")
DEFINE_DEVICE_TYPE(IQ151_BASICG, iq151_basicg_device, "iq151_basicg", "IQ151 BASICG")
DEFINE_DEVICE_TYPE(IQ151_AMOS1,  iq151_amos1_device,  "iq151_amos1",  "IQ151 AMOS cart 1")
DEFINE_DEVICE_TYPE(IQ151_AMOS2,  iq151_amos2_device,  "iq151_amos2",  "IQ151 AMOS cart 2")
DEFINE_DEVICE_TYPE(IQ151_AMOS3,  iq151_amos3_device,  "iq151_amos3",  "IQ151 AMOS cart 3")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  iq151_rom_device - constructor
//-------------------------------------------------

iq151_rom_device::iq151_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_iq151cart_interface(mconfig, *this),
	m_rom(*this, "rom")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void iq151_rom_device::device_start()
{
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *iq151_rom_device::device_rom_region() const
{
	return ROM_NAME( iq151_rom );
}

/*-------------------------------------------------
    get_cart_base
-------------------------------------------------*/

uint8_t* iq151_rom_device::get_cart_base()
{
	return m_rom;
}


//**************************************************************************
//  BASIC6
//**************************************************************************

//-------------------------------------------------
//  iq151_basic6_device - constructor
//-------------------------------------------------

iq151_basic6_device::iq151_basic6_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: iq151_rom_device(mconfig, IQ151_BASIC6, tag, owner, clock)
{
}


//-------------------------------------------------
//  read
//-------------------------------------------------

void iq151_basic6_device::read(offs_t offset, uint8_t &data)
{
	if (offset >= 0xc800 && offset < 0xe800)
		data = m_rom[offset - 0xc800];
}


//**************************************************************************
//  BASICG
//**************************************************************************

//-------------------------------------------------
//  iq151_basicg_device - constructor
//-------------------------------------------------

iq151_basicg_device::iq151_basicg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: iq151_rom_device(mconfig, IQ151_BASICG, tag, owner, clock)
{
}

/*-------------------------------------------------
    read
-------------------------------------------------*/

void iq151_basicg_device::read(offs_t offset, uint8_t &data)
{
	if (offset >= 0xb000 && offset < 0xc000)
		data = m_rom[offset & 0x0fff];
	else if (offset >= 0xc800 && offset < 0xe800)
		data = m_rom[offset - 0xb800];
}


//**************************************************************************
//  AMOS cartridge 1
//**************************************************************************

//-------------------------------------------------
//  iq151_amos1_device - constructor
//-------------------------------------------------

iq151_amos1_device::iq151_amos1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: iq151_rom_device(mconfig, IQ151_AMOS1, tag, owner, clock)
	, m_active(true)
{
}

/*-------------------------------------------------
    read
-------------------------------------------------*/

void iq151_amos1_device::read(offs_t offset, uint8_t &data)
{
	if (offset >= 0x8000 && offset < 0xc000 && m_active)
		data = m_rom[offset & 0x3fff];
}

/*-------------------------------------------------
    IO write
-------------------------------------------------*/

void iq151_amos1_device::io_write(offs_t offset, uint8_t data)
{
	if (offset >= 0xec && offset < 0xf0)
		m_active = data == 0x00;
}

//**************************************************************************
//  AMOS cartridge 2
//**************************************************************************

//-------------------------------------------------
//  iq151_amos2_device - constructor
//-------------------------------------------------

iq151_amos2_device::iq151_amos2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: iq151_rom_device(mconfig, IQ151_AMOS2, tag, owner, clock)
	, m_active(false)
{
}

/*-------------------------------------------------
    read
-------------------------------------------------*/

void iq151_amos2_device::read(offs_t offset, uint8_t &data)
{
	if (offset >= 0x8000 && offset < 0xc000 && m_active)
		data = m_rom[offset & 0x3fff];
}

/*-------------------------------------------------
    IO write
-------------------------------------------------*/

void iq151_amos2_device::io_write(offs_t offset, uint8_t data)
{
	if (offset >= 0xec && offset < 0xf0)
		m_active = data == 0x01;
}

//**************************************************************************
//  AMOS cartridge 3
//**************************************************************************

//-------------------------------------------------
//  iq151_amos3_device - constructor
//-------------------------------------------------

iq151_amos3_device::iq151_amos3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: iq151_rom_device(mconfig, IQ151_AMOS3, tag, owner, clock)
	, m_active(true)
{
}

/*-------------------------------------------------
    read
-------------------------------------------------*/

void iq151_amos3_device::read(offs_t offset, uint8_t &data)
{
	if (offset >= 0x8000 && offset < 0xc000 && m_active)
		data = m_rom[offset & 0x3fff];
}

/*-------------------------------------------------
    IO write
-------------------------------------------------*/

void iq151_amos3_device::io_write(offs_t offset, uint8_t data)
{
	if (offset >= 0xec && offset < 0xf0)
		m_active = data == 0x02;
}
