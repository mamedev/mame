// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Robin Voice Box - The Educational Software Company

**********************************************************************/


#include "emu.h"
#include "voicebox.h"
#include "speaker.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_VOICEBOX, bbc_voicebox_device, "bbc_voicebox", "Robin Voice Box")


ROM_START(voicebox)
	ROM_REGION(0x10000, "sp0256", 0)
	ROM_LOAD("sp0256a-al2.bin", 0x1000, 0x0800, CRC(b504ac15) SHA1(e60fcb5fa16ff3f3b69d36c7a6e955744d3feafc))
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bbc_voicebox_device::device_rom_region() const
{
	return ROM_NAME(voicebox);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_voicebox_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();
	SP0256(config, m_nsp, 3120000); // TODO: unknown crystal
	m_nsp->data_request_callback().set(FUNC(bbc_voicebox_device::cb1_w));
	m_nsp->standby_callback().set(FUNC(bbc_voicebox_device::cb2_w));
	m_nsp->add_route(ALL_OUTPUTS, "mono", 1.0);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_voicebox_device - constructor
//-------------------------------------------------

bbc_voicebox_device::bbc_voicebox_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_VOICEBOX, tag, owner, clock)
	, device_bbc_userport_interface(mconfig, *this)
	, m_nsp(*this, "sp0256")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_voicebox_device::device_start()
{
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void bbc_voicebox_device::pb_w(uint8_t data)
{
	m_nsp->ald_w(data & 0x3f);
}

WRITE_LINE_MEMBER(bbc_voicebox_device::cb1_w)
{
	m_slot->cb1_w(state);
}

WRITE_LINE_MEMBER(bbc_voicebox_device::cb2_w)
{
	m_slot->cb2_w(state);
}
