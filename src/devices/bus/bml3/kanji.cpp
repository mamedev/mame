// license:GPL-2.0+
// copyright-holders:Jonathan Edwards
/*********************************************************************

    bml3kanji.c

    Hitachi MP-9740 (?) kanji character ROM for the MB-689x

*********************************************************************/

#include "emu.h"
#include "kanji.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(BML3BUS_KANJI, bml3bus_kanji_device, "bml3kanji", "Hitachi MP-9740 Kanji Character ROM Card")

ROM_START( kanji )
	ROM_REGION( 0x20000, "kanji_rom", ROMREGION_ERASEFF )
	ROM_LOAD("kanji.rom", 0x00000, 0x20000, BAD_DUMP CRC(de99a726) SHA1(65fead5d0d779b242f6e0ac25fcc9899dc343101))
ROM_END


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bml3bus_kanji_device::device_rom_region() const
{
	return ROM_NAME( kanji );
}

uint8_t bml3bus_kanji_device::read(offs_t offset)
{
	return m_rom[((uint32_t)m_kanji_addr << 1) + offset];
}

void bml3bus_kanji_device::write(offs_t offset, uint8_t data)
{
	m_kanji_addr &= (0xff << (offset*8));
	m_kanji_addr |= (data << ((offset^1)*8));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

bml3bus_kanji_device::bml3bus_kanji_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, BML3BUS_KANJI, tag, owner, clock),
	device_bml3bus_card_interface(mconfig, *this), m_kanji_addr(0),
	m_rom(*this, "kanji_rom")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bml3bus_kanji_device::device_start()
{
	save_item(NAME(m_kanji_addr));
}

void bml3bus_kanji_device::map_io(address_space_installer &space)
{
	// install into memory
	space.install_readwrite_handler(0xff75, 0xff76, read8sm_delegate(*this, FUNC(bml3bus_kanji_device::read)), write8sm_delegate(*this, FUNC(bml3bus_kanji_device::write)));
}

void bml3bus_kanji_device::device_reset()
{
	m_kanji_addr = 0;
}
