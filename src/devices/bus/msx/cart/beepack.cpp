// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***********************************************************************************

Emulation for the Hudson Soft BP-0001 Bee Pack card reader cartridge.

This cartridge allows Bee Cards (a predecessor of HuCards) to be used on an MSX system.

***********************************************************************************/

#include "emu.h"
#include "beepack.h"
#include "bus/msx/beecard/beecard.h"
#include "softlist_dev.h"


DEFINE_DEVICE_TYPE(MSX_CART_BEEPACK, msx_cart_beepack_device, "msx_cart_beepack", "Hudson Soft BP-0001 Bee Pack card reader")


msx_cart_beepack_device::msx_cart_beepack_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSX_CART_BEEPACK, tag, owner, clock)
	, device_cartrom_image_interface(mconfig, *this)
	, device_single_card_slot_interface<bee_card_interface>(mconfig, *this)
	, msx_cart_interface(mconfig, *this)
{
}

void msx_cart_beepack_device::device_resolve_objects()
{
	m_beecard = get_card_device();
	if (m_beecard)
	{
		m_beecard->set_views(page(0), page(1), page(2), page(3));
	}
}

void msx_cart_beepack_device::device_add_mconfig(machine_config &config)
{
	bee_card(*this);
	SOFTWARE_LIST(config, "bee_card_list").set_original("msx1_bee_card");
}

std::pair<std::error_condition, std::string> msx_cart_beepack_device::call_load()
{
	if (m_beecard)
	{
		if (loaded_through_softlist())
		{
			u32 const length = get_software_region_length("rom");
			// Only 16KB or 32KB images are supported
			if (length != 0x4000 && length != 0x8000)
				return std::make_pair(image_error::BADSOFTWARE, "Invalid file size for a bee card (must be 16K or 32K)");
		}
		else
		{
			u32 const length = this->length();
			// Only 16KB or 32KB images are supported
			if (length != 0x4000 && length != 0x8000)
				return std::make_pair(image_error::BADSOFTWARE, "Invalid file size for a bee card (must be 16K or 32K)");

			memory_region *const romregion = machine().memory().region_alloc(subtag("rom"), length, 1, ENDIANNESS_LITTLE);
			if (fread(romregion->base(), length) != length)
				return std::make_pair(image_error::UNSPECIFIED, "Unable to fully read file");
		}

		std::string message;
		std::error_condition result = m_beecard->initialize_cartridge(message);
		return std::make_pair(result, message);
	}
	return std::make_pair(std::error_condition(), std::string());
}

std::string msx_cart_beepack_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	return software_get_default_slot("nomapper");
}



bee_card_interface::bee_card_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "beecard")
	, m_page{nullptr, nullptr, nullptr, nullptr}
	, m_slot(dynamic_cast<msx_cart_beepack_device *>(device.owner()))
{
}

void bee_card_interface::set_views(memory_view::memory_view_entry *page0, memory_view::memory_view_entry *page1, memory_view::memory_view_entry *page2, memory_view::memory_view_entry *page3)
{
	m_page[0] = page0;
	m_page[1] = page1;
	m_page[2] = page2;
	m_page[3] = page3;
}
