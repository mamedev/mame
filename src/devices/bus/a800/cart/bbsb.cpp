// license: BSD-3-Clause
// copyright-holders: Fabio Priuli, Angelo Salese
/**************************************************************************************************

A800/A5200 Bounty Bob Strikes Back ROM schemes

Dual banking overlay access mapped in cart_map rd4 space at $xff6-$xff9,
A12 select bank window, bank bases are different.
rd5 just points to the last bank.

A5200 BBSB is mostly similar to the A800 version but with shuffled around data banks
i.e. fixed bank is actually the first one, and the overlay banks comes afterwards in the ROM space.

TODO:
- It's unknown if rd4 and rd5 can even be disabled from software (probably not);

**************************************************************************************************/

#include "emu.h"
#include "bbsb.h"

DEFINE_DEVICE_TYPE(A800_ROM_BBSB,      a800_rom_bbsb_device,      "a800_bbsb",     "Atari 8-bit Bounty Bob Strikes Back cart")
DEFINE_DEVICE_TYPE(A5200_ROM_BBSB,     a5200_rom_bbsb_device,     "a5200_bbsb",    "Atari 5200 ROM Bounty Bob Strikes Back cart")


a800_rom_bbsb_device::a800_rom_bbsb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_device(mconfig, A800_ROM_BBSB, tag, owner, clock)
{
}


void a800_rom_bbsb_device::device_start()
{
	// TODO: m_banks should be an extra interface for a5200
	save_item(NAME(m_banks));
}

void a800_rom_bbsb_device::device_reset()
{
	m_banks[0] = 0;
	m_banks[1] = 0;
}

template <unsigned BankNum> u8 a800_rom_bbsb_device::read_bank(offs_t offset)
{
	return m_rom[(offset & 0xfff) + (m_banks[BankNum] * 0x1000) + (BankNum * 0x4000)];
}

void a800_rom_bbsb_device::cart_map(address_map &map)
{
	map(0x0000, 0x0fff).r(FUNC(a800_rom_bbsb_device::read_bank<0>));
	map(0x1000, 0x1fff).r(FUNC(a800_rom_bbsb_device::read_bank<1>));
	map(0x0ff6, 0x0ff9).select(0x1000).rw(FUNC(a800_rom_bbsb_device::bank_r), FUNC(a800_rom_bbsb_device::bank_w));

	map(0x2000, 0x3fff).lr8(
		NAME([this](offs_t offset) { return m_rom[(offset & 0x1fff) + 0x8000]; } )
	);
}

uint8_t a800_rom_bbsb_device::bank_r(offs_t offset)
{
	const u8 bank_window = (offset & ~3) != 0;
	const u8 bank_num = offset & 3;
	if (!machine().side_effects_disabled())
		m_banks[bank_window] = bank_num;

	// TODO: unconfirmed, all $xff6-$xff9 just points to 0xff values
	// Real cart overlay may return floating bus or the contents of the previous bank instead,
	// needs to be tested by trying to execute code from there.
	return 0xff;
}

void a800_rom_bbsb_device::bank_w(offs_t offset, uint8_t data)
{
	const u8 bank_window = (offset & ~3) != 0;
	const u8 bank_num = offset & 3;
	m_banks[bank_window] = bank_num;
}

/*-------------------------------------------------

 A5200 Bounty Bob Strikes Back! cart

 -------------------------------------------------*/

a5200_rom_bbsb_device::a5200_rom_bbsb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a5200_rom_device(mconfig, A5200_ROM_BBSB, tag, owner, clock)
{
}

void a5200_rom_bbsb_device::device_start()
{
	save_item(NAME(m_banks));
}

void a5200_rom_bbsb_device::device_reset()
{
	m_banks[0] = 0;
	m_banks[1] = 0;
}

template <unsigned BankNum> u8 a5200_rom_bbsb_device::read_bank(offs_t offset)
{
	return m_rom[(offset & 0xfff) + (m_banks[BankNum] * 0x1000) + (BankNum ? 0x6000 : 0x2000)];
}

void a5200_rom_bbsb_device::cart_map(address_map &map)
{
	map(0x0000, 0x0fff).r(FUNC(a5200_rom_bbsb_device::read_bank<0>));
	map(0x1000, 0x1fff).r(FUNC(a5200_rom_bbsb_device::read_bank<1>));
	map(0x0ff6, 0x0ff9).select(0x1000).rw(FUNC(a5200_rom_bbsb_device::bank_r), FUNC(a5200_rom_bbsb_device::bank_w));

	map(0x4000, 0x7fff).lr8(
		NAME([this](offs_t offset) { return m_rom[offset & 0x1fff]; } )
	);
}

uint8_t a5200_rom_bbsb_device::bank_r(offs_t offset)
{
	const u8 bank_window = (offset & ~3) != 0;
	const u8 bank_num = offset & 3;
	if (!machine().side_effects_disabled())
		m_banks[bank_window] = bank_num;

	// TODO: unconfirmed, all $xff6-$xff9 just points to 0xff values
	// Real cart overlay may return floating bus or the contents of the previous bank instead,
	// needs to be tested by trying to execute code from there.
	return 0xff;
}

void a5200_rom_bbsb_device::bank_w(offs_t offset, uint8_t data)
{
	const u8 bank_window = (offset & ~3) != 0;
	const u8 bank_num = offset & 3;
	m_banks[bank_window] = bank_num;
}
