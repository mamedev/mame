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

void a2bus_sam_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "speaker").front_center();
	DAC_8BIT_R2R(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.5); // unknown DAC
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);
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
