// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Microtan Bulldog Sound Generator Board

    http://www.microtan.ukpc.net/pageProducts.html#SOUND

**********************************************************************/


#include "emu.h"
#include "bullsnd.h"
#include "speaker.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(TANBUS_BULLSND, tanbus_bullsnd_device, "tanbus_bullsnd", "Microtan Bulldog Sound Generator Board")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void tanbus_bullsnd_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "speaker").front_center();
	AY8910(config, m_ay8910[0], DERIVED_CLOCK(1, 8)).add_route(ALL_OUTPUTS, "speaker", 0.5);
	AY8910(config, m_ay8910[1], DERIVED_CLOCK(1, 8)).add_route(ALL_OUTPUTS, "speaker", 0.5);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tanbus_bullsnd_device - constructor
//-------------------------------------------------

tanbus_bullsnd_device::tanbus_bullsnd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TANBUS_BULLSND, tag, owner, clock)
	, device_tanbus_interface(mconfig, *this)
	, m_ay8910(*this, "ay8910%u", 0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tanbus_bullsnd_device::device_start()
{
}


//-------------------------------------------------
//  read - card read
//-------------------------------------------------

uint8_t tanbus_bullsnd_device::read(offs_t offset, int inhrom, int inhram, int be)
{
	uint8_t data = 0xff;

	switch (offset)
	{
	case 0xbc01:
		data = m_ay8910[0]->data_r();
		break;
	case 0xbc03:
		data = m_ay8910[1]->data_r();
		break;
	}

	return data;
}


//-------------------------------------------------
//  write - card write
//-------------------------------------------------

void tanbus_bullsnd_device::write(offs_t offset, uint8_t data, int inhrom, int inhram, int be)
{
	switch (offset)
	{
	case 0xbc00:
		m_ay8910[0]->address_w(data);
		break;
	case 0xbc01:
		m_ay8910[0]->data_w(data);
		break;
	case 0xbc02:
		m_ay8910[1]->address_w(data);
		break;
	case 0xbc03:
		m_ay8910[1]->data_w(data);
		break;
	}
}
