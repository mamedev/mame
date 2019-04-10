// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * doubler.cpp  --  Draysoft Doubler - external cassette interface for the 464 (works on 664/6128 with external cassette?),
 *                intended for use in duplicating cassette software
 *
 */

#include "emu.h"
#include "doubler.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CPC_DOUBLER, cpc_doubler_device, "cpc_doubler", "Draysoft Doubler")


void cpc_doubler_device::device_add_mconfig(machine_config &config)
{
	CASSETTE(config, m_tape);
	m_tape->set_formats(cdt_cassette_formats);
	m_tape->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED);
	m_tape->set_interface("cpc_cass");

	// no pass-through seen on remake PCBs, unknown if actual hardware had a pass-through port or not
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

cpc_doubler_device::cpc_doubler_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CPC_DOUBLER, tag, owner, clock),
	device_cpc_expansion_card_interface(mconfig, *this), m_slot(nullptr),
	m_tape(*this,"doubler_tape")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cpc_doubler_device::device_start()
{
	m_slot = dynamic_cast<cpc_expansion_slot_device *>(owner());
	address_space &space = m_slot->cpu().space(AS_IO);
	space.install_read_handler(0xf0e0,0xf0e0,read8_delegate(FUNC(cpc_doubler_device::ext_tape_r),this));
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
	uint8_t data = 0;
	if(m_tape->input() > 0.03)
		data |= 0x20;
	return data;
}
