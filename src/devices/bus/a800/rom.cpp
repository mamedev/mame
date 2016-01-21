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

const device_type A800_ROM = &device_creator<a800_rom_device>;
const device_type A800_ROM_BBSB = &device_creator<a800_rom_bbsb_device>;
const device_type A800_ROM_WILLIAMS = &device_creator<a800_rom_williams_device>;
const device_type A800_ROM_EXPRESS = &device_creator<a800_rom_express_device>;
const device_type A800_ROM_TURBO = &device_creator<a800_rom_turbo_device>;
const device_type A800_ROM_TELELINK2 = &device_creator<a800_rom_telelink2_device>;
const device_type A800_ROM_MICROCALC = &device_creator<a800_rom_microcalc_device>;
const device_type XEGS_ROM = &device_creator<xegs_rom_device>;
const device_type A5200_ROM_2CHIPS = &device_creator<a5200_rom_2chips_device>;
const device_type A5200_ROM_BBSB = &device_creator<a5200_rom_bbsb_device>;


a800_rom_device::a800_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						device_a800_cart_interface( mconfig, *this )
{
}

a800_rom_device::a800_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, A800_ROM, "Atari 800 ROM Carts", tag, owner, clock, "a800_rom", __FILE__),
						device_a800_cart_interface( mconfig, *this )
{
}


a800_rom_bbsb_device::a800_rom_bbsb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: a800_rom_device(mconfig, A800_ROM_BBSB, "Atari 800 ROM Cart BBSB", tag, owner, clock, "a800_bbsb", __FILE__)
{
}



xegs_rom_device::xegs_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: a800_rom_device(mconfig, XEGS_ROM, "Atari XEGS 64K ROM Carts", tag, owner, clock, "a800_xegs", __FILE__), m_bank(0)
				{
}


a800_rom_williams_device::a800_rom_williams_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: a800_rom_device(mconfig, A800_ROM_WILLIAMS, "Atari 800 64K ROM Carts Williams", tag, owner, clock, "a800_williams", __FILE__), m_bank(0)
				{
}


a800_rom_express_device::a800_rom_express_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: a800_rom_device(mconfig, A800_ROM_EXPRESS, "Atari 800 64K ROM Carts Express/Diamond", tag, owner, clock, "a800_express", __FILE__), m_bank(0)
				{
}


a800_rom_turbo_device::a800_rom_turbo_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: a800_rom_device(mconfig, A800_ROM_TURBO, "Atari 800 64K ROM Carts Turbosoft", tag, owner, clock, "a800_turbo", __FILE__), m_bank(0)
				{
}


a800_rom_telelink2_device::a800_rom_telelink2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: a800_rom_device(mconfig, A800_ROM_TELELINK2, "Atari 800 64K ROM Cart Telelink II", tag, owner, clock, "a800_tlink2", __FILE__)
{
}


a800_rom_microcalc_device::a800_rom_microcalc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: a800_rom_device(mconfig, A800_ROM_MICROCALC, "Atari 800 64K ROM Cart SITSA MicroCalc", tag, owner, clock, "a800_sitsa", __FILE__), m_bank(0)
				{
}


a5200_rom_2chips_device::a5200_rom_2chips_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: a800_rom_device(mconfig, A5200_ROM_2CHIPS, "Atari 5200 ROM Cart 16K in 2 Chips", tag, owner, clock, "a5200_16k2c", __FILE__)
{
}


a5200_rom_bbsb_device::a5200_rom_bbsb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: a800_rom_device(mconfig, A5200_ROM_BBSB, "Atari 5200 ROM Cart BBSB", tag, owner, clock, "a5200_bbsb", __FILE__)
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

READ8_MEMBER(a800_rom_device::read_80xx)
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

READ8_MEMBER(a800_rom_bbsb_device::read_80xx)
{
	if (offset < 0x1000)
		return m_rom[(offset & 0xfff) + (m_banks[0] * 0x1000) + 0];
	else if (offset < 0x2000)
		return m_rom[(offset & 0xfff) + (m_banks[1] * 0x1000) + 0x4000];
	else
		return m_rom[(offset & 0x1fff) + 0x8000];
}

