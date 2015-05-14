// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2echoii.c

    Implementation of the Street Electronics Echo II speech card

*********************************************************************/

#include "a2echoii.h"
#include "includes/apple2.h"
#include "sound/tms5220.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A2BUS_ECHOII = &device_creator<a2bus_echoii_device>;

#define TMS_TAG         "tms5220"

MACHINE_CONFIG_FRAGMENT( a2echoii )
	MCFG_SPEAKER_STANDARD_MONO("echoii")
	MCFG_SOUND_ADD(TMS_TAG, TMS5220, 640000) // Note the Echo II card has a "FREQ" potentiometer which can be used to adjust the tms5220's clock frequency; 640khz is the '8khz' value according to the tms5220 datasheet
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "echoii", 1.0)
MACHINE_CONFIG_END

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor a2bus_echoii_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( a2echoii );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_echoii_device::a2bus_echoii_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_a2bus_card_interface(mconfig, *this),
	m_tms(*this, TMS_TAG)
{
}

a2bus_echoii_device::a2bus_echoii_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, A2BUS_ECHOII, "Street Electronics Echo II", tag, owner, clock, "a2echoii", __FILE__),
	device_a2bus_card_interface(mconfig, *this),
	m_tms(*this, TMS_TAG)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_echoii_device::device_start()
{
	// set_a2bus_device makes m_slot valid
	set_a2bus_device();
}

void a2bus_echoii_device::device_reset()
{
}

UINT8 a2bus_echoii_device::read_c0nx(address_space &space, UINT8 offset)
{
	switch (offset)
	{
		case 0:
			return 0x1f | m_tms->status_r(space, 0);
	}

	return 0;
}

void a2bus_echoii_device::write_c0nx(address_space &space, UINT8 offset, UINT8 data)
{
	switch (offset)
	{
		case 0:
			m_tms->data_w(space, offset, data);
			break;
	}
}

bool a2bus_echoii_device::take_c800()
{
	return false;
}
