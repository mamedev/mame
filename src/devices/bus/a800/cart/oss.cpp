// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Angelo Salese
/**************************************************************************************************

 "Optimized System Software" OSS ROM cart emulation

https://en.wikipedia.org/wiki/Optimized_Systems_Software

Running on similar non-linear but incompatible banking schemes.

TODO:
- OSS034M doesn't really exist, it's an OSS043M with wrong ROM contents.

**************************************************************************************************/


#include "emu.h"
#include "oss.h"


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(A800_ROM_OSS8K, a800_rom_oss8k_device,   "a800_oss8k", "Atari 8-bit OSS 8K cart")
DEFINE_DEVICE_TYPE(A800_ROM_OSS34, a800_rom_oss034m_device, "a800_034m",  "Atari 8-bit OSS-034M cart")
DEFINE_DEVICE_TYPE(A800_ROM_OSS43, a800_rom_oss043m_device, "a800_043m",  "Atari 8-bit OSS-043M cart")
DEFINE_DEVICE_TYPE(A800_ROM_OSS91, a800_rom_oss091m_device, "a800_m091",  "Atari 8-bit OSS-M091 cart")

/*-------------------------------------------------

 OSS 8K

 This is used by The Writer's Tool only.

 -------------------------------------------------*/

a800_rom_oss8k_device::a800_rom_oss8k_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_device(mconfig, type, tag, owner, clock)
	, m_bank(0)
{
}

a800_rom_oss8k_device::a800_rom_oss8k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_oss8k_device(mconfig, A800_ROM_OSS8K, tag, owner, clock)
{
}

void a800_rom_oss8k_device::device_start()
{
	save_item(NAME(m_bank));
}

void a800_rom_oss8k_device::device_reset()
{
	m_bank = 0;
}

void a800_rom_oss8k_device::cart_map(address_map &map)
{
	map(0x2000, 0x2fff).lr8(
		NAME([this](offs_t offset) { return m_rom[(offset & 0x0fff) | m_bank * 0x1000]; })
	);
	map(0x3000, 0x3fff).lr8(
		NAME([this](offs_t offset) { return m_rom[offset & 0x0fff]; })
	);
}

/*
 * ---- 0--x selects bank 1
 * ---- 1--0 RD5 clear
 * ---- 1--1 selects bank 0
 */
inline void a800_rom_oss8k_device::bank_config_access(offs_t offset)
{
	if (!(BIT(offset, 3)))
	{
		rd5_w(1);
		m_bank = 1;
	}
	else
	{
		rd5_w(BIT(offset, 0));
		m_bank = 0;
	}
}

u8 a800_rom_oss8k_device::rom_bank_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		bank_config_access(offset);
	return 0xff;
}

void a800_rom_oss8k_device::rom_bank_w(offs_t offset, u8 data)
{
	bank_config_access(offset);
}

void a800_rom_oss8k_device::cctl_map(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(a800_rom_oss8k_device::rom_bank_r), FUNC(a800_rom_oss8k_device::rom_bank_w));
}

/*-------------------------------------------------

 OSS 091M

 Later variant of OSS 8k

 -------------------------------------------------*/

a800_rom_oss091m_device::a800_rom_oss091m_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_oss8k_device(mconfig, A800_ROM_OSS91, tag, owner, clock)
{
}

/*
 * ---- 0--0 bank 1
 * ---- 0--1 bank 3
 * ---- 1--0 RD5 clear
 * ---- 1--1 bank 2
 */
inline void a800_rom_oss091m_device::bank_config_access(offs_t offset)
{
	const u8 a0 = BIT(offset, 0);
	if (!(BIT(offset, 3)))
	{
		rd5_w(1);
		m_bank = 1 | (a0 << 1);
	}
	else
	{
		rd5_w(a0);
		m_bank = 2;
	}
}

/*-------------------------------------------------

 OSS 043M

 -------------------------------------------------*/

