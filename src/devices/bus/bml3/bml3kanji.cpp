// license:GPL-2.0+
// copyright-holders:Jonathan Edwards
/*********************************************************************

    bml3kanji.c

    Hitachi MP-9740 (?) kanji character ROM for the MB-689x

*********************************************************************/

#include "emu.h"
#include "bml3kanji.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type BML3BUS_KANJI = &device_creator<bml3bus_kanji_device>;

#define KANJI_ROM_REGION  "kanji_rom"

ROM_START( kanji )
	ROM_REGION( 0x20000, KANJI_ROM_REGION, ROMREGION_ERASEFF )
	ROM_LOAD("kanji.rom", 0x00000, 0x20000, BAD_DUMP CRC(de99a726) SHA1(65fead5d0d779b242f6e0ac25fcc9899dc343101))
ROM_END

MACHINE_CONFIG_FRAGMENT( kanji )
	// nothing to add
MACHINE_CONFIG_END

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor bml3bus_kanji_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( kanji );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *bml3bus_kanji_device::device_rom_region() const
{
	return ROM_NAME( kanji );
}

READ8_MEMBER( bml3bus_kanji_device::bml3_kanji_r )
{
	return m_rom[((UINT32)m_kanji_addr << 1) + offset];
}

WRITE8_MEMBER( bml3bus_kanji_device::bml3_kanji_w )
{
	m_kanji_addr &= (0xff << (offset*8));
	m_kanji_addr |= (data << ((offset^1)*8));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

bml3bus_kanji_device::bml3bus_kanji_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, BML3BUS_KANJI, "Hitachi MP-9740 Kanji Character ROM Card", tag, owner, clock, "bml3kanji", __FILE__),
	device_bml3bus_card_interface(mconfig, *this), m_kanji_addr(0), m_rom(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bml3bus_kanji_device::device_start()
{
	// set_bml3bus_device makes m_slot valid
	set_bml3bus_device();

	m_rom = memregion(KANJI_ROM_REGION)->base();

	// install into memory
	address_space &space_prg = machine().firstcpu->space(AS_PROGRAM);
	space_prg.install_readwrite_handler(0xff75, 0xff76, read8_delegate( FUNC(bml3bus_kanji_device::bml3_kanji_r), this), write8_delegate(FUNC(bml3bus_kanji_device::bml3_kanji_w), this) );
}

void bml3bus_kanji_device::device_reset()
{
	m_kanji_addr = 0;
}
