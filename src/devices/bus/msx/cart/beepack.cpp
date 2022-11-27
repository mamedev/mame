// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***********************************************************************************

Emulation for the Hudson Soft BP-0001 Bee Pack card reader cartridge.

This cartridge allows Bee Cards (a predecessor of HuCards) to be used on an MSX system.

***********************************************************************************/

#include "emu.h"
#include "beepack.h"
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

image_init_result msx_cart_beepack_device::call_load()
{
	if (m_beecard)
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

			m_beecard->rom_alloc(length);
			u8 *rom_base = m_beecard->get_rom_base();
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

			m_beecard->rom_alloc(length);
			if (fread(m_beecard->get_rom_base(), length) != length)
			{
				seterror(image_error::UNSPECIFIED, "Unable to fully read file");
				return image_init_result::FAIL;
			}
		}

		m_beecard->initialize_cartridge();
	}
	return image_init_result::PASS;
}

std::string msx_cart_beepack_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	return software_get_default_slot("nomapper");
}
