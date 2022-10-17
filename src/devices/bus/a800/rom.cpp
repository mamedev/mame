// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************

 A800/A5200/XEGS ROM cart emulation

 Basic carts work the same (in addition of being mostly compatible) for all these systems
 and thus we deal with them in a single file

***********************************************************************************************************/


#include "emu.h"
#include "rom.h"


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(A800_ROM,           a800_rom_device,           "a800_rom",      "Atari 800 ROM Carts")
DEFINE_DEVICE_TYPE(A800_ROM_BBSB,      a800_rom_bbsb_device,      "a800_bbsb",     "Atari 800 ROM Carts BBSB")
DEFINE_DEVICE_TYPE(A800_ROM_WILLIAMS,  a800_rom_williams_device,  "a800_williams", "Atari 800 64K ROM Carts Williams")
DEFINE_DEVICE_TYPE(A800_ROM_EXPRESS,   a800_rom_express_device,   "a800_express",  "Atari 800 64K ROM Carts Express/Diamond")
DEFINE_DEVICE_TYPE(A800_ROM_TURBO,     a800_rom_turbo_device,     "a800_turbo",    "Atari 800 64K ROM Carts Turbosoft")
DEFINE_DEVICE_TYPE(A800_ROM_TELELINK2, a800_rom_telelink2_device, "a800_tlink2",   "Atari 800 64K ROM Cart Telelink II")
DEFINE_DEVICE_TYPE(A800_ROM_MICROCALC, a800_rom_microcalc_device, "a800_sitsa",    "Atari 800 64K ROM Carts SITSA MicroCalc")
DEFINE_DEVICE_TYPE(A800_ROM_CORINA,    a800_rom_corina_device,    "a800_corina",   "Atari 800 ROM Carts Corina 1MB Flash ROM")
DEFINE_DEVICE_TYPE(A800_ROM_CORINA_SRAM, a800_rom_corina_sram_device,    "a800_corina_sram",   "Atari 800 ROM Carts Corina 512KB Flash ROM + 512KB RAM")
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


a800_rom_bbsb_device::a800_rom_bbsb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_device(mconfig, A800_ROM_BBSB, tag, owner, clock)
{
}



xegs_rom_device::xegs_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_device(mconfig, XEGS_ROM, tag, owner, clock)
	, m_bank(0)
{
}


a800_rom_williams_device::a800_rom_williams_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_device(mconfig, A800_ROM_WILLIAMS, tag, owner, clock)
	, m_bank(0)
{
}


a800_rom_express_device::a800_rom_express_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_device(mconfig, A800_ROM_EXPRESS, tag, owner, clock)
	, m_bank(0)
{
}


a800_rom_turbo_device::a800_rom_turbo_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_device(mconfig, A800_ROM_TURBO, tag, owner, clock)
	, m_bank(0)
{
}


a800_rom_telelink2_device::a800_rom_telelink2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_device(mconfig, A800_ROM_TELELINK2, tag, owner, clock)
{
}


a800_rom_microcalc_device::a800_rom_microcalc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_device(mconfig, A800_ROM_MICROCALC, tag, owner, clock)
	, m_bank(0)
{
}

a800_rom_corina_device::a800_rom_corina_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_device(mconfig, type, tag, owner, clock)
	, m_rom_bank(0)
	, m_view_select(0)
{
}

a800_rom_corina_device::a800_rom_corina_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_device(mconfig, A800_ROM_CORINA, tag, owner, clock)
{
}

a800_rom_corina_sram_device::a800_rom_corina_sram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a800_rom_corina_device(mconfig, A800_ROM_CORINA_SRAM, tag, owner, clock)
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




void a800_rom_device::device_start()
{
}

void a800_rom_device::device_reset()
{
}


void a800_rom_bbsb_device::device_start()
{
	save_item(NAME(m_banks));
}

void a800_rom_bbsb_device::device_reset()
{
	m_banks[0] = 0;
	m_banks[1] = 0;
}


void xegs_rom_device::device_start()
{
	save_item(NAME(m_bank));
}

void xegs_rom_device::device_reset()
{
	m_bank = 0;
}


