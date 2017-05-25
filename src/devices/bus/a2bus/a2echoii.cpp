// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2echoii.c

    Implementation of the Street Electronics Echo II speech card

*********************************************************************/

#include "emu.h"
#include "a2echoii.h"
#include "sound/tms5220.h"
#include "speaker.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A2BUS_ECHOII, a2bus_echoii_device, "a2echoii", "Street Electronics Echo II")

#define TMS_TAG         "tms5220"

MACHINE_CONFIG_START( a2echoii )
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

a2bus_echoii_device::a2bus_echoii_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_tms(*this, TMS_TAG)
{
}

a2bus_echoii_device::a2bus_echoii_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_echoii_device(mconfig, A2BUS_ECHOII, tag, owner, clock)
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

uint8_t a2bus_echoii_device::read_c0nx(address_space &space, uint8_t offset)
{
	switch (offset)
	{
		case 0:
			return 0x1f | m_tms->status_r(space, 0);
	}

	return 0;
}

void a2bus_echoii_device::write_c0nx(address_space &space, uint8_t offset, uint8_t data)
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
