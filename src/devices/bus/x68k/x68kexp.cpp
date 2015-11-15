// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * x68kexp.c
 */

#include "emu.h"
#include "emuopts.h"
#include "x68kexp.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type X68K_EXPANSION_SLOT = &device_creator<x68k_expansion_slot_device>;


//**************************************************************************
//  DEVICE CPC_EXPANSION CARD INTERFACE
//**************************************************************************


device_x68k_expansion_card_interface::device_x68k_expansion_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
}


device_x68k_expansion_card_interface::~device_x68k_expansion_card_interface()
{
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

x68k_expansion_slot_device::x68k_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, X68K_EXPANSION_SLOT, "Sharp X680x0 expansion slot", tag, owner, clock, "x68k_expansion_slot", __FILE__),
		device_slot_interface(mconfig, *this),
		m_out_irq2_cb(*this),
		m_out_irq4_cb(*this),
		m_out_nmi_cb(*this),
		m_out_reset_cb(*this), m_card(nullptr)
{
}

x68k_expansion_slot_device::~x68k_expansion_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void x68k_expansion_slot_device::device_start()
{
	m_card = dynamic_cast<device_x68k_expansion_card_interface *>(get_card_device());

	// resolve callbacks
	m_out_irq2_cb.resolve_safe();
	m_out_irq4_cb.resolve_safe();
	m_out_nmi_cb.resolve_safe();
	m_out_reset_cb.resolve_safe();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void x68k_expansion_slot_device::device_reset()
{
}


WRITE_LINE_MEMBER( x68k_expansion_slot_device::irq2_w ) { m_out_irq2_cb(state); }
WRITE_LINE_MEMBER( x68k_expansion_slot_device::irq4_w ) { m_out_irq4_cb(state); }
WRITE_LINE_MEMBER( x68k_expansion_slot_device::nmi_w ) { m_out_nmi_cb(state); }
WRITE_LINE_MEMBER( x68k_expansion_slot_device::reset_w ) { m_out_reset_cb(state); }
