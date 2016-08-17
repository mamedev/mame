// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Portfolio Memory Card Port emulation

**********************************************************************/

#include "ccm.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type PORTFOLIO_MEMORY_CARD_SLOT = &device_creator<portfolio_memory_card_slot_t>;



//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_portfolio_memory_card_slot_interface - constructor
//-------------------------------------------------

device_portfolio_memory_card_slot_interface::device_portfolio_memory_card_slot_interface(const machine_config &mconfig, device_t &device) :
	device_slot_card_interface(mconfig, device),
	m_rom(*this, "rom"),
	m_nvram(*this, "nvram")
{
	m_slot = dynamic_cast<portfolio_memory_card_slot_t *>(device.owner());
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  portfolio_memory_card_slot_t - constructor
//-------------------------------------------------

portfolio_memory_card_slot_t::portfolio_memory_card_slot_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, PORTFOLIO_MEMORY_CARD_SLOT, "Atari Portfolio memory card port", tag, owner, clock, "portfolio_ccm_slot", __FILE__),
	device_slot_interface(mconfig, *this),
	device_image_interface(mconfig, *this),
	m_card(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void portfolio_memory_card_slot_t::device_start()
{
	m_card = dynamic_cast<device_portfolio_memory_card_slot_interface *>(get_card_device());
}


//-------------------------------------------------
//  call_load -
//-------------------------------------------------

image_init_result portfolio_memory_card_slot_t::call_load()
{
	if (m_card)
	{
		if (software_entry() == nullptr)
		{
			fread(m_card->m_rom, length());
		}
		else
		{
			load_software_region("rom", m_card->m_rom);
		}
	}

	return image_init_result::PASS;
}


//-------------------------------------------------
//  get_default_card_software -
//-------------------------------------------------

std::string portfolio_memory_card_slot_t::get_default_card_software()
{
	return software_get_default_slot("rom");
}


//-------------------------------------------------
//  SLOT_INTERFACE( portfolio_memory_cards )
//-------------------------------------------------

// slot devices
#include "ram.h"
#include "rom.h"

SLOT_INTERFACE_START( portfolio_memory_cards )
	SLOT_INTERFACE("ram", PORTFOLIO_RAM_CARD)
	SLOT_INTERFACE("rom", PORTFOLIO_ROM_CARD)
SLOT_INTERFACE_END
