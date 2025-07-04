// license:BSD-3-Clause
// copyright-holders:Nigel Barnes,Thomas Busse
/**********************************************************************

    Spectrum Currah MicroSpeech emulation

**********************************************************************/

#include "emu.h"
#include "uspeech.h"
#include "speaker.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SPECTRUM_USPEECH, spectrum_uspeech_device, "spectrum_uspeech", u8"Spectrum Currah µSpeech")


//-------------------------------------------------
//  ROM( uspeech )
//-------------------------------------------------

ROM_START( uspeech )
	ROM_REGION(0x0800, "rom", 0)
	ROM_LOAD("currah.rom", 0x0000, 0x0800, CRC(ce7cf52e) SHA1(90dbba5afbf07949df9cbdcb0a8ec0b106340422))

	ROM_REGION(0x10000, "sp0256", 0)
	ROM_LOAD( "sp0256a-al2.bin", 0x1000, 0x0800, CRC(b504ac15) SHA1(e60fcb5fa16ff3f3b69d36c7a6e955744d3feafc) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *spectrum_uspeech_device::device_rom_region() const
{
	return ROM_NAME( uspeech );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void spectrum_uspeech_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();
	SP0256(config, m_nsp, 14_MHz_XTAL / 4);
	m_nsp->add_route(ALL_OUTPUTS, "mono", 1.0);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spectrum_uspeech_device - constructor
//-------------------------------------------------

spectrum_uspeech_device::spectrum_uspeech_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SPECTRUM_USPEECH, tag, owner, clock),
	device_spectrum_expansion_interface(mconfig, *this),
	m_nsp(*this, "sp0256"),
	m_rom(*this, "rom")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spectrum_uspeech_device::device_start()
{
	save_item(NAME(m_romcs));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spectrum_uspeech_device::device_reset()
{
	m_romcs = 0;
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

bool spectrum_uspeech_device::romcs()
{
	return m_romcs;
}

void spectrum_uspeech_device::pre_opcode_fetch(offs_t offset)
{
	if (!machine().side_effects_disabled() && (offset == 0x0038))
	{
		m_romcs = !m_romcs;
	}
}

uint8_t spectrum_uspeech_device::iorq_r(offs_t offset)
{
	if (!machine().side_effects_disabled() && (offset == 0x0038))
	{
		m_romcs = !m_romcs;
	}

	return offset & 1 ? m_slot->fb_r() : 0xff;
}

uint8_t spectrum_uspeech_device::mreq_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_romcs)
	{
		switch (offset & 0xf000)
		{
		case 0x0000:
			data = m_rom->base()[offset & 0x7ff];
			break;
		case 0x1000:
			data = !m_nsp->lrq_r();
			break;
		}
	}

	return data;
}

void spectrum_uspeech_device::mreq_w(offs_t offset, uint8_t data)
{
	if (m_romcs)
	{
		switch (offset & 0xf001)
		{
		case 0x1000:
		case 0x1001:
			// allophone
			m_nsp->ald_w(data & 0x3f);
			break;

		case 0x3000:
			// intonation low
			m_nsp->set_clock(3050000); // oscillator frequency read from hardware
			break;

		case 0x3001:
			// intonation high
			m_nsp->set_clock(3260000); // oscillator frequency read from hardware
			break;
		}
	}
}
