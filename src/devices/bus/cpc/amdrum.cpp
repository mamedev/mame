// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * amdrum.cpp
 *
 *  Created on: 23/08/2014
 */

#include "emu.h"
#include "amdrum.h"

#include "sound/volt_reg.h"
#include "speaker.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CPC_AMDRUM, cpc_amdrum_device, "cpc_amdrum", "Amdrum")


void cpc_amdrum_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "speaker").front_center();
	ZN428E(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.5);
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);
	// no pass-through
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

cpc_amdrum_device::cpc_amdrum_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CPC_AMDRUM, tag, owner, clock),
	device_cpc_expansion_card_interface(mconfig, *this),
	m_slot(nullptr),
	m_dac(*this,"dac")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cpc_amdrum_device::device_start()
{
	m_slot = dynamic_cast<cpc_expansion_slot_device *>(owner());
	address_space &space = m_slot->cpu().space(AS_IO);
	space.install_write_handler(0xff00,0xffff,write8_delegate(FUNC(cpc_amdrum_device::dac_w),this));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cpc_amdrum_device::device_reset()
{
	// TODO
}

WRITE8_MEMBER(cpc_amdrum_device::dac_w)
{
	m_dac->write(data);
}
