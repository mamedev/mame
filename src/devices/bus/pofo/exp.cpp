// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Portfolio Expansion Port emulation

**********************************************************************/

#include "emu.h"
#include "exp.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(PORTFOLIO_EXPANSION_SLOT, portfolio_expansion_slot_device, "portfolio_expansion_slot", "Atari Portfolio expansion port")



//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_portfolio_expansion_slot_interface - constructor
//-------------------------------------------------

device_portfolio_expansion_slot_interface::device_portfolio_expansion_slot_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "pofoexp")
{
	m_slot = dynamic_cast<portfolio_expansion_slot_device *>(device.owner());
}

WRITE_LINE_MEMBER( device_portfolio_expansion_slot_interface::eint_w ) { m_slot->eint_w(state); }
WRITE_LINE_MEMBER( device_portfolio_expansion_slot_interface::nmio_w ) { m_slot->nmio_w(state); }
WRITE_LINE_MEMBER( device_portfolio_expansion_slot_interface::wake_w ) { m_slot->wake_w(state); }



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  portfolio_expansion_slot_device - constructor
//-------------------------------------------------

portfolio_expansion_slot_device::portfolio_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PORTFOLIO_EXPANSION_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_portfolio_expansion_slot_interface>(mconfig, *this),
	m_write_eint(*this),
	m_write_nmio(*this),
	m_write_wake(*this),
	m_card(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void portfolio_expansion_slot_device::device_start()
{
	m_card = get_card_device();

	// resolve callbacks
	m_write_eint.resolve_safe();
	m_write_nmio.resolve_safe();
	m_write_wake.resolve_safe();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void portfolio_expansion_slot_device::device_reset()
{
	if (m_card != nullptr)
	{
		m_card->device().reset();
	}
}



//-------------------------------------------------
//  SLOT_INTERFACE( portfolio_expansion_cards )
//-------------------------------------------------

// slot devices
#include "hpc101.h"
#include "hpc102.h"
#include "hpc104.h"

void portfolio_expansion_cards(device_slot_interface &device)
{
	device.option_add("lpt",  POFO_HPC101);
	device.option_add("uart", POFO_HPC102);
	device.option_add("ram",  POFO_HPC104);
	device.option_add("ram2", POFO_HPC104_2);
}
