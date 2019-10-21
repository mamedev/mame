// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * x68kexp.c
 */

#include "emu.h"
#include "x68kexp.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(X68K_EXPANSION_SLOT, x68k_expansion_slot_device, "x68k_expansion_slot", "Sharp X680x0 expansion slot")


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

uint8_t device_x68k_expansion_card_interface::iack2()
{
	device().logerror("Failed to acknowledge IRQ2\n");
	return 0x18; // spurious interrupt
}

uint8_t device_x68k_expansion_card_interface::iack4()
{
	device().logerror("Failed to acknowledge IRQ4\n");
	return 0x18; // spurious interrupt
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

x68k_expansion_slot_device::x68k_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, X68K_EXPANSION_SLOT, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_space(*this, finder_base::DUMMY_TAG, -1),
	m_out_irq2_cb(*this),
	m_out_irq4_cb(*this),
	m_out_nmi_cb(*this),
	m_out_reset_cb(*this),
	m_card(nullptr)
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
