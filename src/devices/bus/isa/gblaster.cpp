// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

  ISA 8 bit Creative Labs Game Blaster Sound Card

***************************************************************************/

#include "emu.h"
#include "gblaster.h"

#include "sound/spkrdev.h"
#include "speaker.h"

/*
  creative labs game blaster (CMS creative music system)
  2 x saa1099 chips
  also on sound blaster 1.0
  option on sound blaster 1.5

  jumperable? normally 0x220
*/


uint8_t isa8_gblaster_device::saa1099_16_r(offs_t offset)
{
	return 0xff;
}

void isa8_gblaster_device::saa1099_1_16_w(offs_t offset, uint8_t data)
{
	m_saa1099_1->write(offset, data);
}

void isa8_gblaster_device::saa1099_2_16_w(offs_t offset, uint8_t data)
{
	m_saa1099_2->write(offset, data);
}

uint8_t isa8_gblaster_device::detect_r(offs_t offset)
{
	switch(offset)
	{
		case 0:
		case 1: return 0x7f; break;  // this register reportedly returns 0x3f on a Tandy 1000 TL, and 0x7f on a generic 486 PC.
		case 6:
		case 7: return detect_reg; break;
		default: return 0xff;
	}
}

void isa8_gblaster_device::detect_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		case 2:
		case 3: detect_reg = (data & 0xff); break;
	}
}

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA8_GAME_BLASTER, isa8_gblaster_device, "isa_gblaster", "Game Blaster Sound Card")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isa8_gblaster_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "speaker", 2).front();
	SAA1099(config, m_saa1099_1, XTAL(14'318'181) / 2); // or CMS-301, from OSC pin in ISA bus
	m_saa1099_1->add_route(0, "speaker", 0.50, 0);
	m_saa1099_1->add_route(1, "speaker", 0.50, 1);
	SAA1099(config, m_saa1099_2, XTAL(14'318'181) / 2); // or CMS-301, from OSC pin in ISA bus
	m_saa1099_2->add_route(0, "speaker", 0.50, 0);
	m_saa1099_2->add_route(1, "speaker", 0.50, 1);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_gblaster_device - constructor
//-------------------------------------------------

isa8_gblaster_device::isa8_gblaster_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA8_GAME_BLASTER, tag, owner, clock),
	device_isa8_card_interface(mconfig, *this),
	m_saa1099_1(*this, "saa1099.1"),
	m_saa1099_2(*this, "saa1099.2"),
	detect_reg(0xFF)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_gblaster_device::device_start()
{
	set_isa_device();
	m_isa->install_device(0x0220, 0x0221, read8sm_delegate(*this, FUNC(isa8_gblaster_device::saa1099_16_r)), write8sm_delegate(*this, FUNC(isa8_gblaster_device::saa1099_1_16_w)));
	m_isa->install_device(0x0222, 0x0223, read8sm_delegate(*this, FUNC(isa8_gblaster_device::saa1099_16_r)), write8sm_delegate(*this, FUNC(isa8_gblaster_device::saa1099_2_16_w)));
	m_isa->install_device(0x0224, 0x022F, read8sm_delegate(*this, FUNC(isa8_gblaster_device::detect_r)), write8sm_delegate(*this, FUNC(isa8_gblaster_device::detect_w)));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_gblaster_device::device_reset()
{
}
