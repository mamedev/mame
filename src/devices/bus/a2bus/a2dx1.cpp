// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2dx1.c

    Implementation of the Decillionix DX-1 sampler card

*********************************************************************/

#include "a2dx1.h"
#include "includes/apple2.h"
#include "sound/dac.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A2BUS_DX1 = &device_creator<a2bus_dx1_device>;

#define DAC_TAG         "dac"

MACHINE_CONFIG_FRAGMENT( a2dx1 )
	MCFG_SPEAKER_STANDARD_MONO("dx1spkr")
	MCFG_SOUND_ADD(DAC_TAG, DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "dx1spkr", 1.00)
MACHINE_CONFIG_END

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor a2bus_dx1_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( a2dx1 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_dx1_device::a2bus_dx1_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_a2bus_card_interface(mconfig, *this),
	m_dac(*this, DAC_TAG), m_volume(0), m_lastdac(0)
{
}

a2bus_dx1_device::a2bus_dx1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, A2BUS_DX1, "Decillonix DX-1", tag, owner, clock, "a2dx1", __FILE__),
	device_a2bus_card_interface(mconfig, *this),
	m_dac(*this, DAC_TAG), m_volume(0), m_lastdac(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_dx1_device::device_start()
{
	// set_a2bus_device makes m_slot valid
	set_a2bus_device();

	save_item(NAME(m_volume));
	save_item(NAME(m_lastdac));
}

void a2bus_dx1_device::device_reset()
{
	m_volume = m_lastdac = 0;
}

UINT8 a2bus_dx1_device::read_c0nx(address_space &space, UINT8 offset)
{
	switch (offset)
	{
		case 1: // ADC input
			return 0;

		case 3: // busy flag
			return 0x80;    // indicate not busy

		case 7: // 1-bit ADC input (bit 7 of c0n1, probably)
			return 0;
	}

	return 0xff;
}

void a2bus_dx1_device::write_c0nx(address_space &space, UINT8 offset, UINT8 data)
{
	switch (offset)
	{
		case 5: // volume
			m_volume = data;
			m_dac->write_unsigned16(data*m_lastdac);
			break;

		case 6:
			m_lastdac = data;
			m_dac->write_unsigned16(data*m_volume);
			break;
	}
}

bool a2bus_dx1_device::take_c800()
{
	return false;
}
