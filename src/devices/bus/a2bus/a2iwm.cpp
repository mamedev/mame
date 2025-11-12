// license:BSD-3-Clause
// copyright-holders:R. Belmont, O. Galibert
/*********************************************************************

    a2iwm.c

    Implementation of the Apple II IWM controller card

    WANTED: there are no ROM dumps from this card in any form
    (the IWM card, the UniDisk )

*********************************************************************/

#include "emu.h"
#include "a2iwm.h"

#include "machine/applefdintf.h"
#include "formats/ap2_dsk.h"
#include "formats/as_dsk.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A2BUS_IWM, a2bus_iwm_int_device, "a2iwmint", "Apple IWM controller")
DEFINE_DEVICE_TYPE(A2BUS_IWM_CARD, a2bus_iwm_card_device, "a2iwm", "Apple Disk II IWM controller")

#define DISKII_ROM_REGION  "diskii_rom"

ROM_START( iwm )
	ROM_REGION(0x100, DISKII_ROM_REGION, 0)
	ROM_LOAD( "341-0027-a.p5", 0x000000, 0x000100, CRC(ce7144f6) SHA1(d4181c9f046aafc3fb326b381baac809d9e38d16) )
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_iwm_device::device_add_mconfig(machine_config &config)
{
	IWM(config, m_iwm, clock(), 1021800*2);
	m_iwm->phases_cb().set(FUNC(a2bus_iwm_device::phases_w));
	m_iwm->devsel_cb().set(FUNC(a2bus_iwm_device::devsel_w));
	applefdintf_device::add_525(config, m_floppy[0]);
	applefdintf_device::add_525(config, m_floppy[1]);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *a2bus_iwm_card_device::device_rom_region() const
{
	return ROM_NAME( iwm );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_iwm_device::a2bus_iwm_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_iwm(*this, "iwm"),
	m_floppy(*this, "%u", 0U)
{
}

a2bus_iwm_int_device::a2bus_iwm_int_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_iwm_device(mconfig, A2BUS_IWM, tag, owner, clock)
{
}

a2bus_iwm_card_device::a2bus_iwm_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_iwm_device(mconfig, A2BUS_IWM_CARD, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_iwm_device::device_start()
{
}

void a2bus_iwm_card_device::device_start()
{
	a2bus_iwm_device::device_start();
	m_rom = device().machine().root_device().memregion(this->subtag(DISKII_ROM_REGION).c_str())->base();
}

void a2bus_iwm_device::device_reset()
{
}

void a2bus_iwm_device::reset_from_bus()
{
	m_iwm->reset();
}

/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

uint8_t a2bus_iwm_device::read_c0nx(u8 offset)
{
	return m_iwm->read(offset);
}


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_iwm_device::write_c0nx(u8 offset, u8 data)
{
	m_iwm->write(offset, data);
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's c0nx space
-------------------------------------------------*/

uint8_t a2bus_iwm_card_device::read_cnxx(uint8_t offset)
{
	return m_rom[offset];
}

void a2bus_iwm_device::devsel_w(u8 data)
{
	if(data & 1)
		m_iwm->set_floppy(m_floppy[0]->get_device());
	else if(data & 2)
		m_iwm->set_floppy(m_floppy[1]->get_device());
	else
		m_iwm->set_floppy(nullptr);
}

void a2bus_iwm_device::phases_w(u8 data)
{
	auto flp = m_iwm->get_floppy();
	if(flp)
		flp->seek_phase_w(data);
}
