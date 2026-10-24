// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn
/***************************************************************************

    Cartridge slot and bus interface for XaviX systems. Loads cartridge ROM
    images from files or software lists and forwards ROM accesses, bus-control
    writes and optional I2C lines to the installed cartridge device.

***************************************************************************/

#include "emu.h"
#include "cart.h"

#include <algorithm>
#include <bit>

DEFINE_DEVICE_TYPE(XAVIX_CART_SLOT, xavix_cart_slot_device, "xavix_cart_slot", "XaviX Cartridge Slot")

device_xavix_cart_interface::device_xavix_cart_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "xavixcart"),
	m_rom(nullptr),
	m_rom_size(0)
{
}

device_xavix_cart_interface::~device_xavix_cart_interface()
{
}

xavix_cart_slot_device::xavix_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, XAVIX_CART_SLOT, tag, owner, clock),
	device_cartrom_image_interface(mconfig, *this),
	device_single_card_slot_interface<device_xavix_cart_interface>(mconfig, *this)
{
}

std::string xavix_cart_slot_device::get_default_card_software(get_default_card_software_hook &) const
{
	return software_get_default_slot("plain");
}

bool xavix_cart_slot_device::is_reset_on_load() const noexcept
{
	return true;
}

const char *xavix_cart_slot_device::image_interface() const noexcept
{
	return "xavix_cart";
}

const char *xavix_cart_slot_device::file_extensions() const noexcept
{
	return "bin,u1";
}

void xavix_cart_slot_device::device_start()
{
	m_cart = get_card_device();
}

void device_xavix_cart_interface::rom_alloc(uint32_t size)
{
	if (m_rom == nullptr)
	{
		m_rom = device().machine().memory().region_alloc(device().subtag("^cart:rom"), size, 1, ENDIANNESS_BIG)->base();
		m_rom_size = size;
	}
}

std::pair<std::error_condition, std::string> xavix_cart_slot_device::call_load()
{
	if (m_cart)
	{
		bool const softlist = loaded_through_softlist();
		uint8_t const *const source = softlist ? get_software_region("rom") : nullptr;
		if (softlist && !source)
			return std::make_pair(image_error::BADSOFTWARE, "Software list item is missing 'rom' region");

		uint64_t const size = softlist ? get_software_region_length("rom") : length();
		if (!size)
			return std::make_pair(image_error::INVALIDLENGTH, "Cartridges must not be empty");
		// Supported unbanked cartridges are mapped within 23 address bits.
		if (size > 0x800000)
			return std::make_pair(image_error::INVALIDLENGTH, "Cartridges must be no larger than 8 MiB (0x800000 bytes)");
		if (!std::has_single_bit(size))
			return std::make_pair(image_error::INVALIDLENGTH, "Cartridge size must be a power of two");

		uint32_t const size_bytes = uint32_t(size);
		m_cart->rom_alloc(size_bytes);
		if (softlist)
			std::copy_n(source, size_bytes, m_cart->get_rom_base());
		else if (fread(m_cart->get_rom_base(), size_bytes) != size_bytes)
			return std::make_pair(std::errc::io_error, "Error reading cartridge file");
	}

	return std::make_pair(std::error_condition(), std::string());
}

uint8_t xavix_cart_slot_device::read_cart(offs_t offset)
{
	return m_cart->read_cart(offset);
}

void xavix_cart_slot_device::write_cart(offs_t offset, uint8_t data)
{
	m_cart->write_cart(offset, data);
}

void xavix_cart_slot_device::write_bus_control(offs_t offset, uint8_t data)
{
	m_cart->write_bus_control(offset, data);
}

void xavix_cart_slot_device::write_sda(int state)
{
	m_cart->write_sda(state);
}

void xavix_cart_slot_device::write_scl(int state)
{
	m_cart->write_scl(state);
}

int xavix_cart_slot_device::read_sda()
{
	return m_cart->read_sda();
}
