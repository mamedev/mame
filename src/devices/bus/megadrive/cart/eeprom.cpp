// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Megadrive ROM + I2C EEPROM

**************************************************************************************************/

#include "emu.h"
#include "eeprom.h"

/*
 * evander          Evander Holyfield Boxing
 * ddanpei          Honoo no Toukyuuji - Dodge Danpei
 * gameto           Game Toshokan
 * ghw/ghwj/ghwu    Greatest Heavyweights
 * ninjab           Ninja Burai Densetsu
 * megaman/rockman1 Megaman The Wily Wars
 * sporttbb         Sports Talk Baseball
 * wboymw/wboy5     Wonder Boy in Monster World
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_EEPROM, megadrive_eeprom_device, "megadrive_eeprom", "Megadrive ROM + I2C EEPROM cart")

megadrive_eeprom_device::megadrive_eeprom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_rom_device(mconfig, type, tag, owner, clock)
	, m_i2cmem(*this, "i2cmem")
{
}

megadrive_eeprom_device::megadrive_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_eeprom_device(mconfig, MEGADRIVE_EEPROM, tag, owner, clock)
{
}

void megadrive_eeprom_device::device_add_mconfig(machine_config &config)
{
	I2C_X24C01(config, m_i2cmem);
}

void megadrive_eeprom_device::cart_map(address_map &map)
{
	map(0x00'0000, m_rom_mask).mirror(m_rom_mirror).bankr(m_rom);
	map(0x20'0001, 0x20'0001).lrw8(
		NAME([this] (offs_t offset) {
			return m_i2cmem->read_sda();
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_i2cmem->write_scl(BIT(data, 1));
			m_i2cmem->write_sda(BIT(data, 0));
		})
	);
}

/*
 * nbajam/nbajam1/nbajamj NBA Jam
 *
 * Uses a 24C02, otherwise same as Sega handling
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_EEPROM_NBAJAM, megadrive_eeprom_nbajam_device, "megadrive_eeprom_nbajam", "Megadrive NBA Jam cart")


megadrive_eeprom_nbajam_device::megadrive_eeprom_nbajam_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_eeprom_device(mconfig, MEGADRIVE_EEPROM_NBAJAM, tag, owner, clock)
{
}

void megadrive_eeprom_nbajam_device::device_add_mconfig(machine_config &config)
{
	I2C_24C02(config, m_i2cmem);
}

/*
 * nbajamte NBA Jam TE (MD and 32x versions)
 * nflqb    NFL Quarterback Club
 * blockb   Blockbuster World Video Game Championship II
 *
 * Uses a 24C02, moves SCL to $200000
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_EEPROM_NBAJAMTE, megadrive_eeprom_nbajamte_device, "megadrive_eeprom_nbajamte", "Megadrive NBA Jam TE cart")

megadrive_eeprom_nbajamte_device::megadrive_eeprom_nbajamte_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_eeprom_device(mconfig, type, tag, owner, clock)
{
}

megadrive_eeprom_nbajamte_device::megadrive_eeprom_nbajamte_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_eeprom_nbajamte_device(mconfig, MEGADRIVE_EEPROM_NBAJAMTE, tag, owner, clock)
{
}

void megadrive_eeprom_nbajamte_device::device_add_mconfig(machine_config &config)
{
	// nflqb uses a 24LC02B
	I2C_24C02(config, m_i2cmem);
}

void megadrive_eeprom_nbajamte_device::cart_map(address_map &map)
{
	map(0x00'0000, m_rom_mask).mirror(m_rom_mirror).bankr(m_rom);
	map(0x20'0000, 0x20'0000).lw8(
		NAME([this] (offs_t offset, u8 data) {
			m_i2cmem->write_scl(BIT(data, 0));
		})
	);
	map(0x20'0001, 0x20'0001).lrw8(
		NAME([this] (offs_t offset) {
			return m_i2cmem->read_sda();
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_i2cmem->write_sda(BIT(data, 0));
		})
	);
}

/*
 * nflqb96 NFL Quarterback Club 96
 *
 * Same as NBA Jam TE with a '16 in place of the '02
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_EEPROM_NFLQB96, megadrive_eeprom_nflqb96_device, "megadrive_eeprom_nflqb96", "Megadrive NFL Quarterback Club 96 cart")

megadrive_eeprom_nflqb96_device::megadrive_eeprom_nflqb96_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_eeprom_nbajamte_device(mconfig, MEGADRIVE_EEPROM_NFLQB96, tag, owner, clock)
{
}

void megadrive_eeprom_nflqb96_device::device_add_mconfig(machine_config &config)
{
	I2C_24C16(config, m_i2cmem);
}

/*
 * collslam College Slam
 * bighurt  Frank Thomas Big Hurt Baseball
 *
 * Bump of nflqb96 with a '65 in place of the '16
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_EEPROM_COLLSLAM, megadrive_eeprom_collslam_device, "megadrive_eeprom_collslam", "Megadrive College Slam cart")

megadrive_eeprom_collslam_device::megadrive_eeprom_collslam_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_eeprom_nbajamte_device(mconfig, MEGADRIVE_EEPROM_COLLSLAM, tag, owner, clock)
{
}

void megadrive_eeprom_collslam_device::device_add_mconfig(machine_config &config)
{
	I2C_24C65(config, m_i2cmem);
}

/*
 * nhlpa93  NHLPA Hockey 93
 * ringspow Rings of Power
 *
 * '01 at bits 7-6 of $200001
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_EEPROM_NHLPA, megadrive_eeprom_nhlpa_device, "megadrive_eeprom_nhlpa", "Megadrive NHLPA Hockey 93 cart")

megadrive_eeprom_nhlpa_device::megadrive_eeprom_nhlpa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_eeprom_device(mconfig, MEGADRIVE_EEPROM_NHLPA, tag, owner, clock)
{
}

void megadrive_eeprom_nhlpa_device::device_add_mconfig(machine_config &config)
{
	I2C_24C01(config, m_i2cmem);
}

void megadrive_eeprom_nhlpa_device::cart_map(address_map &map)
{
	map(0x00'0000, m_rom_mask).mirror(m_rom_mirror).bankr(m_rom);
	map(0x20'0001, 0x20'0001).lrw8(
		NAME([this] (offs_t offset) {
			return m_i2cmem->read_sda() << 7;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_i2cmem->write_sda(BIT(data, 7));
			m_i2cmem->write_scl(BIT(data, 6));
		})
	);
}



/*
 * brianlar  Brian Lara Cricket
 *
 * Same as Codemasters J-Cart I2C implementation
 *
 * Guru I2C hookup notes:
 * 1,2,3,4,7 GND
 * 5 to LS244 pin 17
 * 6 to PAL pin 15
 * Output of 244 pin 17 is pin 3 which goes to ROM pin 29. It's a 42 pin ROM so pin 29 is D7
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_EEPROM_BLARA95, megadrive_eeprom_blara95_device, "megadrive_eeprom_blara95", "Megadrive Brian Lara Cricket 95 cart")

megadrive_eeprom_blara95_device::megadrive_eeprom_blara95_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_eeprom_device(mconfig, type, tag, owner, clock)
{
}

megadrive_eeprom_blara95_device::megadrive_eeprom_blara95_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_eeprom_blara95_device(mconfig, MEGADRIVE_EEPROM_BLARA95, tag, owner, clock)
{
}

void megadrive_eeprom_blara95_device::device_add_mconfig(machine_config &config)
{
	I2C_24C08(config, m_i2cmem);
}

void megadrive_eeprom_blara95_device::cart_map(address_map &map)
{
	map(0x00'0000, m_rom_mask).mirror(m_rom_mirror).bankr(m_rom);
	map(0x30'0000, 0x30'0000).lw8(
		NAME([this] (offs_t offset, u8 data) {
			m_i2cmem->write_scl(BIT(data, 1));
			m_i2cmem->write_sda(BIT(data, 0));
		})
	);
	map(0x38'0001, 0x38'0001).lr8(
		NAME([this] (offs_t offset) {
			return m_i2cmem->read_sda() << 7;
		})
	);
}

/*
 * brianl96  Brian Lara Cricket 96
 * shanewar  Shane Warne Cricket
 *
 * Same as Brian Lara 95, with a '65 here
 *
 */

DEFINE_DEVICE_TYPE(MEGADRIVE_EEPROM_BLARA96, megadrive_eeprom_blara96_device, "megadrive_eeprom_blara96", "Megadrive Brian Lara Cricket 96 cart")

megadrive_eeprom_blara96_device::megadrive_eeprom_blara96_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: megadrive_eeprom_blara95_device(mconfig, MEGADRIVE_EEPROM_BLARA96, tag, owner, clock)
{
}

void megadrive_eeprom_blara96_device::device_add_mconfig(machine_config &config)
{
	I2C_24C65(config, m_i2cmem);
}

