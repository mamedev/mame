// license:BSD-3-Clause
// copyright-holders:smf
#include "emu.h"
#include "linflash.h"

DEFINE_DEVICE_TYPE(LINEAR_FLASH_PCCARD_16MB, linear_flash_pccard_16mb_device, "linearflash16mb", "Linear Flash PC Card (16MB)")
DEFINE_DEVICE_TYPE(LINEAR_FLASH_PCCARD_32MB, linear_flash_pccard_32mb_device, "linearflash32mb", "Linear Flash PC Card (32MB)")
DEFINE_DEVICE_TYPE(LINEAR_FLASH_PCCARD_64MB, linear_flash_pccard_64mb_device, "linearflash64mb", "Linear Flash PC Card (64MB)")


linear_flash_pccard_device::linear_flash_pccard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	device_slot_card_interface(mconfig, *this),
	m_space(nullptr)
{
}

void linear_flash_pccard_device::device_start()
{
	m_space = &space(0);
}

device_memory_interface::space_config_vector linear_flash_pccard_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

READ16_MEMBER( linear_flash_pccard_device::read_memory )
{
	uint16_t data = m_space->read_word(offset * 2, mem_mask);
	//printf( "<%08x %04x %04x\n", offset, data, mem_mask );
	return data;
}

WRITE16_MEMBER( linear_flash_pccard_device::write_memory )
{
	//printf( ">%08x %04x %04x\n", offset, data, mem_mask );
	m_space->write_word(offset * 2, data, mem_mask);
}


void linear_flash_pccard_16mb_device::linear_flash_pccard_16mb(address_map &map)
{
	map.unmap_value_high();
	map(0x00000000, 0x003fffff).rw("1l", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);
	map(0x00000000, 0x003fffff).rw("1u", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0xff00);
	map(0x00400000, 0x007fffff).rw("2l", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);
	map(0x00400000, 0x007fffff).rw("2u", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0xff00);
	map(0x00800000, 0x00bfffff).rw("3l", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);
	map(0x00800000, 0x00bfffff).rw("3u", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0xff00);
	map(0x00c00000, 0x00ffffff).rw("4l", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);
	map(0x00c00000, 0x00ffffff).rw("4u", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0xff00);
}

linear_flash_pccard_16mb_device::linear_flash_pccard_16mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	linear_flash_pccard_device(mconfig, LINEAR_FLASH_PCCARD_16MB, tag, owner, clock)
{
	m_space_config = address_space_config("memory", ENDIANNESS_LITTLE, 16,  26, 0, address_map_constructor(FUNC(linear_flash_pccard_16mb_device::linear_flash_pccard_16mb), this));
}

MACHINE_CONFIG_START(linear_flash_pccard_16mb_device::device_add_mconfig)
	MCFG_FUJITSU_29F016A_ADD("1l")
	MCFG_FUJITSU_29F016A_ADD("1u")
	MCFG_FUJITSU_29F016A_ADD("2l")
	MCFG_FUJITSU_29F016A_ADD("2u")
	MCFG_FUJITSU_29F016A_ADD("3l")
	MCFG_FUJITSU_29F016A_ADD("3u")
	MCFG_FUJITSU_29F016A_ADD("4l")
	MCFG_FUJITSU_29F016A_ADD("4u")
MACHINE_CONFIG_END


void linear_flash_pccard_32mb_device::linear_flash_pccard_32mb(address_map &map)
{
	map.unmap_value_high();
	map(0x00000000, 0x003fffff).rw("1l", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);
	map(0x00000000, 0x003fffff).rw("1u", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0xff00);
	map(0x00400000, 0x007fffff).rw("2l", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);
	map(0x00400000, 0x007fffff).rw("2u", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0xff00);
	map(0x00800000, 0x00bfffff).rw("3l", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);
	map(0x00800000, 0x00bfffff).rw("3u", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0xff00);
	map(0x00c00000, 0x00ffffff).rw("4l", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);
	map(0x00c00000, 0x00ffffff).rw("4u", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0xff00);
	map(0x01000000, 0x013fffff).rw("5l", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);
	map(0x01000000, 0x013fffff).rw("5u", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0xff00);
	map(0x01400000, 0x017fffff).rw("6l", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);
	map(0x01400000, 0x017fffff).rw("6u", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0xff00);
	map(0x01800000, 0x01bfffff).rw("7l", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);
	map(0x01800000, 0x01bfffff).rw("7u", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0xff00);
	map(0x01c00000, 0x01ffffff).rw("8l", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);
	map(0x01c00000, 0x01ffffff).rw("8u", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0xff00);
}

linear_flash_pccard_32mb_device::linear_flash_pccard_32mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	linear_flash_pccard_device(mconfig, LINEAR_FLASH_PCCARD_32MB, tag, owner, clock)
{
	m_space_config = address_space_config("memory", ENDIANNESS_LITTLE, 16,  26, 0, address_map_constructor(FUNC(linear_flash_pccard_32mb_device::linear_flash_pccard_32mb), this));
}

MACHINE_CONFIG_START(linear_flash_pccard_32mb_device::device_add_mconfig)
	MCFG_FUJITSU_29F016A_ADD("1l")
	MCFG_FUJITSU_29F016A_ADD("1u")
	MCFG_FUJITSU_29F016A_ADD("2l")
	MCFG_FUJITSU_29F016A_ADD("2u")
	MCFG_FUJITSU_29F016A_ADD("3l")
	MCFG_FUJITSU_29F016A_ADD("3u")
	MCFG_FUJITSU_29F016A_ADD("4l")
	MCFG_FUJITSU_29F016A_ADD("4u")
	MCFG_FUJITSU_29F016A_ADD("5l")
	MCFG_FUJITSU_29F016A_ADD("5u")
	MCFG_FUJITSU_29F016A_ADD("6l")
	MCFG_FUJITSU_29F016A_ADD("6u")
	MCFG_FUJITSU_29F016A_ADD("7l")
	MCFG_FUJITSU_29F016A_ADD("7u")
	MCFG_FUJITSU_29F016A_ADD("8l")
	MCFG_FUJITSU_29F016A_ADD("8u")
