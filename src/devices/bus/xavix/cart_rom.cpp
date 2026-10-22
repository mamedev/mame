// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn
/***************************************************************************

    XaviX ROM cartridges, including variants with I2C EEPROMs. ROM reads
    mirror the installed image; EEPROM access uses cartridge bus-control
    registers or direct SDA/SCL connections. Cartridge options select the
    EEPROM capacity and its wiring to the console.

    Documented physical EEPROMs are 24C04/24C08 for the control 1 variants,
    24LC04/24LC02 for the control 0 variants, and 24LC02/24LC08 for the direct
    variants, respectively. The I2C_24Cxx devices provide compatible models.

***************************************************************************/

#include "emu.h"
#include "cart_rom.h"

#define LOG_ROM (1U << 1)
#define LOG_I2C (1U << 2)
#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(XAVIX_ROM_PLAIN,                xavix_rom_plain_device,                "xavix_rom_plain",          "XaviX ROM Cartridge")
DEFINE_DEVICE_TYPE(XAVIX_ROM_I2C_24C04_CONTROL1,   xavix_rom_i2c_24c04_control1_device,   "xavix_rom_24c04_control1", "XaviX ROM Cartridge with 24C04-Compatible I2C EEPROM (Control 1)")
DEFINE_DEVICE_TYPE(XAVIX_ROM_I2C_24C08_CONTROL1,   xavix_rom_i2c_24c08_control1_device,   "xavix_rom_24c08_control1", "XaviX ROM Cartridge with 24C08-Compatible I2C EEPROM")
DEFINE_DEVICE_TYPE(XAVIX_ROM_I2C_24C04_CONTROL0,   xavix_rom_i2c_24c04_control0_device,   "xavix_rom_24c04_control0", "XaviX ROM Cartridge with 24C04-Compatible I2C EEPROM (Control 0)")
DEFINE_DEVICE_TYPE(XAVIX_ROM_I2C_24C02_CONTROL0,   xavix_rom_i2c_24c02_control0_device,   "xavix_rom_24c02_control0", "XaviX ROM Cartridge with 24C02-Compatible I2C EEPROM")
DEFINE_DEVICE_TYPE(XAVIX_ROM_I2C_24C02_DIRECT,     xavix_rom_i2c_24c02_direct_device,     "xavix_rom_24c02_direct",   "XaviX ROM Cartridge with 24C02-Compatible I2C EEPROM (Direct I/O)")
DEFINE_DEVICE_TYPE(XAVIX_ROM_I2C_24C08_DIRECT,     xavix_rom_i2c_24c08_direct_device,     "xavix_rom_24c08_direct",   "XaviX ROM Cartridge with 24C08-Compatible I2C EEPROM (Direct I/O)")


xavix_rom_plain_device::xavix_rom_plain_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock), device_xavix_cart_interface(mconfig, *this)
{
}

xavix_rom_plain_device::xavix_rom_plain_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	xavix_rom_plain_device(mconfig, XAVIX_ROM_PLAIN, tag, owner, clock)
{
}

xavix_rom_i2c_base_device::xavix_rom_i2c_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint8_t bus_write_control_index, uint8_t bus_write_control_mask) :
	xavix_rom_plain_device(mconfig, type, tag, owner, clock),
	m_i2cmem(*this, "i2cmem"),
	m_bus_write_control_index(bus_write_control_index),
	m_bus_write_control_mask(bus_write_control_mask)
{
}

xavix_rom_i2c_24c04_control1_device::xavix_rom_i2c_24c04_control1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	xavix_rom_i2c_base_device(mconfig, XAVIX_ROM_I2C_24C04_CONTROL1, tag, owner, clock, 1, 0x04)
{
}

xavix_rom_i2c_24c08_control1_device::xavix_rom_i2c_24c08_control1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	xavix_rom_i2c_base_device(mconfig, XAVIX_ROM_I2C_24C08_CONTROL1, tag, owner, clock, 1, 0x04)
{
}

xavix_rom_i2c_24c04_control0_device::xavix_rom_i2c_24c04_control0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	xavix_rom_i2c_base_device(mconfig, XAVIX_ROM_I2C_24C04_CONTROL0, tag, owner, clock, 0, 0x04)
{
}

xavix_rom_i2c_24c02_control0_device::xavix_rom_i2c_24c02_control0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	xavix_rom_i2c_base_device(mconfig, XAVIX_ROM_I2C_24C02_CONTROL0, tag, owner, clock)
{
}

xavix_rom_i2c_direct_device::xavix_rom_i2c_direct_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	xavix_rom_plain_device(mconfig, type, tag, owner, clock),
	m_i2cmem(*this, "i2cmem")
{
}

