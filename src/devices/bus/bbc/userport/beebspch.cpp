// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Beeb Speech Synthesiser - Watford Electronics

    TODO:
    - verify clock and low/high intonation
    - add reset button

**********************************************************************/


#include "emu.h"
#include "beebspch.h"
#include "speaker.h"


//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define SP0256_TAG      "sp0256"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_BEEBSPCH, bbc_beebspch_device, "bbc_beebspch", "Beeb Speech Synthesiser")

ROM_START(beebspch)
	ROM_REGION(0x4000, "rom", 0)
	ROM_LOAD("watford_speech.rom", 0x0000, 0x2000, CRC(731642a8) SHA1(1bd31345af6043f394bc9d8e65180c93b2356905))
	ROM_RELOAD(0x2000, 0x2000)

	ROM_REGION(0x10000, SP0256_TAG, 0)
	ROM_LOAD("sp0256a-al2.bin", 0x1000, 0x0800, CRC(b504ac15) SHA1(e60fcb5fa16ff3f3b69d36c7a6e955744d3feafc))
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bbc_beebspch_device::device_rom_region() const
{
	return ROM_NAME(beebspch);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(bbc_beebspch_device::device_add_mconfig)
	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD(SP0256_TAG, SP0256, 14_MHz_XTAL / 4) // TODO: Crystal unknown
	MCFG_SP0256_DATA_REQUEST_CB(WRITELINE(*this, bbc_beebspch_device, cb1_w))
	MCFG_SP0256_STANDBY_CB(WRITELINE(*this, bbc_beebspch_device, cb2_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_beebspch_device - constructor
//-------------------------------------------------

bbc_beebspch_device::bbc_beebspch_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_BEEBSPCH, tag, owner, clock)
	, device_bbc_userport_interface(mconfig, *this)
	, m_rom(*this, "rom")
	, m_nsp(*this, SP0256_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_beebspch_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_beebspch_device::device_reset()
{
	machine().root_device().membank("bank4")->configure_entry(13, memregion("rom")->base());
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

WRITE8_MEMBER(bbc_beebspch_device::pb_w)
{
	switch (data & 0xc0)
	{
	case 0x40:
		// intonation high
		m_nsp->set_clock(3800000); // TODO: the exact frequency is unknown
		break;
	case 0x80:
		// intonation low
		m_nsp->set_clock(3500000); // CK / 4 ??
		break;
	}

	// allophone
	m_nsp->ald_w(space, 0, data & 0x3f);
}

WRITE_LINE_MEMBER(bbc_beebspch_device::cb1_w)
{
	m_slot->cb1_w(state);
}

WRITE_LINE_MEMBER(bbc_beebspch_device::cb2_w)
{
	m_slot->cb2_w(state);
}
