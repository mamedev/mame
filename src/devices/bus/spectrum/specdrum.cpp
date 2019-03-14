// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Cheetah Marketing SpecDrum emulation

**********************************************************************/

#include "emu.h"
#include "specdrum.h"
#include "sound/volt_reg.h"
#include "speaker.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SPECTRUM_SPECDRUM, spectrum_specdrum_device, "spectrum_specdrum", "SpecDrum")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void spectrum_specdrum_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "speaker").front_center();
	ZN428E(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.5);
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spectrum_specdrum_device - constructor
//-------------------------------------------------

spectrum_specdrum_device::spectrum_specdrum_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPECTRUM_SPECDRUM, tag, owner, clock)
	, device_spectrum_expansion_interface(mconfig, *this)
	, m_dac(*this, "dac")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spectrum_specdrum_device::device_start()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void spectrum_specdrum_device::iorq_w(offs_t offset, uint8_t data)
{
	switch (offset & 0x00ff)
	{
	case 0xdf:
		m_dac->write(data);
		break;
	}
}