xavix_rom_i2c_24c02_direct_device::xavix_rom_i2c_24c02_direct_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	xavix_rom_i2c_direct_device(mconfig, XAVIX_ROM_I2C_24C02_DIRECT, tag, owner, clock)
{
}

xavix_rom_i2c_24c08_direct_device::xavix_rom_i2c_24c08_direct_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	xavix_rom_i2c_direct_device(mconfig, XAVIX_ROM_I2C_24C08_DIRECT, tag, owner, clock)
{
}

void xavix_rom_i2c_24c04_control1_device::device_add_mconfig(machine_config &config)
{
	I2C_24C04(config, m_i2cmem);
}

void xavix_rom_i2c_24c08_control1_device::device_add_mconfig(machine_config &config)
{
	I2C_24C08(config, m_i2cmem);
}

void xavix_rom_i2c_24c04_control0_device::device_add_mconfig(machine_config &config)
{
	I2C_24C04(config, m_i2cmem);
}

void xavix_rom_i2c_24c02_control0_device::device_add_mconfig(machine_config &config)
{
	I2C_24C02(config, m_i2cmem);
}

void xavix_rom_i2c_24c02_direct_device::device_add_mconfig(machine_config &config)
{
	I2C_24C02(config, m_i2cmem);
}

void xavix_rom_i2c_24c08_direct_device::device_add_mconfig(machine_config &config)
{
	I2C_24C08(config, m_i2cmem);
}

void xavix_cart_slot_device::cart_devices(device_slot_interface &device)
{
	device.option_add_internal("plain",                XAVIX_ROM_PLAIN);
	device.option_add_internal("rom_24c04_control1",   XAVIX_ROM_I2C_24C04_CONTROL1);
	device.option_add_internal("rom_24c08_control1",   XAVIX_ROM_I2C_24C08_CONTROL1);
	device.option_add_internal("rom_24c04_control0",   XAVIX_ROM_I2C_24C04_CONTROL0);
	device.option_add_internal("rom_24c02_control0",   XAVIX_ROM_I2C_24C02_CONTROL0);
	device.option_add_internal("rom_24c02_direct",     XAVIX_ROM_I2C_24C02_DIRECT);
	device.option_add_internal("rom_24c08_direct",     XAVIX_ROM_I2C_24C08_DIRECT);
}

void xavix_rom_i2c_base_device::device_start()
{
	save_item(NAME(m_buscontrol));
}

void xavix_rom_plain_device::device_start()
{
}

void xavix_rom_i2c_base_device::device_reset()
{
	m_buscontrol[0] = m_buscontrol[1] = m_buscontrol[2] = 0x00;
}

uint8_t xavix_rom_plain_device::read_cart(offs_t offset)
{
	return m_rom[offset & (m_rom_size - 1)];
}

void xavix_rom_plain_device::write_cart(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_ROM, "%s: ROM write %08x = %02x\n", machine().describe_context(), offset, data);
}

void xavix_rom_i2c_base_device::write_bus_control(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_I2C, "%s: Bus control write %08x = %02x\n", machine().describe_context(), offset, data);
	m_buscontrol[offset] = data;
}

void xavix_rom_i2c_base_device::write_cart(offs_t offset, uint8_t data)
{
	if ((m_buscontrol[m_bus_write_control_index] & m_bus_write_control_mask) && (offset == 0x3fffff))
		write_i2c(offset, data);
	else
		LOGMASKED(LOG_ROM, "%s: ROM write %08x = %02x\n", machine().describe_context(), offset, data);
}

uint8_t xavix_rom_i2c_base_device::read_cart(offs_t offset)
{
	if ((m_buscontrol[1] & 0x08) && (offset == 0x5fffff))
		return read_i2c(offset);

	return xavix_rom_plain_device::read_cart(offset);
}

uint8_t xavix_rom_i2c_base_device::read_i2c(offs_t offset)
{
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_I2C, "%s: I2C read %08x\n", machine().describe_context(), offset);

	return m_i2cmem->read_sda() ? 0xff : 0x00;
}

void xavix_rom_i2c_base_device::write_i2c(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_I2C, "%s: I2C write %08x = %02x\n", machine().describe_context(), offset, data);

	m_i2cmem->write_sda(BIT(data, 0));
	m_i2cmem->write_scl(BIT(data, 1));
}

// The cartridge bus exposes the shared I2C memory device's pins directly.
// Super TV-PC uses a separate slot interface and EEPROM child tag.
void xavix_rom_i2c_direct_device::write_sda(int state)
{
	m_i2cmem->write_sda(state);
}

void xavix_rom_i2c_direct_device::write_scl(int state)
{
	m_i2cmem->write_scl(state);
}

int xavix_rom_i2c_direct_device::read_sda()
{
	return m_i2cmem->read_sda();
}
