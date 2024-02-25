// license: BSD-3-Clause
// copyright-holders: Fabio Priuli, Angelo Salese
/**************************************************************************************************

Williams cart scheme and variants

- Regular Williams cart maps ROM bankswitch to $x0 - $x7, RD5 disables with A3. x is mirrored.
- Express carts maps x = $7x, bank map is xor-ed
- Diamond carts maps x = $dx, bank map is xor-ed
- Turbosoft carts raises number of banks to 16, moves RD5 disable to A4 instead of A3
- All CCTL accesses works both on reading and writing.

**************************************************************************************************/

#include "emu.h"
#include "williams.h"

DEFINE_DEVICE_TYPE(A800_ROM_WILLIAMS,  a800_rom_williams_device,  "a800_williams", "Atari 8-bit Williams cart")
DEFINE_DEVICE_TYPE(A800_ROM_EXPRESS,   a800_rom_express_device,   "a800_express",  "Atari 8-bit Express cart")
DEFINE_DEVICE_TYPE(A800_ROM_DIAMOND,   a800_rom_diamond_device,   "a800_diamond",  "Atari 8-bit Diamond cart")
DEFINE_DEVICE_TYPE(A800_ROM_TURBO,     a800_rom_turbo_device,     "a800_turbo",    "Atari 8-bit Turbosoft 64KB/128KB cart")

a800_rom_williams_device::a800_rom_williams_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_device(mconfig, type, tag, owner, clock)
	, m_bank(0)
{
}

a800_rom_williams_device::a800_rom_williams_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_williams_device(mconfig, A800_ROM_WILLIAMS, tag, owner, clock)
{
}

void a800_rom_williams_device::device_start()
{
	save_item(NAME(m_bank));
}

void a800_rom_williams_device::device_reset()
{
	// turboc1 (at least) reads ROM window without setting bank first,
	// any non-zero value will make it to punt
	m_bank = 0;
}

void a800_rom_williams_device::cart_map(address_map &map)
{
	map(0x2000, 0x3fff).lr8(
		NAME([this](offs_t offset) { return m_rom[(offset & 0x1fff) + (m_bank * 0x2000)]; })
	);
}

void a800_rom_williams_device::cctl_map(address_map &map)
{
	map(0x00, 0x07).mirror(0xf0).rw(FUNC(a800_rom_williams_device::rom_bank_r), FUNC(a800_rom_williams_device::rom_bank_w));
	map(0x08, 0x0f).mirror(0xf0).rw(FUNC(a800_rom_williams_device::disable_rom_r), FUNC(a800_rom_williams_device::disable_rom_w));
}

uint8_t a800_rom_williams_device::disable_rom_r(offs_t offset)
{
	if(!machine().side_effects_disabled())
		rd5_w(0);

	return 0xff;
}

void a800_rom_williams_device::disable_rom_w(offs_t offset, uint8_t data)
{
	rd5_w(0);
}

// m_bank_mask necessary for turbo128 carts
uint8_t a800_rom_williams_device::rom_bank_r(offs_t offset)
{
	if(!machine().side_effects_disabled())
	{
		rd5_w(1);
		m_bank = (offset & m_bank_mask);
	}
	return 0xff;
}

void a800_rom_williams_device::rom_bank_w(offs_t offset, uint8_t data)
{
	rd5_w(1);
	m_bank = (offset & m_bank_mask);
}

/*-------------------------------------------------

 Express 64K carts

 -------------------------------------------------*/

a800_rom_express_device::a800_rom_express_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_williams_device(mconfig, type, tag, owner, clock)
{
}

a800_rom_express_device::a800_rom_express_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_williams_device(mconfig, A800_ROM_EXPRESS, tag, owner, clock)
{
}

void a800_rom_express_device::cctl_map(address_map &map)
{
	map(0x70, 0x77).rw(FUNC(a800_rom_express_device::rom_bank_r), FUNC(a800_rom_express_device::rom_bank_w));
	map(0x78, 0x7f).rw(FUNC(a800_rom_express_device::disable_rom_r), FUNC(a800_rom_express_device::disable_rom_w));
}

uint8_t a800_rom_express_device::rom_bank_r(offs_t offset)
{
	if(!machine().side_effects_disabled())
	{
		rd5_w(1);
		m_bank = ((offset ^ m_bank_mask) & m_bank_mask);
	}
	return 0xff;
}

void a800_rom_express_device::rom_bank_w(offs_t offset, uint8_t data)
{
	rd5_w(1);
	m_bank = ((offset ^ m_bank_mask) & m_bank_mask);
}

/*-------------------------------------------------

 Diamond 64K carts

 -------------------------------------------------*/


a800_rom_diamond_device::a800_rom_diamond_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_express_device(mconfig, A800_ROM_DIAMOND, tag, owner, clock)
{
}

void a800_rom_diamond_device::cctl_map(address_map &map)
{
	map(0xd0, 0xd7).rw(FUNC(a800_rom_diamond_device::rom_bank_r), FUNC(a800_rom_diamond_device::rom_bank_w));
	map(0xd8, 0xdf).rw(FUNC(a800_rom_diamond_device::disable_rom_r), FUNC(a800_rom_diamond_device::disable_rom_w));
}

/*-------------------------------------------------

 Turbosoft 64K / 128K

 -------------------------------------------------*/

a800_rom_turbo_device::a800_rom_turbo_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_williams_device(mconfig, A800_ROM_TURBO, tag, owner, clock)
{
}

void a800_rom_turbo_device::cctl_map(address_map &map)
{
	map(0x00, 0x0f).mirror(0xe0).rw(FUNC(a800_rom_turbo_device::rom_bank_r), FUNC(a800_rom_turbo_device::rom_bank_w));
	map(0x10, 0x1f).mirror(0xe0).rw(FUNC(a800_rom_turbo_device::disable_rom_r), FUNC(a800_rom_turbo_device::disable_rom_w));
}
