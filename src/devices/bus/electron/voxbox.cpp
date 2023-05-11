// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Millsgrade Voxbox Speech Synthesiser

**********************************************************************/

#include "emu.h"
#include "voxbox.h"
#include "speaker.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ELECTRON_VOXBOX, electron_voxbox_device, "electron_voxbox", "Millsgrade Voxbox Speech Synthesiser")


//-------------------------------------------------
//  ROM( voxbox )
//-------------------------------------------------

ROM_START( voxbox )
	ROM_REGION(0x10000, "sp0256", 0)
	ROM_LOAD( "sp0256a-al2.bin", 0x1000, 0x0800, CRC(b504ac15) SHA1(e60fcb5fa16ff3f3b69d36c7a6e955744d3feafc) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *electron_voxbox_device::device_rom_region() const
{
	return ROM_NAME( voxbox );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void electron_voxbox_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();
	SP0256(config, m_nsp, DERIVED_CLOCK(1, 4));
	m_nsp->data_request_callback().set(FUNC(electron_voxbox_device::lrq_cb));
	m_nsp->add_route(ALL_OUTPUTS, "mono", 1.0);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  electron_voxbox_device - constructor
//-------------------------------------------------

electron_voxbox_device::electron_voxbox_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ELECTRON_VOXBOX, tag, owner, clock)
	, device_electron_expansion_interface(mconfig, *this)
	, m_nsp(*this, "sp0256")
	, m_nmi(1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void electron_voxbox_device::device_start()
{
	save_item(NAME(m_nmi));
}


//-------------------------------------------------
//  expbus_w - expansion data write
//-------------------------------------------------

void electron_voxbox_device::expbus_w(offs_t offset, uint8_t data)
{
	if ((offset & 0xff00) == 0xc000)
	{
		m_nsp->ald_w(offset & 0x3f);
	}
}

WRITE_LINE_MEMBER(electron_voxbox_device::lrq_cb)
{
	if (state != m_nmi)
	{
		m_nmi = state;
		m_slot->nmi_w(state);
	}
}