WRITE8_MEMBER(a800_rom_bbsb_device::write_80xx)
{
	UINT16 addr = offset & 0xfff;
	if (addr >= 0xff6 && addr <= 0xff9)
		m_banks[BIT(offset, 12)] = (addr - 0xff6);
}

/*-------------------------------------------------

 XEGS carts (32K, 64K or 128K)

 Bankswitch is controlled by data written in
 0xd500-0xd5ff

 -------------------------------------------------*/

READ8_MEMBER(xegs_rom_device::read_80xx)
{
	if (offset < 0x2000)
		return m_rom[(offset & 0x1fff) + (m_bank * 0x2000)];
	else
		return m_rom[(offset & 0x1fff) + (m_bank_mask * 0x2000)];   // always last 8K bank

}

WRITE8_MEMBER(xegs_rom_device::write_d5xx)
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

READ8_MEMBER(a800_rom_williams_device::read_80xx)
{
	return m_rom[(offset & 0x1fff) + (m_bank * 0x2000)];
}

WRITE8_MEMBER(a800_rom_williams_device::write_d5xx)
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

READ8_MEMBER(a800_rom_express_device::read_80xx)
{
	return m_rom[(offset & 0x1fff) + (m_bank * 0x2000)];
}

WRITE8_MEMBER(a800_rom_express_device::write_d5xx)
{
	m_bank = (offset ^ 0x07) & 0x0f;
}


/*-------------------------------------------------

 Turbosoft 64K / 128K


 -------------------------------------------------*/

READ8_MEMBER(a800_rom_turbo_device::read_80xx)
{
	return m_rom[(offset & 0x1fff) + (m_bank * 0x2000)];
}

WRITE8_MEMBER(a800_rom_turbo_device::write_d5xx)
{
	m_bank = offset & m_bank_mask;
}


/*-------------------------------------------------

 Telelink II


 -------------------------------------------------*/

READ8_MEMBER(a800_rom_telelink2_device::read_80xx)
{
	if (offset >= 0x2000)
		return m_rom[offset & 0x1fff];
	if (offset >= 0x1000 && offset < 0x1100)
		return m_nvram[offset & 0xff];

	return 0xff;
}

WRITE8_MEMBER(a800_rom_telelink2_device::write_80xx)
{
	m_nvram[offset & 0xff] = data | 0xf0;   // low 4bits only
}

READ8_MEMBER(a800_rom_telelink2_device::read_d5xx)
{
	// this should affect NVRAM enable / save
	return 0xff;
}

WRITE8_MEMBER(a800_rom_telelink2_device::write_d5xx)
{
	// this should affect NVRAM enable / save
}



/*-------------------------------------------------

 SITSA Microcalc


 -------------------------------------------------*/

READ8_MEMBER(a800_rom_microcalc_device::read_80xx)
{
	return m_rom[(offset & 0x1fff) + (m_bank * 0x2000)];
}

WRITE8_MEMBER(a800_rom_microcalc_device::write_d5xx)
{
	m_bank = data;
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

READ8_MEMBER(a5200_rom_2chips_device::read_80xx)
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

READ8_MEMBER(a5200_rom_bbsb_device::read_80xx)
{
	if (offset < 0x1000)
		return m_rom[(offset & 0xfff) + (m_banks[0] * 0x1000) + 0];
	else if (offset < 0x2000)
		return m_rom[(offset & 0xfff) + (m_banks[1] * 0x1000) + 0x4000];
	else if (offset >= 0x4000)
		return m_rom[(offset & 0x1fff) + 0x8000];
	else
		return 0;
}

WRITE8_MEMBER(a5200_rom_bbsb_device::write_80xx)
{
	UINT16 addr = offset & 0xfff;
	if (addr >= 0xff6 && addr <= 0xff9)
		m_banks[BIT(offset, 12)] = (addr - 0xff6);
}
