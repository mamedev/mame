// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Voicebox for SAM Coupe

***************************************************************************/

#include "emu.h"
#include "voicebox.h"
#include "speaker.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SAM_VOICEBOX, sam_voicebox_device, "voicebox", "Voicebox")

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( sp0256 )
	ROM_REGION(0x10000, "sp0256", 0)
	ROM_LOAD("sp0256a-al2.bin", 0x1000, 0x0800, CRC(b504ac15) SHA1(e60fcb5fa16ff3f3b69d36c7a6e955744d3feafc))
ROM_END

const tiny_rom_entry *sam_voicebox_device::device_rom_region() const
{
	return ROM_NAME( sp0256 );
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void sam_voicebox_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();
	SP0256(config, m_sp0256, 3000000); // ???
	m_sp0256->add_route(ALL_OUTPUTS, "mono", 1.00);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sam_voicebox_device - constructor
//-------------------------------------------------

sam_voicebox_device::sam_voicebox_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SAM_VOICEBOX, tag, owner, clock),
	device_samcoupe_expansion_interface(mconfig, *this),
	m_sp0256(*this, "sp0256")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sam_voicebox_device::device_start()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t sam_voicebox_device::iorq_r(offs_t offset)
{
	uint8_t data = 0xff;

	if ((offset & 0xff) == 0x7f)
		data = 0x18 | m_sp0256->lrq_r();

	return data;
}

void sam_voicebox_device::iorq_w(offs_t offset, uint8_t data)
{
	if ((offset & 0xff) == 0x7f)
		m_sp0256->ald_w(data);
}
