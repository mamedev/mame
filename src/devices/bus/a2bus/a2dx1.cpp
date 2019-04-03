// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2dx1.c

    Implementation of the Decillionix DX-1 sampler card

*********************************************************************/

#include "emu.h"
#include "a2dx1.h"
#include "sound/volt_reg.h"
#include "speaker.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A2BUS_DX1, a2bus_dx1_device, "a2dx1", "Decillonix DX-1")

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_dx1_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "speaker").front_center();
	DAC_8BIT_R2R(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.5); // unknown DAC
	DAC_8BIT_R2R(config, m_dacvol, 0).add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT).add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT); // unknown DAC
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dacvol", 1.0, DAC_VREF_POS_INPUT);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_dx1_device::a2bus_dx1_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_dac(*this, "dac"),
	m_dacvol(*this, "dacvol")
{
}

a2bus_dx1_device::a2bus_dx1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_dx1_device(mconfig, A2BUS_DX1, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_dx1_device::device_start()
{
}

uint8_t a2bus_dx1_device::read_c0nx(uint8_t offset)
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

void a2bus_dx1_device::write_c0nx(uint8_t offset, uint8_t data)
{
	switch (offset)
	{
		case 5:
			m_dacvol->write(data);
			break;

		case 6:
			m_dac->write(data);
			break;
	}
}

bool a2bus_dx1_device::take_c800()
{
	return false;
}
