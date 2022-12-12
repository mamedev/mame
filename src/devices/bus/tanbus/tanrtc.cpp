// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Microtanic Real Time Clock

**********************************************************************/


#include "emu.h"
#include "tanrtc.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(TANBUS_TANRTC, tanbus_tanrtc_device, "tanbus_tanrtc", "Microtanic Real Time Clock")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void tanbus_tanrtc_device::device_add_mconfig(machine_config &config)
{
	MC146818(config, m_rtc, 32.768_kHz_XTAL);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tanbus_tanrtc_device - constructor
//-------------------------------------------------

tanbus_tanrtc_device::tanbus_tanrtc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TANBUS_TANRTC, tag, owner, clock)
	, device_tanbus_interface(mconfig, *this)
	, m_rtc(*this, "rtc")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tanbus_tanrtc_device::device_start()
{
}


//-------------------------------------------------
//  read - card read
//-------------------------------------------------

uint8_t tanbus_tanrtc_device::read(offs_t offset, int inhrom, int inhram, int be)
{
	uint8_t data = 0xff;

	switch (offset & 0xffc0)
	{
	case 0xbc00:
		data = m_rtc->read_direct(offset);
		break;
	}

	return data;
}


//-------------------------------------------------
//  write - card write
//-------------------------------------------------

void tanbus_tanrtc_device::write(offs_t offset, uint8_t data, int inhrom, int inhram, int be)
{
	switch (offset & 0xffc0)
	{
	case 0xbc00:
		m_rtc->write_direct(offset, data);
		break;
	}
}
