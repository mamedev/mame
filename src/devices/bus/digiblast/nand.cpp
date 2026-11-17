// license:BSD-3-Clause
// copyright-holders: Peter Wilhelmsen

// The boot firmware accepts Toshiba 0x73/0x75/0x76 devices.  These configurations
// match the dump capacities; the parts fitted to the cartridges need verification.
// Photographed cartridges have 24C08 EEPROMs; other cartridge PCBs need verification.

#include "emu.h"
#include "nand.h"

#include <algorithm>

DEFINE_DEVICE_TYPE(DIGIBLAST_NAND_16M,   digiblast_nand_16m_device,   "digiblast_nand_16m",   "digiBLAST 16 MiB NAND Cartridge")
DEFINE_DEVICE_TYPE(DIGIBLAST_NAND_32M,   digiblast_nand_32m_device,   "digiblast_nand_32m",   "digiBLAST 32 MiB NAND Cartridge")
DEFINE_DEVICE_TYPE(DIGIBLAST_NAND_64M,   digiblast_nand_64m_device,   "digiblast_nand_64m",   "digiBLAST 64 MiB NAND Cartridge")

ROM_START(digiblast_cart)
	ROM_REGION(0x400, "i2cmem", ROMREGION_ERASEFF)
ROM_END

digiblast_nand_device::digiblast_nand_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_digiblast_cart_interface(mconfig, *this)
	, m_nand(*this, "nand")
	, m_i2cmem(*this, "i2cmem")
	, m_slot(*this, DEVICE_SELF_OWNER)
	, m_default_eeprom(*this, "i2cmem")
{
}

void digiblast_nand_device::device_start()
{
}

const tiny_rom_entry *digiblast_nand_device::device_rom_region() const
{
	return ROM_NAME(digiblast_cart);
}

void digiblast_nand_device::configure_cart(machine_config &config)
{
	m_nand->rnb_wr_callback().set(m_slot, FUNC(digiblast_cart_slot_device::ready_w));
	I2C_24C08(config, m_i2cmem);
	m_i2cmem->set_address(0xa0);
	m_i2cmem->set_e0(0);
	m_i2cmem->set_e1(0);
	m_i2cmem->set_e2(0);
}

digiblast_nand_16m_device::digiblast_nand_16m_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: digiblast_nand_16m_device(mconfig, DIGIBLAST_NAND_16M, tag, owner, clock)
{
}

digiblast_nand_16m_device::digiblast_nand_16m_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: digiblast_nand_device(mconfig, type, tag, owner, clock)
{
}

void digiblast_nand_16m_device::device_add_mconfig(machine_config &config)
{
	TOSHIBA_TC58128FT(config, m_nand);
	configure_cart(config);
}

digiblast_nand_32m_device::digiblast_nand_32m_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: digiblast_nand_32m_device(mconfig, DIGIBLAST_NAND_32M, tag, owner, clock)
{
}

digiblast_nand_32m_device::digiblast_nand_32m_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: digiblast_nand_device(mconfig, type, tag, owner, clock)
{
}

void digiblast_nand_32m_device::device_add_mconfig(machine_config &config)
{
	TOSHIBA_TC58256FT(config, m_nand);
	configure_cart(config);
}

digiblast_nand_64m_device::digiblast_nand_64m_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: digiblast_nand_64m_device(mconfig, DIGIBLAST_NAND_64M, tag, owner, clock)
{
}

digiblast_nand_64m_device::digiblast_nand_64m_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: digiblast_nand_device(mconfig, type, tag, owner, clock)
{
}

void digiblast_nand_64m_device::device_add_mconfig(machine_config &config)
{
	TOSHIBA_TC58512FT(config, m_nand);
	configure_cart(config);
}

u8 digiblast_nand_device::data_r()
{
	return m_nand->data_r();
}

void digiblast_nand_device::data_w(u8 data)
{
	m_nand->data_w(data);
}

void digiblast_nand_device::command_w(u8 data)
{
	m_nand->command_w(data);
}

void digiblast_nand_device::address_w(u8 data)
{
	m_nand->address_w(data);
}

int digiblast_nand_device::i2c_sda_r()
{
	return m_i2cmem->read_sda();
}

void digiblast_nand_device::i2c_sda_w(int state)
{
	m_i2cmem->write_sda(state);
}

void digiblast_nand_device::i2c_scl_w(int state)
{
	m_i2cmem->write_scl(state);
}

u32 digiblast_nand_device::rom_size() const
{
	return m_nand->nand_size();
}

void digiblast_nand_device::set_rom(const u8 *data, u32 size)
{
	m_nand->set_rom(data, size);
}

u32 digiblast_nand_device::eeprom_size() const
{
	return m_default_eeprom.bytes();
}

void digiblast_nand_device::set_default_eeprom(const u8 *data)
{
	std::copy_n(data, m_default_eeprom.bytes(), &m_default_eeprom[0]);
}
