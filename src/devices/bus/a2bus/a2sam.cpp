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

DEFINE_DEVICE_TYPE(A2BUS_SAM, a2bus_sam_device, "a2sam", "Don't Ask Software S.A.M.")

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(a2bus_sam_device::device_add_mconfig)
	SPEAKER(config, "speaker").front_center();
	MCFG_DEVICE_ADD("dac", DAC_8BIT_R2R, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.5) // unknown DAC
	MCFG_DEVICE_ADD("vref", VOLTAGE_REGULATOR, 0) MCFG_VOLTAGE_REGULATOR_OUTPUT(5.0)
	MCFG_SOUND_ROUTE(0, "dac", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE(0, "dac", -1.0, DAC_VREF_NEG_INPUT)
MACHINE_CONFIG_END

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
}

void a2bus_sam_device::device_reset()
{
}

void a2bus_sam_device::write_c0nx(uint8_t offset, uint8_t data)
{
	m_dac->write(data);
}

bool a2bus_sam_device::take_c800()
{
	return false;
}
