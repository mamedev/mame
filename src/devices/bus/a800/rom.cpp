// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Angelo Salese
/**************************************************************************************************

 A800/A5200/XEGS ROM base cart emulation

**************************************************************************************************/

#include "emu.h"
#include "rom.h"

//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(A800_ROM,           a800_rom_device,           "a800_rom",      "Atari 8-bit ROM cart")
DEFINE_DEVICE_TYPE(A800_ROM_16KB,      a800_rom_16kb_device,      "a800_rom_16kb", "Atari 8-bit ROM 16kb cart")
DEFINE_DEVICE_TYPE(A800_ROM_RIGHT,     a800_rom_right_device,     "a800_rom_right","Atari 8-bit ROM Right cart")
DEFINE_DEVICE_TYPE(XEGS_ROM,           xegs_rom_device,           "a800_xegs",     "Atari XEGS 64K cart")

DEFINE_DEVICE_TYPE(A5200_ROM,          a5200_rom_device,          "a5200_rom",     "Atari 5200 ROM cart")
DEFINE_DEVICE_TYPE(A5200_ROM_2CHIPS,   a5200_rom_2chips_device,   "a5200_16k2c",   "Atari 5200 ROM cart 16K in 2 Chips")


a800_rom_device::a800_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_a800_cart_interface( mconfig, *this )
{
}

/*-------------------------------------------------

 Generic left cart 8kb

 -------------------------------------------------*/

a800_rom_device::a800_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_device(mconfig, A800_ROM, tag, owner, clock)
{
}

void a800_rom_device::device_start()
{
}

void a800_rom_device::device_reset()
{
}

void a800_rom_device::cart_map(address_map &map)
{
	map(0x2000, 0x3fff).lr8(
		NAME([this](offs_t offset) { return m_rom[offset & (m_rom_size - 1)]; })
	);
}

/*-------------------------------------------------

 Generic right cart 8kb

 -------------------------------------------------*/

a800_rom_right_device::a800_rom_right_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_device(mconfig, A800_ROM_RIGHT, tag, owner, clock)
{
}

void a800_rom_right_device::device_start()
{
}

void a800_rom_right_device::device_reset()
{
	rd4_w(1);
	rd5_w(0);
}

void a800_rom_right_device::cart_map(address_map &map)
{
	map(0x0000, 0x1fff).lr8(
		NAME([this](offs_t offset) { return m_rom[offset & (m_rom_size - 1)]; })
	);
}

/*-------------------------------------------------

 Generic 16kb RD4 + RD5 cart

 -------------------------------------------------*/

a800_rom_16kb_device::a800_rom_16kb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_device(mconfig, A800_ROM_16KB, tag, owner, clock)
{
}

void a800_rom_16kb_device::device_start()
{
}

void a800_rom_16kb_device::device_reset()
{
	rd_both_w(1);
}

void a800_rom_16kb_device::cart_map(address_map &map)
{
	map(0x0000, 0x3fff).lr8(
		NAME([this](offs_t offset) { return m_rom[offset & (m_rom_size - 1)]; })
	);
}

/*-------------------------------------------------

 XEGS carts (32K, 64K or 128K)

 -------------------------------------------------*/

xegs_rom_device::xegs_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_device(mconfig, XEGS_ROM, tag, owner, clock)
	, m_bank(0)
{
}

void xegs_rom_device::device_start()
{
	save_item(NAME(m_bank));
}

void xegs_rom_device::device_reset()
{
	// TODO: random
	m_bank = 0;
}

// RD5 always maps to the last bank
void xegs_rom_device::cart_map(address_map &map)
{
	map(0x0000, 0x1fff).lr8(
		NAME([this](offs_t offset) { return m_rom[(offset & 0x1fff) + (m_bank * 0x2000)]; })
	);
	map(0x2000, 0x3fff).lr8(
		NAME([this](offs_t offset) { return m_rom[(offset & 0x1fff) + (m_bank_mask * 0x2000)]; })
	);
}

void xegs_rom_device::cctl_map(address_map &map)
{
	map(0x00, 0xff).lw8(
		NAME([this](offs_t offset, u8 data) { m_bank = data & m_bank_mask; })
	);
}


// Atari 5200


/*-------------------------------------------------

 Carts with no bankswitch (4K, 8K, 16K, 32K)

 Same as base carts above

 -------------------------------------------------*/

a5200_rom_device::a5200_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_a5200_cart_interface( mconfig, *this )
{
}

a5200_rom_device::a5200_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a5200_rom_device(mconfig, A5200_ROM, tag, owner, clock)
{
}

void a5200_rom_device::cart_map(address_map &map)
{
	map(0x0000, 0x7fff).lr8(
		NAME([this](offs_t offset) { return m_rom[(offset & (m_rom_size - 1))]; })
	);
}

void a5200_rom_device::device_start()
{
}

/*-------------------------------------------------

 Carts with 2x8K (16K) with A13 line not connected

 Range 0x4000-0x7fff contains two copies of the low
 8K, range 0x8000-0xbfff contains two copies of the
 high 8K

 -------------------------------------------------*/

a5200_rom_2chips_device::a5200_rom_2chips_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_device(mconfig, A5200_ROM_2CHIPS, tag, owner, clock)
{
}

void a5200_rom_2chips_device::cart_map(address_map &map)
{
	map(0x0000, 0x3fff).lr8(
		NAME([this](offs_t offset) { return m_rom[offset & 0x1fff]; })
	);
	map(0x4000, 0x7fff).lr8(
		NAME([this](offs_t offset) { return m_rom[(offset & 0x1fff) + 0x2000]; })
	);
}
