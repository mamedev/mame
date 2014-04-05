/*
 * cpcexp.c  --  Amstrad CPC Expansion port
 *
 *  Created on: 16/07/2011
 *
 */


#include "emu.h"
#include "emuopts.h"
#include "cpcexp.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type CPC_EXPANSION_SLOT = &device_creator<cpc_expansion_slot_device>;


//**************************************************************************
//  DEVICE CPC_EXPANSION CARD INTERFACE
//**************************************************************************


device_cpc_expansion_card_interface::device_cpc_expansion_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig,device)
{
}


device_cpc_expansion_card_interface::~device_cpc_expansion_card_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

cpc_expansion_slot_device::cpc_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, CPC_EXPANSION_SLOT, "Amstrad CPC expansion port", tag, owner, clock, "cpc_expansion_slot", __FILE__),
		device_slot_interface(mconfig, *this)
{
}

cpc_expansion_slot_device::~cpc_expansion_slot_device()
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void cpc_expansion_slot_device::device_config_complete()
{
	// inherit a copy of the static data
	const cpc_expansion_slot_interface *intf = reinterpret_cast<const cpc_expansion_slot_interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<cpc_expansion_slot_interface *>(this) = *intf;
	}

	// or initialize to defaults if none provided
	else
	{
		memset(&m_out_irq_cb, 0, sizeof(m_out_irq_cb));
		memset(&m_out_nmi_cb, 0, sizeof(m_out_nmi_cb));
		memset(&m_out_reset_cb, 0, sizeof(m_out_reset_cb));
		memset(&m_out_romdis_cb, 0, sizeof(m_out_romdis_cb));
		memset(&m_out_romen_cb, 0, sizeof(m_out_romen_cb));
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cpc_expansion_slot_device::device_start()
{
	m_card = dynamic_cast<device_cpc_expansion_card_interface *>(get_card_device());

	// resolve callbacks
	m_out_irq_func.resolve(m_out_irq_cb, *this);
	m_out_nmi_func.resolve(m_out_nmi_cb, *this);
	m_out_reset_func.resolve(m_out_reset_cb, *this);
	m_out_romdis_func.resolve(m_out_romdis_cb, *this);
	m_out_romen_func.resolve(m_out_romen_cb, *this);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cpc_expansion_slot_device::device_reset()
{
}


WRITE_LINE_MEMBER( cpc_expansion_slot_device::irq_w ) { m_out_irq_func(state); }
WRITE_LINE_MEMBER( cpc_expansion_slot_device::nmi_w ) { m_out_nmi_func(state); }
WRITE_LINE_MEMBER( cpc_expansion_slot_device::reset_w ) { m_out_reset_func(state); }
WRITE_LINE_MEMBER( cpc_expansion_slot_device::romdis_w ) { m_out_romdis_func(state); }
WRITE_LINE_MEMBER( cpc_expansion_slot_device::romen_w ) { m_out_romen_func(state); }