MACHINE_CONFIG_END


void linear_flash_pccard_64mb_device::linear_flash_pccard_64mb(address_map &map)
{
	map.unmap_value_high();
	map(0x00000000, 0x003fffff).rw("1l", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);
	map(0x00000000, 0x003fffff).rw("1u", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0xff00);
	map(0x00400000, 0x007fffff).rw("2l", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);
	map(0x00400000, 0x007fffff).rw("2u", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0xff00);
	map(0x00800000, 0x00bfffff).rw("3l", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);
	map(0x00800000, 0x00bfffff).rw("3u", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0xff00);
	map(0x00c00000, 0x00ffffff).rw("4l", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);
	map(0x00c00000, 0x00ffffff).rw("4u", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0xff00);
	map(0x01000000, 0x013fffff).rw("5l", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);
	map(0x01000000, 0x013fffff).rw("5u", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0xff00);
	map(0x01400000, 0x017fffff).rw("6l", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);
	map(0x01400000, 0x017fffff).rw("6u", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0xff00);
	map(0x01800000, 0x01bfffff).rw("7l", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);
	map(0x01800000, 0x01bfffff).rw("7u", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0xff00);
	map(0x01c00000, 0x01ffffff).rw("8l", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);
	map(0x01c00000, 0x01ffffff).rw("8u", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0xff00);
	map(0x02000000, 0x023fffff).rw("9l", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);
	map(0x02000000, 0x023fffff).rw("9u", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0xff00);
	map(0x02400000, 0x027fffff).rw("10l", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);
	map(0x02400000, 0x027fffff).rw("10u", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0xff00);
	map(0x02800000, 0x02bfffff).rw("11l", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);
	map(0x02800000, 0x02bfffff).rw("11u", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0xff00);
	map(0x02c00000, 0x02ffffff).rw("12l", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);
	map(0x02c00000, 0x02ffffff).rw("12u", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0xff00);
	map(0x03000000, 0x033fffff).rw("13l", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);
	map(0x03000000, 0x033fffff).rw("13u", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0xff00);
	map(0x03400000, 0x037fffff).rw("14l", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);
	map(0x03400000, 0x037fffff).rw("14u", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0xff00);
	map(0x03800000, 0x03bfffff).rw("15l", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);
	map(0x03800000, 0x03bfffff).rw("15u", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0xff00);
	map(0x03c00000, 0x03ffffff).rw("16l", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0x00ff);
	map(0x03c00000, 0x03ffffff).rw("16u", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write)).umask16(0xff00);
}

linear_flash_pccard_64mb_device::linear_flash_pccard_64mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	linear_flash_pccard_device(mconfig, LINEAR_FLASH_PCCARD_64MB, tag, owner, clock)
{
	m_space_config = address_space_config("memory", ENDIANNESS_LITTLE, 16,  26, 0, address_map_constructor(FUNC(linear_flash_pccard_64mb_device::linear_flash_pccard_64mb), this));
}

MACHINE_CONFIG_START(linear_flash_pccard_64mb_device::device_add_mconfig)
	MCFG_FUJITSU_29F016A_ADD("1l")
	MCFG_FUJITSU_29F016A_ADD("1u")
	MCFG_FUJITSU_29F016A_ADD("2l")
	MCFG_FUJITSU_29F016A_ADD("2u")
	MCFG_FUJITSU_29F016A_ADD("3l")
	MCFG_FUJITSU_29F016A_ADD("3u")
	MCFG_FUJITSU_29F016A_ADD("4l")
	MCFG_FUJITSU_29F016A_ADD("4u")
	MCFG_FUJITSU_29F016A_ADD("5l")
	MCFG_FUJITSU_29F016A_ADD("5u")
	MCFG_FUJITSU_29F016A_ADD("6l")
	MCFG_FUJITSU_29F016A_ADD("6u")
	MCFG_FUJITSU_29F016A_ADD("7l")
	MCFG_FUJITSU_29F016A_ADD("7u")
	MCFG_FUJITSU_29F016A_ADD("8l")
	MCFG_FUJITSU_29F016A_ADD("8u")
	MCFG_FUJITSU_29F016A_ADD("9l")
	MCFG_FUJITSU_29F016A_ADD("9u")
	MCFG_FUJITSU_29F016A_ADD("10l")
	MCFG_FUJITSU_29F016A_ADD("10u")
	MCFG_FUJITSU_29F016A_ADD("11l")
	MCFG_FUJITSU_29F016A_ADD("11u")
	MCFG_FUJITSU_29F016A_ADD("12l")
	MCFG_FUJITSU_29F016A_ADD("12u")
	MCFG_FUJITSU_29F016A_ADD("13l")
	MCFG_FUJITSU_29F016A_ADD("13u")
	MCFG_FUJITSU_29F016A_ADD("14l")
	MCFG_FUJITSU_29F016A_ADD("14u")
	MCFG_FUJITSU_29F016A_ADD("15l")
	MCFG_FUJITSU_29F016A_ADD("15u")
	MCFG_FUJITSU_29F016A_ADD("16l")
	MCFG_FUJITSU_29F016A_ADD("16u")
MACHINE_CONFIG_END
