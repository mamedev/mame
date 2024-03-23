// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    ETI Real Time Clock/Calendar

    http://www.microtan.ukpc.net/pageProducts.html#CLOCK

**********************************************************************/


#include "emu.h"
#include "etirtc.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(TANBUS_ETIRTC, tanbus_etirtc_device, "tanbus_etirtc", "Microtan ETI Real Time Clock")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void tanbus_etirtc_device::device_add_mconfig(machine_config &config)
{
	MM58174(config, "rtc", 32.768_kHz_XTAL);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tanbus_etirtc_device - constructor
//-------------------------------------------------

tanbus_etirtc_device::tanbus_etirtc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TANBUS_ETIRTC, tag, owner, clock)
	, device_tanbus_interface(mconfig, *this)
	, m_rtc(*this, "rtc")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tanbus_etirtc_device::device_start()
{
}


//-------------------------------------------------
//  read - card read
//-------------------------------------------------

uint8_t tanbus_etirtc_device::read(offs_t offset, int inhrom, int inhram, int be)
{
	uint8_t data = 0xff;

	switch (offset & 0xfff0)
	{
	case 0xbc00:
		data = m_rtc->read(offset);
		break;
	}

	return data;
}


//-------------------------------------------------
//  write - card write
//-------------------------------------------------

void tanbus_etirtc_device::write(offs_t offset, uint8_t data, int inhrom, int inhram, int be)
{
	switch (offset & 0xfff0)
	{
	case 0xbc00:
		m_rtc->write(offset, data);
		break;
	}
}
