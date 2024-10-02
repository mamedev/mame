// license:BSD-3-Clause
// copyright-holders:

#include "emu.h"
#include "slot.h"


DEFINE_DEVICE_TYPE(SUPERACAN_CART_SLOT, superacan_cart_slot_device, "superacan_cart_slot", "Super A'Can Cartridge Slot")


device_superacan_cart_interface::device_superacan_cart_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "superacan_cart")
	, m_slot(dynamic_cast<superacan_cart_slot_device *>(device.owner()))
{
}


device_superacan_cart_interface::~device_superacan_cart_interface()
{
}


void device_superacan_cart_interface::battery_load(void *buffer, int length, int fill)
{
	assert(m_slot);
	m_slot->battery_load(buffer, length, fill);
}

void device_superacan_cart_interface::battery_load(void *buffer, int length, void *def_buffer)
{
	assert(m_slot);
	m_slot->battery_load(buffer, length, def_buffer);
}

void device_superacan_cart_interface::battery_save(const void *buffer, int length)
{
	assert(m_slot);
	m_slot->battery_save(buffer, length);
}


superacan_cart_slot_device::superacan_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, SUPERACAN_CART_SLOT, tag, owner, clock),
	device_cartrom_image_interface(mconfig, *this),
	device_single_card_slot_interface<device_superacan_cart_interface>(mconfig, *this),
	m_cart(nullptr)
{
}


//-------------------------------------------------
//  superacan_cart_slot_device - destructor
//-------------------------------------------------

superacan_cart_slot_device::~superacan_cart_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void superacan_cart_slot_device::device_start()
{
	m_cart = get_card_device();
}



/*-------------------------------------------------
 call load
 -------------------------------------------------*/

std::pair<std::error_condition, std::string> superacan_cart_slot_device::call_load()
{
	if (!m_cart)
		return std::make_pair(std::error_condition(), std::string());

	memory_region *romregion = loaded_through_softlist() ? memregion("rom") : nullptr;
	if (loaded_through_softlist() && !romregion)
		return std::make_pair(image_error::INVALIDLENGTH, "Software list item has no 'rom' data area");

	const u32 len = loaded_through_softlist() ? romregion->bytes() : length();

	if (!loaded_through_softlist())
	{
		romregion = machine().memory().region_alloc(subtag("rom"), len, 2, ENDIANNESS_BIG);
		u16 *const rombase = reinterpret_cast<u16 *>(romregion->base());
		const u32 cnt = fread(rombase, len);
		if (cnt != len)
			return std::make_pair(std::errc::io_error, "Error reading cartridge file");
	}

	return std::make_pair(m_cart->load(), std::string());
}


/*-------------------------------------------------
 call unload
 -------------------------------------------------*/

void superacan_cart_slot_device::call_unload()
{
	if (m_cart)
		m_cart->unload();
}


/*-------------------------------------------------
 get default card software
 -------------------------------------------------*/

std::string superacan_cart_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	return software_get_default_slot("std");
}
