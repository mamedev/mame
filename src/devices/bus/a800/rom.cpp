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

DEFINE_DEVICE_TYPE(A800_ROM,           a800_rom_device,           "a800_rom",      "Atari 800 ROM Carts")
DEFINE_DEVICE_TYPE(XEGS_ROM,           xegs_rom_device,           "a800_xegs",     "Atari XEGS 64K ROM Carts")
DEFINE_DEVICE_TYPE(A5200_ROM_2CHIPS,   a5200_rom_2chips_device,   "a5200_16k2c",   "Atari 5200 ROM Cart 16K in 2 Chips")
DEFINE_DEVICE_TYPE(A5200_ROM_BBSB,     a5200_rom_bbsb_device,     "a5200_bbsb",    "Atari 5200 ROM Cart BBSB")


a800_rom_device::a800_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_a800_cart_interface( mconfig, *this )
{
}

a800_rom_device::a800_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_device(mconfig, A800_ROM, tag, owner, clock)
{
}


xegs_rom_device::xegs_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_device(mconfig, XEGS_ROM, tag, owner, clock)
	, m_bank(0)
{
}


a5200_rom_2chips_device::a5200_rom_2chips_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_device(mconfig, A5200_ROM_2CHIPS, tag, owner, clock)
{
}


a5200_rom_bbsb_device::a5200_rom_bbsb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_device(mconfig, A5200_ROM_BBSB, tag, owner, clock)
{
}

void xegs_rom_device::device_start()
{
	save_item(NAME(m_bank));
}

void xegs_rom_device::device_reset()
{
	m_bank = 0;
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



/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Carts with no bankswitch (8K, 16K)

 The cart accessors are mapped in the correct
 range at driver start

 -------------------------------------------------*/

void a800_rom_device::device_start()
{
}

void a800_rom_device::device_reset()
{
	rd4_w(0);
	rd5_w(1);
}

void a800_rom_device::cart_map(address_map &map)
{
	map(0x2000, 0x3fff).lr8(
		NAME([this](offs_t offset) { return m_rom[offset & (m_rom_size - 1)]; })
	);
}

/*-------------------------------------------------

 XEGS carts (32K, 64K or 128K)

 Bankswitch is controlled by data written in
 0xd500-0xd5ff

 -------------------------------------------------*/

uint8_t xegs_rom_device::read_80xx(offs_t offset)
{
	if (offset < 0x2000)
		return m_rom[(offset & 0x1fff) + (m_bank * 0x2000)];
	else
		return m_rom[(offset & 0x1fff) + (m_bank_mask * 0x2000)];   // always last 8K bank

}

void xegs_rom_device::write_d5xx(offs_t offset, uint8_t data)
{
	m_bank = data & m_bank_mask;
}

// Atari 5200


/*-------------------------------------------------

 Carts with no bankswitch (4K, 8K, 16K, 32K)

 Same as base carts above

 -------------------------------------------------*/

/*-------------------------------------------------

 Carts with 2x8K (16K) with A13 line not connected

 Range 0x4000-0x7fff contains two copies of the low
 8K, range 0x8000-0xbfff contains two copies of the
 high 8K

 -------------------------------------------------*/

uint8_t a5200_rom_2chips_device::read_80xx(offs_t offset)
{
	if (offset < 0x4000)
		return m_rom[offset & 0x1fff];
	else
		return m_rom[(offset & 0x1fff) + 0x2000];
}


/*-------------------------------------------------

 Bounty Bob Strikes Back! cart (40K)

 Similar to the A800 version, but:
 Area 0x8000-0xbfff always point to last 8K bank
 (repeated twice)
 Areas 0x4000-0x4fff and 0x5000-0x5fff are
 separate banks of 4K mapped either in the first
 16K chunk or in the second 16K chunk
 Bankswitch is controlled by data written in
 0x4000-0x4fff and 0x5000-0x5fff respectively

 -------------------------------------------------*/

uint8_t a5200_rom_bbsb_device::read_80xx(offs_t offset)
{
	if ((offset & 0xe000) == 0 && !machine().side_effects_disabled())
	{
		uint16_t addr = offset & 0xfff;
		if (addr >= 0xff6 && addr <= 0xff9)
			m_banks[BIT(offset, 12)] = (addr - 0xff6);
	}

	if (offset < 0x1000)
		return m_rom[(offset & 0xfff) + (m_banks[0] * 0x1000) + 0x2000];
	else if (offset < 0x2000)
		return m_rom[(offset & 0xfff) + (m_banks[1] * 0x1000) + 0x6000];
	else if (offset >= 0x4000)
		return m_rom[(offset & 0x1fff) + 0x0000];
	else
		return 0;
}

void a5200_rom_bbsb_device::write_80xx(offs_t offset, uint8_t data)
{
	uint16_t addr = offset & 0xfff;
	if (addr >= 0xff6 && addr <= 0xff9)
		m_banks[BIT(offset, 12)] = (addr - 0xff6);
}