a800_rom_oss043m_device::a800_rom_oss043m_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_device(mconfig, type, tag, owner, clock)
	, m_bankdev(*this, "bankdev")
	, m_bank_base1(0)
	, m_bank_base2(0)
{
}

a800_rom_oss043m_device::a800_rom_oss043m_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_oss043m_device(mconfig, A800_ROM_OSS43, tag, owner, clock)
{
}

void a800_rom_oss043m_device::device_add_mconfig(machine_config &config)
{
	ADDRESS_MAP_BANK(config, m_bankdev).set_map(&a800_rom_oss043m_device::bankdev_map).set_options(ENDIANNESS_LITTLE, 8, 12 + 3, 0x1000);
}

void a800_rom_oss043m_device::device_start()
{
	m_bank_base1 = 0x2000;
	m_bank_base2 = 0x1000;
}

void a800_rom_oss043m_device::device_reset()
{
	m_bankdev->set_bank(0);

	rd4_w(0);
	rd5_w(1);
}

void a800_rom_oss043m_device::bankdev_map(address_map &map)
{
	map(0x0000, 0x0fff).lr8(
		NAME([this](offs_t offset) { return m_rom[offset & 0x0fff]; })
	);
	map(0x1000, 0x1fff).lr8(
		NAME([this](offs_t offset) { return m_rom[offset & 0x0fff] & m_rom[(offset & 0x0fff) | m_bank_base1]; })
	);
	map(0x2000, 0x2fff).mirror(0x4000).lr8(
		NAME([]() { return 0xff; })
	);
	map(0x3000, 0x3fff).mirror(0x4000).lr8(
		NAME([this](offs_t offset) { return m_rom[(offset & 0x0fff) | m_bank_base1]; })
	);
	map(0x4000, 0x4fff).lr8(
		NAME([this](offs_t offset) { return m_rom[(offset & 0x0fff) | m_bank_base2]; })
	);
	map(0x5000, 0x5fff).lr8(
		NAME([this](offs_t offset) { return m_rom[(offset & 0x0fff) | m_bank_base1] & m_rom[(offset & 0x0fff) | m_bank_base2]; })
	);
}

void a800_rom_oss043m_device::cart_map(address_map &map)
{
	map(0x2000, 0x2fff).m(m_bankdev, FUNC(address_map_bank_device::amap8));
	map(0x3000, 0x3fff).lr8(
		NAME([this](offs_t offset) { return m_rom[(offset & 0x0fff) | 0x3000]; })
	);
}

void a800_rom_oss043m_device::cctl_map(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(a800_rom_oss043m_device::rom_bank_r), FUNC(a800_rom_oss043m_device::rom_bank_w));
}

/*
 * 0000 bank 0
 * 0001 returns AND-ed contents of banks 0 & 2
 * 0x10 returns 0xff to window (i.e. RD5 is still asserted)
 * 0x11 bank 2
 * 0100 bank 1
 * 0101 returns AND-ed contents of banks 1 & 2
 * 1000 RD5 clear
 */
inline void a800_rom_oss043m_device::bank_config_access(offs_t offset)
{
	rd5_w(!(BIT(offset, 3)));
	m_bankdev->set_bank(offset & 7);
}

u8 a800_rom_oss043m_device::rom_bank_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		bank_config_access(offset);
	return 0xff;
}

void a800_rom_oss043m_device::rom_bank_w(offs_t offset, u8 data)
{
	bank_config_access(offset);
}

/*-------------------------------------------------

 OSS 034M (fake)

 -------------------------------------------------*/

a800_rom_oss034m_device::a800_rom_oss034m_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_oss043m_device(mconfig, A800_ROM_OSS34, tag, owner, clock)
{
}

void a800_rom_oss034m_device::device_start()
{
	a800_rom_oss043m_device::device_start();
	m_bank_base1 = 0x1000;
	m_bank_base2 = 0x2000;
}
