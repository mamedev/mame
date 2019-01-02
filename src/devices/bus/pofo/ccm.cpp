// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Portfolio Memory Card Port emulation

**********************************************************************/

#include "emu.h"
#include "ccm.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(PORTFOLIO_MEMORY_CARD_SLOT, portfolio_memory_card_slot_device, "portfolio_ccm_slot", "Atari Portfolio memory card port")



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
	m_slot = dynamic_cast<portfolio_memory_card_slot_device *>(device.owner());
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  portfolio_memory_card_slot_device - constructor
//-------------------------------------------------

portfolio_memory_card_slot_device::portfolio_memory_card_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PORTFOLIO_MEMORY_CARD_SLOT, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	device_image_interface(mconfig, *this),
	m_card(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void portfolio_memory_card_slot_device::device_start()
{
	m_card = dynamic_cast<device_portfolio_memory_card_slot_interface *>(get_card_device());
}


//-------------------------------------------------
//  call_load -
//-------------------------------------------------

image_init_result portfolio_memory_card_slot_device::call_load()
{
	if (m_card)
	{
		if (!loaded_through_softlist())
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

std::string portfolio_memory_card_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	return software_get_default_slot("rom");
}


//-------------------------------------------------
//  SLOT_INTERFACE( portfolio_memory_cards )
//-------------------------------------------------

// slot devices
#include "ram.h"
#include "rom.h"

void portfolio_memory_cards(device_slot_interface &device)
{
	device.option_add("ram", PORTFOLIO_RAM_CARD);
	device.option_add("rom", PORTFOLIO_ROM_CARD);
}
