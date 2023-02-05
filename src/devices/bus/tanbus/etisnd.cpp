// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    ETI Sound Card

    http://www.microtan.ukpc.net/pageProducts.html#SOUND

**********************************************************************/


#include "emu.h"
#include "etisnd.h"
#include "speaker.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(TANBUS_ETISND, tanbus_etisnd_device, "tanbus_etisnd", "Microtan ETI Sound Card")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void tanbus_etisnd_device::device_add_mconfig(machine_config &config)
{
	PIA6821(config, m_pia[0], 0);
	m_pia[0]->writepa_handler().set("dac1", FUNC(dac_byte_interface::data_w));
	m_pia[0]->writepb_handler().set("dac2", FUNC(dac_byte_interface::data_w));

	PIA6821(config, m_pia[1], 0);
	m_pia[1]->writepa_handler().set("dac3", FUNC(dac_byte_interface::data_w));
	m_pia[1]->writepb_handler().set("dac4", FUNC(dac_byte_interface::data_w));

	PIA6821(config, m_pia[2], 0);
	m_pia[2]->writepa_handler().set("dac5", FUNC(dac_byte_interface::data_w));
	m_pia[2]->writepb_handler().set("dac6", FUNC(dac_byte_interface::data_w));

	PIA6821(config, m_pia[3], 0);
	m_pia[2]->writepb_handler().set(FUNC(tanbus_etisnd_device::pia_pb_w));

	SPEAKER(config, "speaker").front_center();
	DAC0800(config, "dac1", 0).add_route(ALL_OUTPUTS, "speaker", 1.0);
	DAC0800(config, "dac2", 0).add_route(ALL_OUTPUTS, "speaker", 1.0);
	DAC0800(config, "dac3", 0).add_route(ALL_OUTPUTS, "speaker", 1.0);
	DAC0800(config, "dac4", 0).add_route(ALL_OUTPUTS, "speaker", 1.0);
	DAC0800(config, "dac5", 0).add_route(ALL_OUTPUTS, "speaker", 1.0);
	DAC0800(config, "dac6", 0).add_route(ALL_OUTPUTS, "speaker", 1.0);

	AY8910(config, m_ay8910, DERIVED_CLOCK(1, 4)).add_route(ALL_OUTPUTS, "speaker", 0.5);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tanbus_etisnd_device - constructor
//-------------------------------------------------

tanbus_etisnd_device::tanbus_etisnd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TANBUS_ETISND, tag, owner, clock)
	, device_tanbus_interface(mconfig, *this)
	, m_pia(*this, "pia%u", 0)
	, m_ay8910(*this, "ay8910")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tanbus_etisnd_device::device_start()
{
}


//-------------------------------------------------
//  read - card read
//-------------------------------------------------

uint8_t tanbus_etisnd_device::read(offs_t offset, int inhrom, int inhram, int be)
{
	uint8_t data = 0xff;

	switch (offset & 0xfffc)
	{
	case 0xbc00:
		data = m_pia[0]->read(offset & 3);
		break;
	case 0xbc04:
		data = m_pia[1]->read(offset & 3);
		break;
	case 0xbc08:
		data = m_pia[2]->read(offset & 3);
		break;
	case 0xbc0c:
		data = m_pia[3]->read(offset & 3);
		break;
	}

	return data;
}


//-------------------------------------------------
//  write - card write
//-------------------------------------------------

void tanbus_etisnd_device::write(offs_t offset, uint8_t data, int inhrom, int inhram, int be)
{
	switch (offset & 0xfffc)
	{
	case 0xbc00:
		m_pia[0]->write(offset & 3, data);
		break;
	case 0xbc04:
		m_pia[1]->write(offset & 3, data);
		break;
	case 0xbc08:
		m_pia[2]->write(offset & 3, data);
		break;
	case 0xbc0c:
		m_pia[3]->write(offset & 3, data);
		break;
	}
}

void tanbus_etisnd_device::pia_pb_w(uint8_t data)
{
	// PB0 -> BC1
	// PB1 -> BDIR
	// +5v -> BC2
	switch (data & 3)
	{
	case 0:
		m_pia[3]->porta_w(0xff);
		break;
	case 1:
		m_pia[3]->porta_w(m_ay8910->data_r());
		break;
	case 2:
		m_ay8910->data_w(m_pia[3]->a_output());
		break;
	case 3:
		m_ay8910->address_w(m_pia[3]->a_output());
		break;
	}
}
