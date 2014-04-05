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
		device_slot_interface(mconfig, *this)
{
}

x68k_expansion_slot_device::~x68k_expansion_slot_device()
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void x68k_expansion_slot_device::device_config_complete()
{
	// inherit a copy of the static data
	const x68k_expansion_slot_interface *intf = reinterpret_cast<const x68k_expansion_slot_interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<x68k_expansion_slot_interface *>(this) = *intf;
	}

	// or initialize to defaults if none provided
	else
	{
		memset(&m_out_irq2_cb, 0, sizeof(m_out_irq2_cb));
		memset(&m_out_irq4_cb, 0, sizeof(m_out_irq4_cb));
		memset(&m_out_nmi_cb, 0, sizeof(m_out_nmi_cb));
		memset(&m_out_reset_cb, 0, sizeof(m_out_reset_cb));
	}
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void x68k_expansion_slot_device::device_start()
{
	m_card = dynamic_cast<device_x68k_expansion_card_interface *>(get_card_device());

	// resolve callbacks
	m_out_irq2_func.resolve(m_out_irq2_cb, *this);
	m_out_irq4_func.resolve(m_out_irq4_cb, *this);
	m_out_nmi_func.resolve(m_out_nmi_cb, *this);
	m_out_reset_func.resolve(m_out_reset_cb, *this);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void x68k_expansion_slot_device::device_reset()
{
}


WRITE_LINE_MEMBER( x68k_expansion_slot_device::irq2_w ) { m_out_irq2_func(state); }
WRITE_LINE_MEMBER( x68k_expansion_slot_device::irq4_w ) { m_out_irq4_func(state); }
WRITE_LINE_MEMBER( x68k_expansion_slot_device::nmi_w ) { m_out_nmi_func(state); }
WRITE_LINE_MEMBER( x68k_expansion_slot_device::reset_w ) { m_out_reset_func(state); }
