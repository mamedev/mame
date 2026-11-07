// license:BSD-3-Clause
// copyright-holders: Peter Wilhelmsen

#include "emu.h"
#include "slot.h"

#include "nand.h"

DEFINE_DEVICE_TYPE(DIGIBLAST_CART_SLOT, digiblast_cart_slot_device, "digiblast_cart_slot", "digiBLAST Cartridge Slot")

device_digiblast_cart_interface::device_digiblast_cart_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "digiblast_cart")
{
}

digiblast_cart_slot_device::digiblast_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, DIGIBLAST_CART_SLOT, tag, owner, clock)
	, device_cartrom_image_interface(mconfig, *this)
	, device_single_card_slot_interface<device_digiblast_cart_interface>(mconfig, *this)
	, m_cart(nullptr)
	, m_ready_cb(*this)
{
}

void digiblast_cart_slot_device::device_start()
{
	m_cart = get_card_device();
}

std::pair<std::error_condition, std::string> digiblast_cart_slot_device::call_load()
{
	if (!m_cart)
		return std::make_pair(image_error::UNSPECIFIED, "No cartridge device selected");

	memory_region *region = loaded_through_softlist() ? memregion("rom") : nullptr;
	if (loaded_through_softlist() && !region)
		return std::make_pair(image_error::BADSOFTWARE, "Cartridge has no 'rom' data area");

	u64 const size = region ? region->bytes() : length();
	if (size != m_cart->rom_size())
		return std::make_pair(image_error::INVALIDLENGTH, "Image size does not match the cartridge NAND (512 data bytes and 16 spare bytes per page)");

	memory_region *default_eeprom = loaded_through_softlist() ? memregion("eeprom") : nullptr;

	if (default_eeprom)
	{
		u64 const default_eeprom_size = default_eeprom->bytes();
		if (default_eeprom_size != m_cart->eeprom_size())
			return std::make_pair(image_error::BADSOFTWARE, "Default EEPROM size does not match the cartridge EEPROM");

		m_cart->set_default_eeprom(default_eeprom->base());
	}

	if (!loaded_through_softlist())
	{
		region = machine().memory().region_alloc(subtag("rom"), size, 1, ENDIANNESS_LITTLE);
		if (fread(region->base(), size) != size)
			return std::make_pair(image_error::UNSPECIFIED, "Error reading cartridge image");
	}

	m_cart->set_rom(region->base(), size);
	return std::make_pair(std::error_condition(), std::string());
}

void digiblast_cart_slot_device::call_unload()
{
	if (m_cart)
		m_cart->set_rom(nullptr, 0);
}

std::string digiblast_cart_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	if (hook.image_file())
	{
		u64 size;
		if (!hook.image_file()->length(size))
		{
			switch (size)
			{
			case 0x1080000: return "nand_16m";
			case 0x2100000: return "nand_32m";
			case 0x4200000: return "nand_64m";
			}
		}
	}
	return software_get_default_slot("nand_64m");
}

void digiblast_cart(device_slot_interface &device)
{
	device.option_add_internal("nand_16m", DIGIBLAST_NAND_16M);
	device.option_add_internal("nand_32m", DIGIBLAST_NAND_32M);
	device.option_add_internal("nand_64m", DIGIBLAST_NAND_64M);
}

bool digiblast_cart_slot_device::is_reset_on_load() const noexcept
{
	return true;
}

const char *digiblast_cart_slot_device::image_interface() const noexcept
{
	return "digiblast_cart";
}

const char *digiblast_cart_slot_device::file_extensions() const noexcept
{
	return "bin";
}

u8 digiblast_cart_slot_device::data_r()
{
	return m_cart ? m_cart->data_r() : 0xff;
}

void digiblast_cart_slot_device::data_w(u8 data)
{
	if (m_cart)
		m_cart->data_w(data);
}

void digiblast_cart_slot_device::command_w(u8 data)
{
	if (m_cart)
		m_cart->command_w(data);
}

void digiblast_cart_slot_device::address_w(u8 data)
{
	if (m_cart)
		m_cart->address_w(data);
}

int digiblast_cart_slot_device::i2c_sda_r()
{
	return m_cart ? m_cart->i2c_sda_r() : 1;
}

void digiblast_cart_slot_device::i2c_sda_w(int state)
{
	if (m_cart)
		m_cart->i2c_sda_w(state);
}

void digiblast_cart_slot_device::i2c_scl_w(int state)
{
	if (m_cart)
		m_cart->i2c_scl_w(state);
}
