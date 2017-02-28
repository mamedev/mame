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

const device_type A2BUS_DX1 = device_creator<a2bus_dx1_device>;

MACHINE_CONFIG_FRAGMENT( a2dx1 )
	MCFG_SPEAKER_STANDARD_MONO("speaker")
	MCFG_SOUND_ADD("dac", DAC_8BIT_R2R, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.5) // unknown DAC
	MCFG_SOUND_ADD("dacvol", DAC_8BIT_R2R, 0) // unknown DAC
	MCFG_SOUND_ROUTE_EX(0, "dac", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE_EX(0, "dac", -1.0, DAC_VREF_NEG_INPUT)
	MCFG_DEVICE_ADD("vref", VOLTAGE_REGULATOR, 0) MCFG_VOLTAGE_REGULATOR_OUTPUT(5.0)
	MCFG_SOUND_ROUTE_EX(0, "dacvol", 1.0, DAC_VREF_POS_INPUT)
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

a2bus_dx1_device::a2bus_dx1_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_a2bus_card_interface(mconfig, *this),
	m_dac(*this, "dac"),
	m_dacvol(*this, "dacvol")
{
}

a2bus_dx1_device::a2bus_dx1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, A2BUS_DX1, "Decillonix DX-1", tag, owner, clock, "a2dx1", __FILE__),
	device_a2bus_card_interface(mconfig, *this),
	m_dac(*this, "dac"),
	m_dacvol(*this, "dacvol")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_dx1_device::device_start()
{
	// set_a2bus_device makes m_slot valid
	set_a2bus_device();
}

uint8_t a2bus_dx1_device::read_c0nx(address_space &space, uint8_t offset)
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

void a2bus_dx1_device::write_c0nx(address_space &space, uint8_t offset, uint8_t data)
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
