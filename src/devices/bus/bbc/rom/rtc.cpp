// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Solidisk Real Time Clock

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Solidisk_RTC.html

    PMS Genie Watch (RTC for the BBC)

***************************************************************************/

#include "emu.h"
#include "rtc.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_STLRTC, bbc_stlrtc_device, "bbc_stlrtc", "Solidisk Real Time Clock")
DEFINE_DEVICE_TYPE(BBC_PMSRTC, bbc_pmsrtc_device, "bbc_pmsrtc", "PMS Genie Real Time Clock")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_stlrtc_device::device_add_mconfig(machine_config &config)
{
	MC146818(config, m_rtc, 32.768_kHz_XTAL); // TODO: verify clock
}

void bbc_pmsrtc_device::device_add_mconfig(machine_config &config)
{
	/* Dallas DS1216 SmartWatch ROM */
	DS1216E(config, m_rtc);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_stlrtc_device - constructor
//-------------------------------------------------

bbc_stlrtc_device::bbc_stlrtc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_STLRTC, tag, owner, clock)
	, device_bbc_rom_interface(mconfig, *this)
	, m_rtc(*this, "rtc")
{
}

bbc_pmsrtc_device::bbc_pmsrtc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_PMSRTC, tag, owner, clock)
	, device_bbc_rom_interface(mconfig, *this)
	, m_rtc(*this, "rtc")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_stlrtc_device::device_start()
{
}

void bbc_pmsrtc_device::device_start()
{
}

//-------------------------------------------------
//  read
//-------------------------------------------------

uint8_t bbc_stlrtc_device::read(offs_t offset)
{
	uint8_t data = get_rom_base()[offset & 0x3fff];

	switch (offset & 0x3fc0)
	{
	case 0x3e00:
		data = m_rtc->data_r();
		break;
	case 0x3e40:
		if (!machine().side_effects_disabled())
			m_rtc->address_w(data);
		break;
	case 0x3e80:
	case 0x3ec0:
		data = m_rtc->get_address(); // FIXME: really?
		break;
	case 0x3f00:
	case 0x3f40:
	case 0x3f80:
	case 0x3fc0:
		if (!machine().side_effects_disabled())
			m_rtc->data_w(data);
		break;
	}
	return data;
}

uint8_t bbc_pmsrtc_device::read(offs_t offset)
{
	uint8_t data = get_rom_base()[offset & 0x1fff];

	if (m_rtc->ceo_r())
		data = m_rtc->read(offset);
	else
		m_rtc->read(offset);

	return data;
}
