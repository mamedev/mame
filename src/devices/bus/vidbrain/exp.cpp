// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    VideoBrain Expansion Port emulation

**********************************************************************/

#include "emu.h"
#include "exp.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VIDEOBRAIN_EXPANSION_SLOT, videobrain_expansion_slot_device, "videobrain_expansion_slot", "VideoBrain expansion port")



//**************************************************************************
//  DEVICE VIDEOBRAIN_EXPANSION CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_videobrain_expansion_card_interface - constructor
//-------------------------------------------------

device_videobrain_expansion_card_interface::device_videobrain_expansion_card_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "vidbrainexp"),
	m_rom_mask(0),
	m_ram_mask(0)
{
	m_slot = dynamic_cast<videobrain_expansion_slot_device *>(device.owner());
}


//-------------------------------------------------
//  videobrain_rom_pointer - get ROM pointer
//-------------------------------------------------

uint8_t* device_videobrain_expansion_card_interface::videobrain_rom_pointer(running_machine &machine, size_t size)
{
	if (m_rom.empty())
	{
		m_rom.resize(size);
		m_rom_mask = size - 1;
	}

	return &m_rom[0];
}


//-------------------------------------------------
//  videobrain_ram_pointer - get RAM pointer
//-------------------------------------------------

uint8_t* device_videobrain_expansion_card_interface::videobrain_ram_pointer(running_machine &machine, size_t size)
{
	if (m_ram.empty())
	{
		m_ram.resize(size);
		m_ram_mask = size - 1;
	}

	return &m_ram[0];
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  videobrain_expansion_slot_device - constructor
//-------------------------------------------------

videobrain_expansion_slot_device::videobrain_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VIDEOBRAIN_EXPANSION_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_videobrain_expansion_card_interface>(mconfig, *this),
	device_cartrom_image_interface(mconfig, *this),
	m_write_extres(*this), m_cart(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void videobrain_expansion_slot_device::device_start()
{
	m_cart = get_card_device();
}


//-------------------------------------------------
//  call_load -
//-------------------------------------------------

std::pair<std::error_condition, std::string> videobrain_expansion_slot_device::call_load()
{
	if (m_cart)
	{
		if (!loaded_through_softlist())
		{
			size_t const romsize = length();

			fread(m_cart->videobrain_rom_pointer(machine(), romsize), romsize);
		}
		else
		{
			size_t const romsize = get_software_region_length("rom");
			if (romsize)
				memcpy(m_cart->videobrain_rom_pointer(machine(), romsize), get_software_region("rom"), romsize);

			size_t const ramsize = get_software_region_length("ram");
			if (ramsize)
				memset(m_cart->videobrain_ram_pointer(machine(), ramsize), 0, ramsize);
		}
	}

	return std::make_pair(std::error_condition(), std::string());
}


//-------------------------------------------------
//  get_default_card_software -
//-------------------------------------------------

std::string videobrain_expansion_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	return software_get_default_slot("standard");
}


//-------------------------------------------------
//  SLOT_INTERFACE_START( vidbrain_expansion_cards )
//-------------------------------------------------

// slot devices
#include "std.h"
#include "comp_language.h"
#include "info_manager.h"
#include "money_minder.h"
#include "timeshare.h"

void vidbrain_expansion_cards(device_slot_interface &device)
{
	device.option_add_internal("standard", VB_STD);
	device.option_add_internal("comp_language", VB_COMP_LANGUAGE);
	device.option_add_internal("info_manager", VB_INFO_MANAGER);
	device.option_add_internal("money_minder", VB_MONEY_MINDER);
	device.option_add_internal("timeshare", VB_TIMESHARE);
}
