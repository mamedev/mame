// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * cpc_pds.cpp  --  CPC interface hardware for the Programmers Development System
 *
 *  Created on: 10/02/2014
 */

#include "emu.h"
#include "cpc_pds.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CPC_PDS, cpc_pds_device, "cpc_pds", "Programmers Development System (CPC Target)")


void cpc_pds_device::device_add_mconfig(machine_config &config)
{
	Z80PIO(config, m_pio, DERIVED_CLOCK(1, 1));   // no clock on the PCB, so will presume that it uses the CPC's clock

	// no pass-through seen on remake PCBs, unknown if actual hardware had a pass-through port or not
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

cpc_pds_device::cpc_pds_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CPC_PDS, tag, owner, clock),
	device_cpc_expansion_card_interface(mconfig, *this),
	m_slot(nullptr),
	m_pio(*this, "pio")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cpc_pds_device::device_start()
{
	m_slot = dynamic_cast<cpc_expansion_slot_device *>(owner());
	address_space &space = m_slot->cpu().space(AS_IO);

	space.install_readwrite_handler(0xfbec,0xfbef, read8_delegate(*this, FUNC(cpc_pds_device::pio_r)), write8_delegate(*this, FUNC(cpc_pds_device::pio_w)));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cpc_pds_device::device_reset()
{
	// TODO
}


READ8_MEMBER(cpc_pds_device::pio_r)
{
	return m_pio->read(offset);
}

WRITE8_MEMBER(cpc_pds_device::pio_w)
{
	m_pio->write(offset,data);
}
