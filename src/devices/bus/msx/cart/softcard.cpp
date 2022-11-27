// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***********************************************************************************

Emulation for the Electric Software Astron SoftCard Adaptor cartridge.

This cartridge allows SoftCards to be used on an MSX system.

***********************************************************************************/

#include "emu.h"
#include "softcard.h"
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
			// Allocate and copy rom contents
			u32 length = get_software_region_length("rom");
			// Only 16KB or 32KB images are supported
			if (length != 0x4000 && length != 0x8000)
			{
				seterror(image_error::UNSPECIFIED, "Invalid file size for a bee card");
				return image_init_result::FAIL;
			}

			m_softcard->rom_alloc(length);
			u8 *rom_base = m_softcard->get_rom_base();
			memcpy(rom_base, get_software_region("rom"), length);
		}
		else
		{
			u32 length = this->length();
			// Only 16KB or 32KB images are supported
			if (length != 0x4000 && length != 0x8000)
			{
				seterror(image_error::UNSPECIFIED, "Invalid file size for a bee card");
				return image_init_result::FAIL;
			}

			m_softcard->rom_alloc(length);
			if (fread(m_softcard->get_rom_base(), length) != length)
			{
				seterror(image_error::UNSPECIFIED, "Unable to fully read file");
				return image_init_result::FAIL;
			}
		}

		m_softcard->initialize_cartridge();
	}
	return image_init_result::PASS;
}

std::string msx_cart_softcard_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	return software_get_default_slot("nomapper");
}
