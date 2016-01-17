// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * doubler.c  --  Draysoft Doubler - external cassette interface for the 464 (works on 664/6128 with external cassette?),
 *                intended for use in duplicating cassette software
 *
 */

#include "doubler.h"
#include "includes/amstrad.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type CPC_DOUBLER = &device_creator<cpc_doubler_device>;


static MACHINE_CONFIG_FRAGMENT( cpc_doubler )
	MCFG_CASSETTE_ADD( "doubler_tape" )
	MCFG_CASSETTE_FORMATS(cdt_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED)
	MCFG_CASSETTE_INTERFACE("cpc_cass")

	// no pass-through seen on remake PCBs, unknown if actual hardware had a pass-through port or not
MACHINE_CONFIG_END


machine_config_constructor cpc_doubler_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( cpc_doubler );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

cpc_doubler_device::cpc_doubler_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, CPC_DOUBLER, "Draysoft Doubler", tag, owner, clock, "cpc_doubler", __FILE__),
	device_cpc_expansion_card_interface(mconfig, *this), m_slot(nullptr),
	m_tape(*this,"doubler_tape")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cpc_doubler_device::device_start()
{
	device_t* cpu = machine().device("maincpu");
	address_space& space = cpu->memory().space(AS_IO);
	m_slot = dynamic_cast<cpc_expansion_slot_device *>(owner());

	space.install_read_handler(0xf0e0,0xf0e0,0,0,read8_delegate(FUNC(cpc_doubler_device::ext_tape_r),this));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cpc_doubler_device::device_reset()
{
	// TODO
}

READ8_MEMBER(cpc_doubler_device::ext_tape_r)
{
	UINT8 data = 0;
	if(m_tape->input() > 0.03)
		data |= 0x20;
	return data;
}
