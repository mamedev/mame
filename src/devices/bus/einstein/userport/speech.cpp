// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Einstein Speech Synthesiser (J&K Software)

    TODO: Verify implementation

***************************************************************************/

#include "emu.h"
#include "speech.h"
#include "speaker.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(EINSTEIN_SPEECH, einstein_speech_device, "einstein_speech", "Einstein Speech Synthesiser")

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( sp0256 )
	ROM_REGION(0x10000, "sp0256", 0)
	ROM_LOAD("sp0256a-al2.bin", 0x1000, 0x0800, CRC(b504ac15) SHA1(e60fcb5fa16ff3f3b69d36c7a6e955744d3feafc))
ROM_END

const tiny_rom_entry *einstein_speech_device::device_rom_region() const
{
	return ROM_NAME( sp0256 );
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void einstein_speech_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();
	SP0256(config, m_sp0256, 3120000); // ???
	m_sp0256->add_route(ALL_OUTPUTS, "mono", 1.00);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  einstein_speech_device - constructor
//-------------------------------------------------

einstein_speech_device::einstein_speech_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, EINSTEIN_SPEECH, tag, owner, clock),
	device_einstein_userport_interface(mconfig, *this),
	m_sp0256(*this, "sp0256")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void einstein_speech_device::device_start()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

// 7-------  reset?
// -6------  sp0256 lrq
// --543210  sp0256 ald

uint8_t einstein_speech_device::read()
{
	return m_sp0256->lrq_r() ? 0x00 : 0x40;
}

void einstein_speech_device::write(uint8_t data)
{
	if (BIT(data, 7) == 0)
		m_sp0256->ald_w(data & 0x3f);
}
