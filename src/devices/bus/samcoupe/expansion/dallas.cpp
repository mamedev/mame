// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Dallas Clock for SAM Coupe

***************************************************************************/

#include "emu.h"
#include "dallas.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SAM_DALLAS_CLOCK, sam_dallas_clock_device, "sam_dallas_clock", "Dallas Clock")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void sam_dallas_clock_device::device_add_mconfig(machine_config &config)
{
	DS12885(config, m_rtc, 32.768_kHz_XTAL); // should be DS12887 or DS1287
	m_rtc->set_24hrs(true);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sam_dallas_clock_device - constructor
//-------------------------------------------------

sam_dallas_clock_device::sam_dallas_clock_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SAM_DALLAS_CLOCK, tag, owner, clock),
	device_samcoupe_expansion_interface(mconfig, *this),
	m_rtc(*this, "rtc")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sam_dallas_clock_device::device_start()
{
	// register for savestates
	save_item(NAME(m_print));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void sam_dallas_clock_device::print_w(int state)
{
	m_print = state;
}

uint8_t sam_dallas_clock_device::iorq_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_print && (offset & 0xfe07) == 0xfe07)
		data &= m_rtc->read((offset >> 8) & 0x01);

	return data;
}

void sam_dallas_clock_device::iorq_w(offs_t offset, uint8_t data)
{
	if (m_print && (offset & 0xfe07) == 0xfe07)
		m_rtc->write((offset >> 8) & 0x01, data);
}
