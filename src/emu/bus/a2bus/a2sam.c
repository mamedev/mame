// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2sam.c

    Implementation of the S.A.M. "Software Automated Mouth" card

*********************************************************************/

#include "a2sam.h"
#include "includes/apple2.h"
#include "sound/dac.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A2BUS_SAM = &device_creator<a2bus_sam_device>;

#define DAC_TAG         "dac"

MACHINE_CONFIG_FRAGMENT( a2sam )
	MCFG_SPEAKER_STANDARD_MONO("samspkr")
	MCFG_SOUND_ADD(DAC_TAG, DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "samspkr", 1.00)
MACHINE_CONFIG_END

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor a2bus_sam_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( a2sam );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_sam_device::a2bus_sam_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_a2bus_card_interface(mconfig, *this),
	m_dac(*this, DAC_TAG)
{
}

a2bus_sam_device::a2bus_sam_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, A2BUS_SAM, "Don't Ask Software SAM", tag, owner, clock, "a2sam", __FILE__),
	device_a2bus_card_interface(mconfig, *this),
	m_dac(*this, DAC_TAG)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_sam_device::device_start()
{
	// set_a2bus_device makes m_slot valid
	set_a2bus_device();
}

void a2bus_sam_device::device_reset()
{
}

void a2bus_sam_device::write_c0nx(address_space &space, UINT8 offset, UINT8 data)
{
	m_dac->write_unsigned8(data);
}

bool a2bus_sam_device::take_c800()
{
	return false;
}
