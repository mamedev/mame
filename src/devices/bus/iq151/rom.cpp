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

const device_type IQ151_BASIC6 = &device_creator<iq151_basic6_device>;
const device_type IQ151_BASICG = &device_creator<iq151_basicg_device>;
const device_type IQ151_AMOS1 = &device_creator<iq151_amos1_device>;
const device_type IQ151_AMOS2 = &device_creator<iq151_amos2_device>;
const device_type IQ151_AMOS3 = &device_creator<iq151_amos3_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  iq151_rom_device - constructor
//-------------------------------------------------

iq151_rom_device::iq151_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
		device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_iq151cart_interface(mconfig, *this), m_rom(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void iq151_rom_device::device_start()
{
	m_rom = (UINT8*)memregion("rom")->base();
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *iq151_rom_device::device_rom_region() const
{
	return ROM_NAME( iq151_rom );
}

/*-------------------------------------------------
    get_cart_base
-------------------------------------------------*/

UINT8* iq151_rom_device::get_cart_base()
{
	return m_rom;
}


//**************************************************************************
//  BASIC6
//**************************************************************************

//-------------------------------------------------
//  iq151_basic6_device - constructor
//-------------------------------------------------

iq151_basic6_device::iq151_basic6_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: iq151_rom_device(mconfig, IQ151_BASIC6, "IQ151 BASIC6", tag, owner, clock, "iq151_basic6", __FILE__)
{
}


//-------------------------------------------------
//  read
//-------------------------------------------------

void iq151_basic6_device::read(offs_t offset, UINT8 &data)
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

iq151_basicg_device::iq151_basicg_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: iq151_rom_device(mconfig, IQ151_BASICG, "IQ151 BASICG", tag, owner, clock, "iq151_basicg", __FILE__)
{
}

/*-------------------------------------------------
    read
-------------------------------------------------*/

void iq151_basicg_device::read(offs_t offset, UINT8 &data)
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

iq151_amos1_device::iq151_amos1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: iq151_rom_device(mconfig, IQ151_AMOS1, "IQ151 AMOS cart 1", tag, owner, clock, "iq151_amos1", __FILE__),
		m_active(true)
{
}

/*-------------------------------------------------
    read
-------------------------------------------------*/

void iq151_amos1_device::read(offs_t offset, UINT8 &data)
{
	if (offset >= 0x8000 && offset < 0xc000 && m_active)
		data = m_rom[offset & 0x3fff];
}

/*-------------------------------------------------
    IO write
-------------------------------------------------*/

void iq151_amos1_device::io_write(offs_t offset, UINT8 data)
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

iq151_amos2_device::iq151_amos2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: iq151_rom_device(mconfig, IQ151_AMOS2, "IQ151 AMOS cart 2", tag, owner, clock, "iq151_amos2", __FILE__),
		m_active(false)
{
}

/*-------------------------------------------------
    read
-------------------------------------------------*/

void iq151_amos2_device::read(offs_t offset, UINT8 &data)
{
	if (offset >= 0x8000 && offset < 0xc000 && m_active)
		data = m_rom[offset & 0x3fff];
}

/*-------------------------------------------------
    IO write
-------------------------------------------------*/

void iq151_amos2_device::io_write(offs_t offset, UINT8 data)
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

iq151_amos3_device::iq151_amos3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: iq151_rom_device(mconfig, IQ151_AMOS3, "IQ151 AMOS cart 3", tag, owner, clock, "iq151_amos3", __FILE__),
		m_active(true)
{
}

/*-------------------------------------------------
    read
-------------------------------------------------*/

void iq151_amos3_device::read(offs_t offset, UINT8 &data)
{
	if (offset >= 0x8000 && offset < 0xc000 && m_active)
		data = m_rom[offset & 0x3fff];
}

/*-------------------------------------------------
    IO write
-------------------------------------------------*/

void iq151_amos3_device::io_write(offs_t offset, UINT8 data)
{
	if (offset >= 0xec && offset < 0xf0)
		m_active = data == 0x02;
}
