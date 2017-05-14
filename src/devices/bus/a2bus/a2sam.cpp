// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2sam.c

    Implementation of the S.A.M. "Software Automated Mouth" card

*********************************************************************/

#include "emu.h"
#include "a2sam.h"
#include "sound/volt_reg.h"
#include "speaker.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A2BUS_SAM, a2bus_sam_device, "a2sam", "Don't Ask Software SAM")

MACHINE_CONFIG_FRAGMENT( a2sam )
	MCFG_SPEAKER_STANDARD_MONO("speaker")
	MCFG_SOUND_ADD("dac", DAC_8BIT_R2R, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.5) // unknown DAC
	MCFG_DEVICE_ADD("vref", VOLTAGE_REGULATOR, 0) MCFG_VOLTAGE_REGULATOR_OUTPUT(5.0)
	MCFG_SOUND_ROUTE_EX(0, "dac", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE_EX(0, "dac", -1.0, DAC_VREF_NEG_INPUT)
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

a2bus_sam_device::a2bus_sam_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, A2BUS_SAM, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_dac(*this, "dac")
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

void a2bus_sam_device::write_c0nx(address_space &space, uint8_t offset, uint8_t data)
{
	m_dac->write(data);
}

bool a2bus_sam_device::take_c800()
{
	return false;
}
