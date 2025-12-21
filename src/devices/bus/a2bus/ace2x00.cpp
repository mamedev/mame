// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    ace2x00.c

    Helpers for the Franklin Ace 2x00's slot 1 and 6 internal peripherals

*********************************************************************/

#include "emu.h"
#include "ace2x00.h"

//#define VERBOSE 1
#include "logmacro.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A2BUS_ACE2X00_SLOT1, a2bus_ace2x00_slot1_device, "ace2x00s1", "Franklin Ace 2x00 Parallel Port")
DEFINE_DEVICE_TYPE(A2BUS_ACE2X00_SLOT6, a2bus_ace2x00_slot6_device, "ace2x00s6", "Franklin Ace 2x00 Disk Port")

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_ace2x00_device::device_add_mconfig(machine_config &config)
{
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_ace2x00_device::a2bus_ace2x00_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this), m_rom(nullptr)
{
}

a2bus_ace2x00_slot1_device::a2bus_ace2x00_slot1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_ace2x00_device(mconfig, A2BUS_ACE2X00_SLOT1, tag, owner, clock)
{
}

a2bus_ace2x00_slot6_device::a2bus_ace2x00_slot6_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_ace2x00_device(mconfig, A2BUS_ACE2X00_SLOT6, tag, owner, clock),
	m_iwm(*this, "iwm"),
	m_floppy(*this, "%u", 0U)
{
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------
void a2bus_ace2x00_slot6_device::device_add_mconfig(machine_config &config)
{
	IWM(config, m_iwm, clock(), 1021800*2);
	m_iwm->phases_cb().set(FUNC(a2bus_ace2x00_slot6_device::phases_w));
	m_iwm->devsel_cb().set(FUNC(a2bus_ace2x00_slot6_device::devsel_w));
	applefdintf_device::add_525(config, m_floppy[0]);
	applefdintf_device::add_525(config, m_floppy[1]);
}

/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

uint8_t a2bus_ace2x00_slot6_device::read_c0nx(u8 offset)
{
	return m_iwm->read(offset);
}

/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_ace2x00_slot6_device::write_c0nx(u8 offset, u8 data)
{
	m_iwm->write(offset, data);
}

void a2bus_ace2x00_slot6_device::devsel_w(u8 data)
{
	if (data & 1)
		m_iwm->set_floppy(m_floppy[0]->get_device());
	else if (data & 2)
		m_iwm->set_floppy(m_floppy[1]->get_device());
	else
		m_iwm->set_floppy(nullptr);
}

void a2bus_ace2x00_slot6_device::phases_w(u8 data)
{
	auto flp = m_iwm->get_floppy();
	if (flp)
		flp->seek_phase_w(data);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_ace2x00_device::device_start()
{
}

void a2bus_ace2x00_device::device_reset()
{
	m_rom = device().machine().root_device().memregion("maincpu")->base();
}

uint8_t a2bus_ace2x00_device::read_cnxx(uint8_t offset)
{
	switch (slotno())
	{
		case 1:
			return m_rom[offset];

		case 6:
			return m_rom[offset + 0x4600];
	}

	return 0xff;
}

uint8_t a2bus_ace2x00_device::read_c800(uint16_t offset)
{
	switch (slotno())
	{
		case 6:
			return m_rom[(offset & 0x7ff) + 0x4800];
	}

	return 0xff;
}

void a2bus_ace2x00_device::write_c800(uint16_t offset, uint8_t data)
{
}

bool a2bus_ace2x00_device::take_c800()
{
	switch (slotno())
	{
		case 6:
			return true;
		default:
			return false;
	}
}