void a800_rom_williams_device::device_start()
{
	save_item(NAME(m_bank));
}

void a800_rom_williams_device::device_reset()
{
	m_bank = 0;
}


void a800_rom_express_device::device_start()
{
	save_item(NAME(m_bank));
}

void a800_rom_express_device::device_reset()
{
	m_bank = 0;
}


void a800_rom_turbo_device::device_start()
{
	save_item(NAME(m_bank));
}

void a800_rom_turbo_device::device_reset()
{
	m_bank = 0;
}


void a800_rom_microcalc_device::device_start()
{
	save_item(NAME(m_bank));
}

void a800_rom_microcalc_device::device_reset()
{
	m_bank = 0;
}


void a800_rom_corina_device::device_start()
{
	save_item(NAME(m_rom_bank));
	save_item(NAME(m_view_select));
}

void a800_rom_corina_device::device_reset()
{
	m_rom_bank = 0;
	m_view_select = 0;
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

uint8_t a800_rom_device::read_80xx(offs_t offset)
{
	return m_rom[offset & (m_rom_size - 1)];
}



/*-------------------------------------------------

 Bounty Bob Strikes Back! cart (40K)

 Area 0xa000-0xbfff always point to last 8K bank
 Areas 0x8000-0x8fff and 0x9000-0x9fff are
 separate banks of 4K mapped either in the first
 16K chunk or in the second 16K chunk
 Bankswitch is controlled by data written in
 0x8000-0x8fff and 0x9000-0x9fff respectively

 -------------------------------------------------*/

uint8_t a800_rom_bbsb_device::read_80xx(offs_t offset)
{
	if ((offset & 0x2000) == 0 && !machine().side_effects_disabled())
	{
		uint16_t addr = offset & 0xfff;

		if (addr >= 0xff6 && addr <= 0xff9)
			m_banks[BIT(offset, 12)] = (addr - 0xff6);
	}

	if (offset < 0x1000)
		return m_rom[(offset & 0xfff) + (m_banks[0] * 0x1000) + 0];
	else if (offset < 0x2000)
		return m_rom[(offset & 0xfff) + (m_banks[1] * 0x1000) + 0x4000];
	else
		return m_rom[(offset & 0x1fff) + 0x8000];
}

void a800_rom_bbsb_device::write_80xx(offs_t offset, uint8_t data)
{
	uint16_t addr = offset & 0xfff;
	if (addr >= 0xff6 && addr <= 0xff9)
		m_banks[BIT(offset, 12)] = (addr - 0xff6);
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


/*-------------------------------------------------

 Williams 64K

 The rom is accessed in 8K chunks at 0xa000-0xbfff
 Bankswitch is controlled by writing to 7 diff
 offsets (their location varies with the cart type):
 offs 0 points to bank 0, offs 1 points to bank 1,
 and so on... the rom can be disabled by writing to
 the offsets 0x8-0xf of the same range as the bankswitch

 -------------------------------------------------*/

uint8_t a800_rom_williams_device::read_80xx(offs_t offset)
{
	return m_rom[(offset & 0x1fff) + (m_bank * 0x2000)];
}

void a800_rom_williams_device::write_d5xx(offs_t offset, uint8_t data)
{
	m_bank = (offset & 0x07);
}

/*-------------------------------------------------

 Express 64K / Diamond 64K carts

 The rom is accessed in 8K chunks at 0xa000-0xbfff
 Bankswitch is the same as above, but writes trigger
 banks in reverse order: offs 7 points to bank 0, offs 6
 points to bank 1, and so on... the rom can be disabled
 by writing to the offsets 0x8-0xf of the same range
 as the bankswitch

 -------------------------------------------------*/

uint8_t a800_rom_express_device::read_80xx(offs_t offset)
{
	return m_rom[(offset & 0x1fff) + (m_bank * 0x2000)];
}

void a800_rom_express_device::write_d5xx(offs_t offset, uint8_t data)
{
	m_bank = (offset ^ 0x07) & 0x0f;
}


/*-------------------------------------------------

 Turbosoft 64K / 128K


 -------------------------------------------------*/

uint8_t a800_rom_turbo_device::read_80xx(offs_t offset)
{
	return m_rom[(offset & 0x1fff) + (m_bank * 0x2000)];
}

void a800_rom_turbo_device::write_d5xx(offs_t offset, uint8_t data)
{
	m_bank = offset & m_bank_mask;
}


/*-------------------------------------------------

 Telelink II


 -------------------------------------------------*/

uint8_t a800_rom_telelink2_device::read_80xx(offs_t offset)
{
	if (offset >= 0x2000)
		return m_rom[offset & 0x1fff];
	if (offset >= 0x1000 && offset < 0x1100)
		return m_nvram[offset & 0xff];

	return 0xff;
}

void a800_rom_telelink2_device::write_80xx(offs_t offset, uint8_t data)
{
	m_nvram[offset & 0xff] = data | 0xf0;   // low 4bits only
}

uint8_t a800_rom_telelink2_device::read_d5xx(offs_t offset)
{
	// this should affect NVRAM enable / save
	return 0xff;
}

void a800_rom_telelink2_device::write_d5xx(offs_t offset, uint8_t data)
{
	// this should affect NVRAM enable / save
}



/*-------------------------------------------------

 SITSA Microcalc


 -------------------------------------------------*/

uint8_t a800_rom_microcalc_device::read_80xx(offs_t offset)
{
	return m_rom[(offset & 0x1fff) + (m_bank * 0x2000)];
}

void a800_rom_microcalc_device::write_d5xx(offs_t offset, uint8_t data)
{
	m_bank = data;
}

/*-------------------------------------------------

 Corina

 Comes in two configs:
 - 1MB Flash ROM (yakungfu)
 - 512KB Flash ROM + 512KB SRAM (bombjake)

 Both contains 8KB NVRAM
 -------------------------------------------------*/

uint8_t a800_rom_corina_device::read_view_1(offs_t offset)
{
	return m_rom[(offset & 0x3fff) + (m_rom_bank * 0x4000) + 0x80000];
}

void a800_rom_corina_device::write_view_1(offs_t offset, u8 data)
{
}

uint8_t a800_rom_corina_sram_device::read_view_1(offs_t offset)
{
	return m_ram[(offset & 0x3fff) + (m_rom_bank * 0x4000)];
}

void a800_rom_corina_sram_device::write_view_1(offs_t offset, u8 data)
{
	m_ram[(offset & 0x3fff) + (m_rom_bank * 0x4000)] = data;
}

uint8_t a800_rom_corina_device::read_80xx(offs_t offset)
{
	switch( m_view_select )
	{
		case 0:
			return m_rom[(offset & 0x3fff) + (m_rom_bank * 0x4000)];
		case 1:
			return read_view_1(offset);
		case 2:
			return m_nvram[offset & 0x1fff];
	}

	logerror("view select R=3 [%04x]\n", offset);
	return 0xff;
}

void a800_rom_corina_device::write_80xx(offs_t offset, uint8_t data)
{
	switch( m_view_select )
	{
		case 1:
			write_view_1(offset, data);
			return;
		case 2:
			m_nvram[offset & 0x1fff] = data;
			return;
	}
	// view 0: flash ROM commands?
	// TODO: identify
	logerror("view select W=%d [%04x, %02x] -> %02x\n", m_view_select, offset, m_rom_bank, data);
}

/*
 * 0--- ---- enable Corina window
 * 1--- ---- disable Corina and select main unit 8000-bfff window instead
 * -xx- ---- view select
 * -00- ---- first half of ROM
 * -01- ---- second half of ROM or RAM (^ depending on PCB config)
 * -10- ---- NVRAM
 * -11- ---- <reserved>
 * ---x xxxx ROM/RAM lower bank value,
 *           ignored if view select is not in ROM/RAM mode
 *           or Corina window is disabled
 */
void a800_rom_corina_device::write_d5xx(offs_t offset, uint8_t data)
{
	m_rom_bank = data & 0x1f;
	m_view_select = (data & 0x60) >> 5;
	// TODO: bit 7, currently handled in a400_state
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
