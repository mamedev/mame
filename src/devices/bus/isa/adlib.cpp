// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

  ISA 8 bit AdLib Music Synthesizer Card

***************************************************************************/

#include "emu.h"
#include "adlib.h"

#include "sound/spkrdev.h"
#include "speaker.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA8_ADLIB, isa8_adlib_device, "isa_adlib", "AdLib Music Synthesizer Card")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isa8_adlib_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();
	YM3812(config, m_ym3812, 14.318181_MHz_XTAL / 4); // from ISA OSC pin
	m_ym3812->add_route(ALL_OUTPUTS, "mono", 3.00);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_adlib_device - constructor
//-------------------------------------------------

isa8_adlib_device::isa8_adlib_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ISA8_ADLIB, tag, owner, clock)
	, device_isa8_card_interface(mconfig, *this)
	, m_ym3812(*this, "ym3812")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_adlib_device::device_start()
{
	set_isa_device();
	m_isa->install_device(0x0388, 0x0389, read8sm_delegate(*m_ym3812, FUNC(ym3812_device::read)), write8sm_delegate(*m_ym3812, FUNC(ym3812_device::write)));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_adlib_device::device_reset()
{
}
