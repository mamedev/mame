// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Super TV-PC cartridge slot and ROM/EEPROM variants. Loads ROM images up
    to 4 MiB with power-of-two sizes, provides optional I2C storage and forwards
    infrared receive pulses from cartridges with a transceiver. Cartridge
    address decoding remains in the system driver.

***************************************************************************/

#include "emu.h"
#include "suprtvpc_cart.h"

#include "suprtvpc_cart_ir.h"

#include <algorithm>

DEFINE_DEVICE_TYPE(SUPRTVPC_CART_SLOT, suprtvpc_cart_slot_device, "suprtvpc_cart", "Super TV-PC Cartridge Slot")
DEFINE_DEVICE_TYPE(SUPRTVPC_ROM, suprtvpc_rom_device, "suprtvpc_rom", "Super TV-PC Cartridge")
DEFINE_DEVICE_TYPE(SUPRTVPC_ROM_24C04, suprtvpc_rom_24c04_device, "suprtvpc_rom_24c04", "Super TV-PC Cartridge with 24C04 EEPROM")
DEFINE_DEVICE_TYPE(SUPRTVPC_ROM_24C08, suprtvpc_rom_24c08_device, "suprtvpc_rom_24c08", "Super TV-PC Cartridge with 24C08 EEPROM")
DEFINE_DEVICE_TYPE(SUPRTVPC_ROM_24C64, suprtvpc_rom_24c64_device, "suprtvpc_rom_24c64", "Super TV-PC Cartridge with 24C64 EEPROM")

device_suprtvpc_cart_interface::device_suprtvpc_cart_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "suprtvpccart")
	, m_rom(nullptr)
	, m_rom_size(0)
{
}

device_suprtvpc_cart_interface::~device_suprtvpc_cart_interface()
{
}

suprtvpc_cart_slot_device::suprtvpc_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SUPRTVPC_CART_SLOT, tag, owner, clock)
	, device_cartrom_image_interface(mconfig, *this)
	, device_single_card_slot_interface<device_suprtvpc_cart_interface>(mconfig, *this)
	, m_ir_rx_cb(*this)
{
}

suprtvpc_rom_device::suprtvpc_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: suprtvpc_rom_device(mconfig, SUPRTVPC_ROM, tag, owner, clock)
{
}

suprtvpc_rom_device::suprtvpc_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_suprtvpc_cart_interface(mconfig, *this)
{
}

suprtvpc_rom_i2c_device::suprtvpc_rom_i2c_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: suprtvpc_rom_device(mconfig, type, tag, owner, clock)
	, m_eeprom(*this, "eeprom")
{
}

suprtvpc_rom_24c04_device::suprtvpc_rom_24c04_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: suprtvpc_rom_i2c_device(mconfig, SUPRTVPC_ROM_24C04, tag, owner, clock)
{
}

suprtvpc_rom_24c08_device::suprtvpc_rom_24c08_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: suprtvpc_rom_i2c_device(mconfig, SUPRTVPC_ROM_24C08, tag, owner, clock)
{
}

suprtvpc_rom_24c64_device::suprtvpc_rom_24c64_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: suprtvpc_rom_i2c_device(mconfig, SUPRTVPC_ROM_24C64, tag, owner, clock)
{
}

std::string suprtvpc_cart_slot_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	return software_get_default_slot("rom");
}

bool suprtvpc_cart_slot_device::is_reset_on_load() const noexcept
{
	return true;
}

const char *suprtvpc_cart_slot_device::image_interface() const noexcept
{
	return "super_tv_pc_cart";
}

const char *suprtvpc_cart_slot_device::file_extensions() const noexcept
{
	return "bin";
}

void suprtvpc_rom_24c04_device::device_add_mconfig(machine_config &config)
{
	I2C_24C04(config, m_eeprom);
}

void suprtvpc_rom_24c08_device::device_add_mconfig(machine_config &config)
{
	I2C_24C08(config, m_eeprom);
}

void suprtvpc_rom_24c64_device::device_add_mconfig(machine_config &config)
{
	I2C_24C64(config, m_eeprom);
}

void suprtvpc_cart_slot_device::cart_devices(device_slot_interface &device)
{
	device.option_add_internal("rom", SUPRTVPC_ROM);
	device.option_add_internal("rom_24c04", SUPRTVPC_ROM_24C04);
	device.option_add_internal("rom_24c08", SUPRTVPC_ROM_24C08);
	device.option_add_internal("rom_24c64", SUPRTVPC_ROM_24C64);
	device.option_add_internal("rom_24c64_ir", SUPRTVPC_ROM_24C64_IR);
}

void suprtvpc_cart_slot_device::device_start()
{
	m_ir_rx_cb.resolve_safe();

	m_cart = get_card_device();
}

void suprtvpc_rom_device::device_start()
{
}

void device_suprtvpc_cart_interface::sda_w(int state)
{
}

void device_suprtvpc_cart_interface::scl_w(int state)
{
}

int device_suprtvpc_cart_interface::sda_r()
{
	return 1;
}

void device_suprtvpc_cart_interface::ir_tx_w(int state)
{
}

void suprtvpc_rom_i2c_device::sda_w(int state)
{
	m_eeprom->write_sda(state);
}

void suprtvpc_rom_i2c_device::scl_w(int state)
{
	m_eeprom->write_scl(state);
}

int suprtvpc_rom_i2c_device::sda_r()
{
	return m_eeprom->read_sda();
}

void device_suprtvpc_cart_interface::ir_rx_pulse()
{
	downcast<suprtvpc_cart_slot_device &>(*device().owner()).ir_rx_pulse();
}

void device_suprtvpc_cart_interface::rom_alloc(u32 size)
{
	if (!m_rom)
	{
		m_rom = device().machine().memory().region_alloc(device().subtag("^cart:rom"), size, 1, ENDIANNESS_LITTLE)->base();
		m_rom_size = size;
	}
}

std::pair<std::error_condition, std::string> suprtvpc_cart_slot_device::call_load()
{
	u64 const size = loaded_through_softlist() ? get_software_region_length("prg") : length();
	if (!size)
		return std::make_pair(image_error::INVALIDLENGTH, "Cartridges must not be empty");
	if (size > 0x40'0000)
		return std::make_pair(image_error::INVALIDLENGTH, "Cartridges must be no larger than 4 MiB (0x400000 bytes)");
	if (size & (size - 1))
		return std::make_pair(image_error::INVALIDLENGTH, "Cartridge size must be a power of two");

	u32 const size_bytes = u32(size);
	m_cart->rom_alloc(size_bytes);
	if (loaded_through_softlist())
	{
		u8 const *const source = get_software_region("prg");
		if (!source)
			return std::make_pair(image_error::BADSOFTWARE, "Software list item is missing 'prg' region");
		std::copy_n(source, size_bytes, m_cart->rom_base());
	}
	else if (fread(m_cart->rom_base(), size_bytes) != size_bytes)
	{
		return std::make_pair(std::errc::io_error, "Error reading cartridge file");
	}

	return std::make_pair(std::error_condition(), std::string());
}
