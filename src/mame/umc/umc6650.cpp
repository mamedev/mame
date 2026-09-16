// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/*************************************************************************************************

[Super A'Can] UMC 6650 lockout chip

Located in every A'Can cartridge

TODO:
- signal to cart B26 & B27 (from register $09?).
- Does the effective lockout resolution input merges with $1c signal from UMC6619 host space?
- /WR for optional cart save RAM

**************************************************************************************************/

#include "emu.h"
#include "umc6650.h"

#define VERBOSE     (1)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(UMC6650, umc6650_device, "umc6650", "UMC UM6650 lockout chip")

umc6650_device::umc6650_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, UMC6650, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_romkey(*this, "romkey")
	, m_space_io_config("io", ENDIANNESS_BIG, 8, 7, 0, address_map_constructor(FUNC(umc6650_device::internal_map), this))
{
}

ROM_START( umc6650 )
	ROM_REGION(0x10, "romkey", ROMREGION_ERASEFF)
	// 68k internal ROM (security related)
	ROM_LOAD( "umc6650.bin", 0x00,  0x10, CRC(0ba78597) SHA1(f94805457976d60b91e8df18f9f49cccec77be78) )
ROM_END

const tiny_rom_entry *umc6650_device::device_rom_region() const
{
	return ROM_NAME( umc6650 );
}

device_memory_interface::space_config_vector umc6650_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_IO, &m_space_io_config)
	};
}

void umc6650_device::device_start()
{
	m_space_io = &space(AS_IO);
	save_item(NAME(m_address));
}

void umc6650_device::device_reset()
{
	m_address = 0x7f;
}

u8 umc6650_device::read(offs_t offset)
{
	return offset == 1 ? m_address : m_space_io->read_byte(m_address);
}

void umc6650_device::write(offs_t offset, u8 data)
{
	if (offset == 1)
		m_address = data & 0x7f;
	else
		m_space_io->write_byte(m_address, data);
}

void umc6650_device::internal_map(address_map &map)
{
//  map(0x09, 0x09)
//  map(0x0c, 0x0c)
	map(0x20, 0x2f).rom().region(m_romkey, 0);
	map(0x40, 0x5f).ram();
}
