// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    Nintendo DS game card slot
	Based on gba_slot by Ryan Holtz and Fabio Priuli

***************************************************************************/

#include "emu.h"
#include "ndsslot.h"

DEFINE_DEVICE_TYPE(NDS_CART_SLOT, nds_cart_slot_device, "nds_cart_slot", "Nintendo DS Game Card Slot")

//**************************************************************************
//  device_nds_cart_interface
//**************************************************************************

device_nds_cart_interface::device_nds_cart_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "ndscart"),
	m_rom(nullptr),
	m_rom_size(0),
	m_key1_table(nullptr)
{
}

device_nds_cart_interface::~device_nds_cart_interface()
{
}

void device_nds_cart_interface::rom_alloc(uint32_t size, const char *tag)
{
	if (m_rom == nullptr)
	{
		m_rom = device().machine().memory().region_alloc(std::string(tag).append(NDSSLOT_ROM_REGION_TAG), size, 1, ENDIANNESS_LITTLE)->base();
		m_rom_size = size;
	}
}


//**************************************************************************
//  nds_cart_slot_device
//**************************************************************************

nds_cart_slot_device::nds_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NDS_CART_SLOT, tag, owner, clock),
	device_cartrom_image_interface(mconfig, *this),
	device_single_card_slot_interface<device_nds_cart_interface>(mconfig, *this),
	m_cart(nullptr)
{
}

nds_cart_slot_device::~nds_cart_slot_device()
{
}

void nds_cart_slot_device::device_start()
{
	m_cart = get_card_device();
}

std::pair<std::error_condition, std::string> nds_cart_slot_device::call_load()
{
	if (m_cart)
	{
		uint32_t const size = loaded_through_softlist() ? get_software_region_length("rom") : length();
		if (size < 0x200)
			return std::make_pair(image_error::INVALIDLENGTH, "Image is too small to hold a card header");
		if (size > 0x2000'0000)
			return std::make_pair(image_error::INVALIDLENGTH, "Cards larger than 512MB are not supported");

		// the ROM chips are powers of two, and the rest of the chip reads back as 0xff
		uint32_t alloc = 0x200;
		while (alloc < size)
			alloc <<= 1;

		m_cart->rom_alloc(alloc, tag());
		uint8_t *const rom = m_cart->get_rom_base();
		std::fill_n(rom, alloc, 0xff);

		if (!loaded_through_softlist())
			fread(rom, size);
		else
			memcpy(rom, get_software_region("rom"), size);

		if (m_cart->get_nvram_size())
			battery_load(m_cart->get_nvram_base(), m_cart->get_nvram_size(), 0xff);
	}

	return std::make_pair(std::error_condition(), std::string());
}

void nds_cart_slot_device::call_unload()
{
	if (m_cart && m_cart->get_nvram_size())
		battery_save(m_cart->get_nvram_base(), m_cart->get_nvram_size());
}

std::string nds_cart_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	if (hook.image_file())
		return std::string("nds_rom");

	return software_get_default_slot("nds_rom");
}
