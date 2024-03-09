// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Portfolio Memory Card Port emulation

**********************************************************************/

#include "emu.h"
#include "ccm.h"

#include "softlist_dev.h"

#include <tuple>



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
	device_interface(device, "pofoccm")
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
	device_single_card_slot_interface<device_portfolio_memory_card_slot_interface>(mconfig, *this),
	device_memcard_image_interface(mconfig, *this),
	m_card(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void portfolio_memory_card_slot_device::device_start()
{
	m_card = get_card_device();
}


//-------------------------------------------------
//  call_load -
//-------------------------------------------------

std::pair<std::error_condition, std::string> portfolio_memory_card_slot_device::call_load()
{
	std::error_condition err;

	if (m_card)
	{
		if (!loaded_through_softlist())
		{
			size_t const size = length();
			size_t actual;
			std::tie(err, m_card->m_rom, actual) = read(image_core_file(), size);
			if (!err && (actual != size))
				err = std::errc::io_error;
		}
		else
		{
			load_software_region("rom", m_card->m_rom);
		}
	}

	return std::make_pair(err, std::string());
}


//-------------------------------------------------
//  get_software_list_loader -
//-------------------------------------------------

const software_list_loader &portfolio_memory_card_slot_device::get_software_list_loader() const
{
	return rom_software_list_loader::instance();
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
