// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***********************************************************************************

Emulation for the Electric Software Astron SoftCard Adaptor cartridge.

This cartridge allows SoftCards to be used on an MSX system.

***********************************************************************************/

#include "emu.h"
#include "softcard.h"
#include "bus/msx/softcard/softcard.h"
#include "softlist_dev.h"


DEFINE_DEVICE_TYPE(MSX_CART_SOFTCARD, msx_cart_softcard_device, "msx_cart_softcard", "Electric Softward Astron SoftCard Adaptor")


msx_cart_softcard_device::msx_cart_softcard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSX_CART_SOFTCARD, tag, owner, clock)
	, device_cartrom_image_interface(mconfig, *this)
	, device_single_card_slot_interface<softcard_interface>(mconfig, *this)
	, msx_cart_interface(mconfig, *this)
{
}

void msx_cart_softcard_device::device_resolve_objects()
{
	m_softcard = get_card_device();
	if (m_softcard)
	{
		m_softcard->set_views(page(0), page(1), page(2), page(3));
	}
}

void msx_cart_softcard_device::device_add_mconfig(machine_config &config)
{
	softcard(*this);
	SOFTWARE_LIST(config, "softcard_list").set_original("msx_softcard");
}

image_init_result msx_cart_softcard_device::call_load()
{
	if (m_softcard)
	{
		if (loaded_through_softlist())
		{
			u32 length = get_software_region_length("rom");
			// Only 32KB images are supported
			if (length != 0x8000)
			{
				seterror(image_error::UNSPECIFIED, "Invalid file size for a softcard");
				return image_init_result::FAIL;
			}
		}
		else
		{
			u32 length = this->length();
			// Only 32KB images are supported
			if (length != 0x8000)
			{
				seterror(image_error::UNSPECIFIED, "Invalid file size for a softcard");
				return image_init_result::FAIL;
			}

			memory_region *const romregion = machine().memory().region_alloc(subtag("rom"), length, 1, ENDIANNESS_LITTLE);
			if (fread(romregion->base(), length) != length)
			{
				seterror(image_error::UNSPECIFIED, "Unable to fully read file");
				return image_init_result::FAIL;
			}
		}

		std::string message;
		image_init_result result = m_softcard->initialize_cartridge(message);
		if (image_init_result::PASS != result)
			seterror(image_error::INVALIDIMAGE, message.c_str());

		return result;
	}
	return image_init_result::PASS;
}

std::string msx_cart_softcard_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	return software_get_default_slot("nomapper");
}


softcard_interface::softcard_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "softcard")
	, m_page{nullptr, nullptr, nullptr, nullptr}
	, m_slot(dynamic_cast<msx_cart_softcard_device *>(device.owner()))
{
}

void softcard_interface::set_views(memory_view::memory_view_entry *page0, memory_view::memory_view_entry *page1, memory_view::memory_view_entry *page2, memory_view::memory_view_entry *page3)
{
	m_page[0] = page0;
	m_page[1] = page1;
	m_page[2] = page2;
	m_page[3] = page3;
}
