// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * amdrum.c
 *
 *  Created on: 23/08/2014
 */

#include "emu.h"
#include "amdrum.h"
#include "includes/amstrad.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type CPC_AMDRUM = &device_creator<cpc_amdrum_device>;


static MACHINE_CONFIG_FRAGMENT( cpc_amdrum )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
	// no pass-through
MACHINE_CONFIG_END

machine_config_constructor cpc_amdrum_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( cpc_amdrum );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

cpc_amdrum_device::cpc_amdrum_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, CPC_AMDRUM, "Amdrum", tag, owner, clock, "cpc_amdrum", __FILE__),
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
	device_t* cpu = machine().device("maincpu");
	address_space& space = cpu->memory().space(AS_IO);
	m_slot = dynamic_cast<cpc_expansion_slot_device *>(owner());

	space.install_write_handler(0xff00,0xffff,0,0,write8_delegate(FUNC(cpc_amdrum_device::dac_w),this));
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
	m_dac->write_unsigned8(data);
}
