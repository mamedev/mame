// license:BSD-3-Clause
// copyright-holders:TwistedTom
/*********************************************************************

    MICRO-POKEer

    (Micro-Studio, Hungary 1988-1989)

    A back-up/snapshot type device similar to multiface.
    Saves full ram or screen to tape (normal or high speed)
    Cheat/poke and jump functions.
    No pass-through slot.

    Operation:
    Press freeze button...
    H:      on-screen help
    space:  exit help
    enter:  return
    r:      total ram save
    R:      turbo total ram save
    s:      screen save
    S:      turbo screen save
    l:      turbo screen load
    L:      turbo total ram load
    w:      warm reset
    m:      micro-monitor (poke or jump)

    micro-monitor:
    black entry box appears...
    move around with q/a/o/p, enter
    input address (dec)
    to jump: j, enter
    to poke: p, input data (dec), enter  (repeats)
    to quit: q, enter

    normal saves are useable independant of the interface,
    "turbo" saves need the interface present, must use l or L options to load.

    Lots of info including schematic and rom disassembly:
    https://bitbandit.org/20141231/disassembling-the-micro-pokeer-rom-firmware/


*********************************************************************/

#include "emu.h"
#include "mpoker.h"


/***************************************************************************
    DEVICE DEFINITIONS
***************************************************************************/

DEFINE_DEVICE_TYPE(SPECTRUM_MPOKER, spectrum_mpoker_device, "spectrum_mpoker", "MICRO-POKEer")


//-------------------------------------------------
//  INPUT_PORTS( mpoker )
//-------------------------------------------------

INPUT_PORTS_START( mpoker )
	PORT_START("BUTTON")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Freeze Button") PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(spectrum_mpoker_device::freeze_button), 0)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor spectrum_mpoker_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(mpoker);
}

//-------------------------------------------------
//  ROM( mpoker )
//-------------------------------------------------

ROM_START(mpoker)
	ROM_REGION(0x2000, "rom", 0)  // v1.6
	ROM_LOAD("mpoker.rom", 0x0000, 0x2000, CRC(11927434) SHA1(fd918519e4bd0eb8220d157a1c3f314669764657))
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

const tiny_rom_entry *spectrum_mpoker_device::device_rom_region() const
{
	return ROM_NAME(mpoker);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spectrum_mpoker_device - constructor
//-------------------------------------------------

spectrum_mpoker_device::spectrum_mpoker_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPECTRUM_MPOKER, tag, owner, clock)
	, device_spectrum_expansion_interface(mconfig, *this)
	, m_rom(*this, "rom")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spectrum_mpoker_device::device_start()
{
	save_item(NAME(m_romcs));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spectrum_mpoker_device::device_reset()
{
	m_romcs = 0;
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

bool spectrum_mpoker_device::romcs()
{
	return m_romcs;
}

void spectrum_mpoker_device::iorq_w(offs_t offset, uint8_t data)
{
	if ((offset & 0x80) == 0)  // 0--- ----  uses 7f
	{
		m_romcs = BIT(data, 0);
	}
}

uint8_t spectrum_mpoker_device::mreq_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_romcs)
	{
		data = m_rom->base()[offset & 0x1fff];
	}

	return data;
}

INPUT_CHANGED_MEMBER(spectrum_mpoker_device::freeze_button)
{
	if (newval)
	{
		m_slot->nmi_w(CLEAR_LINE);
	}
	else
	{
		m_romcs = 1;
		m_slot->nmi_w(ASSERT_LINE);
	}
}
