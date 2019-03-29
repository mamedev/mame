// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BeebSID emulation

**********************************************************************/


#include "emu.h"
#include "beebsid.h"
#include "speaker.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_BEEBSID, bbc_beebsid_device, "beebsid", "BeebSID")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_beebsid_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "speaker").front_center();
	MOS8580(config, m_sid, DERIVED_CLOCK(1, 1));
	m_sid->add_route(ALL_OUTPUTS, "speaker", 1.0);

	BBC_1MHZBUS_SLOT(config, m_1mhzbus, DERIVED_CLOCK(1, 1), bbc_1mhzbus_devices, nullptr);
	m_1mhzbus->irq_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_1mhzbus_slot_device::irq_w));
	m_1mhzbus->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_1mhzbus_slot_device::nmi_w));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_beebsid_device - constructor
//-------------------------------------------------

bbc_beebsid_device::bbc_beebsid_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, BBC_BEEBSID, tag, owner, clock),
	device_bbc_1mhzbus_interface(mconfig, *this),
	m_1mhzbus(*this, "1mhzbus"),
	m_sid(*this, "mos8580")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_beebsid_device::device_start()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_beebsid_device::fred_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (offset >= 0x20 && offset < 0x40)
	{
		data = m_sid->read(offset);
	}

	data &= m_1mhzbus->fred_r(offset);

	return data;
}

void bbc_beebsid_device::fred_w(offs_t offset, uint8_t data)
{
	if (offset >= 0x20 && offset < 0x40)
	{
		m_sid->write(offset, data);
	}

	m_1mhzbus->fred_w(offset, data);
}

uint8_t bbc_beebsid_device::jim_r(offs_t offset)
{
	uint8_t data = 0xff;

	data &= m_1mhzbus->jim_r(offset);

	return data;
}

void bbc_beebsid_device::jim_w(offs_t offset, uint8_t data)
{
	m_1mhzbus->jim_w(offset, data);
}
