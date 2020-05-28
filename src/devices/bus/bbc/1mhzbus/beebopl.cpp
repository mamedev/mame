// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BeebOPL FM Synthesiser emulation

    https://github.com/JudgeBeeb/BeebOPL

**********************************************************************/


#include "emu.h"
#include "beebopl.h"
#include "speaker.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_BEEBOPL, bbc_beebopl_device, "beebopl", "BeebOPL FM Synthesiser")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_beebopl_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "speaker").front_center();

	YM3812(config, m_ym3812, 3.579_MHz_XTAL);
	m_ym3812->irq_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_1mhzbus_slot_device::irq_w));
	m_ym3812->add_route(ALL_OUTPUTS, "speaker", 1.0);

	BBC_1MHZBUS_SLOT(config, m_1mhzbus, DERIVED_CLOCK(1, 1), bbc_1mhzbus_devices, nullptr);
	m_1mhzbus->irq_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_1mhzbus_slot_device::irq_w));
	m_1mhzbus->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_1mhzbus_slot_device::nmi_w));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_beebopl_device - constructor
//-------------------------------------------------

bbc_beebopl_device::bbc_beebopl_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_BEEBOPL, tag, owner, clock)
	, device_bbc_1mhzbus_interface(mconfig, *this)
	, m_1mhzbus(*this, "1mhzbus")
	, m_ym3812(*this, "ym3812")
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_beebopl_device::fred_r(offs_t offset)
{
	uint8_t data = 0xff;

	if ((offset & 0xfe) == 0x04)
	{
		data = m_ym3812->read(offset);
	}

	data &= m_1mhzbus->fred_r(offset);

	return data;
}

void bbc_beebopl_device::fred_w(offs_t offset, uint8_t data)
{
	if ((offset & 0xfe) == 0x04)
	{
		m_ym3812->write(offset, data);
	}

	m_1mhzbus->fred_w(offset, data);
}

uint8_t bbc_beebopl_device::jim_r(offs_t offset)
{
	return m_1mhzbus->jim_r(offset);
}

void bbc_beebopl_device::jim_w(offs_t offset, uint8_t data)
{
	m_1mhzbus->jim_w(offset, data);
}
