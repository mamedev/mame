// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***********************************************************************************************************

 Sega AI card emulation

 ***********************************************************************************************************/


#include "emu.h"
#include "segaai_slot.h"


DEFINE_DEVICE_TYPE(SEGAAI_CARD_SLOT, segaai_card_slot_device, "segaai_card_slot", "Sega AI Card Slot")


static char const *const ROM_128 = "rom_128";
static char const *const ROM_256 = "rom_256";


segaai_card_interface::segaai_card_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "segaai_card")
	, m_slot(dynamic_cast<segaai_card_slot_device *>(device.owner()))
{
}

segaai_card_interface::~segaai_card_interface()
{
}



segaai_card_slot_device::segaai_card_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SEGAAI_CARD_SLOT, tag, owner, clock)
	, device_cartrom_image_interface(mconfig, *this)
	, device_single_card_slot_interface<segaai_card_interface>(mconfig, *this)
	, m_address_space(*this, finder_base::DUMMY_TAG, -1, 8)
{
}

segaai_card_slot_device::~segaai_card_slot_device()
{
}

void segaai_card_slot_device::device_start()
{
	m_cart = get_card_device();
}

std::pair<std::error_condition, std::string> segaai_card_slot_device::call_load()
{
	if (m_cart)
	{
		const u32 length = !loaded_through_softlist() ? this->length() : get_software_region_length("rom");

		if (length != 0x20000 && length != 0x40000)
		{
			return std::make_pair(image_error::INVALIDIMAGE, "Invalid card size. Supported sizes are: 128KB, 256KB");
		}

		if (!loaded_through_softlist())
		{
			memory_region *const romregion = machine().memory().region_alloc(subtag("rom"), length, 1, ENDIANNESS_LITTLE);
			if (fread(romregion->base(), length) != length)
				return std::make_pair(image_error::UNSPECIFIED, "Unable to fully read file");
		}

		m_cart->install_memory_handlers(m_address_space.target());

		return std::make_pair(std::error_condition(), std::string());
	}

	return std::make_pair(std::error_condition(), std::string());
}

std::string segaai_card_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	if (hook.image_file())
	{
		uint64_t length;
		hook.image_file()->length(length);

		return std::string(length == 0x40000 ? ROM_256 : ROM_128);
	}

	return software_get_default_slot(ROM_128);
}



// slot interfaces
#include "rom.h"


void segaai_cards(device_slot_interface &device)
{
	device.option_add_internal(ROM_128, SEGAAI_ROM_128);
	device.option_add_internal(ROM_256, SEGAAI_ROM_256);
}
