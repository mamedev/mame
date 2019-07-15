// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************

 A7800 ROM cart emulation

 For the moment we use separate devices for each combination of hardware
 - bankswitch or not
 - pokey or not
 - 9 banks or not
 etc...
 But we might merge many of these if they become too many

 TODO:
 - Are POKEY regs readable somewhere in SG 144K + POKEY homebrew? How do they detect
   the POKEY otherwise?!?

***********************************************************************************************************/


#include "emu.h"
#include "rom.h"
#include "speaker.h"


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(A78_ROM,             a78_rom_device,             "a78_rom",      "Atari 7800 ROM Carts w/no Bankswitch")
DEFINE_DEVICE_TYPE(A78_ROM_SG,          a78_rom_sg_device,          "a78_rom_sg",   "Atari 7800 ROM Carts w/SuperGame Bankswitch")
DEFINE_DEVICE_TYPE(A78_ROM_POKEY,       a78_rom_pokey_device,       "a78_rom_pok",  "Atari 7800 ROM Carts w/no Bankswitch + POKEY")
DEFINE_DEVICE_TYPE(A78_ROM_SG_POKEY,    a78_rom_sg_pokey_device,    "a78_rom_sgp",  "Atari 7800 ROM Carts w/SuperGame Bankswitch + POKEY")
DEFINE_DEVICE_TYPE(A78_ROM_SG_RAM,      a78_rom_sg_ram_device,      "a78_rom_sgr",  "Atari 7800 ROM Carts w/SuperGame Bankswitch + RAM")
DEFINE_DEVICE_TYPE(A78_ROM_MRAM,        a78_rom_mram_device,        "a78_rom_mram", "Atari 7800 ROM Carts + Mirror RAM")
DEFINE_DEVICE_TYPE(A78_ROM_SG9,         a78_rom_sg9_device,         "a78_rom_sg9",  "Atari 7800 ROM Carts w/SuperGame 9Banks")
DEFINE_DEVICE_TYPE(A78_ROM_ABSOLUTE,    a78_rom_abs_device,         "a78_rom_abs",  "Atari 7800 ROM Carts w/Absolute Bankswitch")
DEFINE_DEVICE_TYPE(A78_ROM_ACTIVISION,  a78_rom_act_device,         "a78_rom_act",  "Atari 7800 ROM Carts w/Activision Bankswitch")

DEFINE_DEVICE_TYPE(A78_ROM_P450,        a78_rom_p450_device,        "a78_p450_t0",  "Atari 7800 ROM Carts w/POKEY @ 0x0450")
DEFINE_DEVICE_TYPE(A78_ROM_P450_POKEY,  a78_rom_p450_pokey_device,  "a78_p450_t1",  "Atari 7800 ROM Carts w/no Bankswitch + POKEY + POKEY @ 0x0450")
DEFINE_DEVICE_TYPE(A78_ROM_P450_SG_RAM, a78_rom_p450_sg_ram_device, "a78_p450_t6",  "Atari 7800 ROM Carts w/SuperGame Bankswitch + RAM + POKEY @ 0x0450")
DEFINE_DEVICE_TYPE(A78_ROM_P450_SG9,    a78_rom_p450_sg9_device,    "a78_p450_ta",  "Atari 7800 ROM Carts w/SuperGame 9Banks + POKEY @ 0x0450")


a78_rom_device::a78_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_a78_cart_interface( mconfig, *this )
{
}

a78_rom_device::a78_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a78_rom_device(mconfig, A78_ROM, tag, owner, clock)
{
}

a78_rom_pokey_device::a78_rom_pokey_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: a78_rom_device(mconfig, type, tag, owner, clock)
	, m_pokey(*this, "pokey")
{
}

a78_rom_pokey_device::a78_rom_pokey_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a78_rom_pokey_device(mconfig, A78_ROM_POKEY, tag, owner, clock)
{
}


a78_rom_mram_device::a78_rom_mram_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: a78_rom_device(mconfig, type, tag, owner, clock)
{
}

a78_rom_mram_device::a78_rom_mram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a78_rom_mram_device(mconfig, A78_ROM_MRAM, tag, owner, clock)
{
}

a78_rom_sg_device::a78_rom_sg_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: a78_rom_device(mconfig, type, tag, owner, clock)
	, m_bank(0)
{
}

a78_rom_sg_device::a78_rom_sg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a78_rom_sg_device(mconfig, A78_ROM_SG, tag, owner, clock)
{
}

a78_rom_sg_pokey_device::a78_rom_sg_pokey_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a78_rom_sg_device(mconfig, A78_ROM_SG_POKEY, tag, owner, clock)
	, m_pokey(*this, "pokey")
{
}


a78_rom_sg_ram_device::a78_rom_sg_ram_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: a78_rom_sg_device(mconfig, type, tag, owner, clock)
{
}

a78_rom_sg_ram_device::a78_rom_sg_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a78_rom_sg_ram_device(mconfig, A78_ROM_SG_RAM, tag, owner, clock)
{
}


a78_rom_sg9_device::a78_rom_sg9_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: a78_rom_sg_device(mconfig, type, tag, owner, clock)
{
}

a78_rom_sg9_device::a78_rom_sg9_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a78_rom_sg9_device(mconfig, A78_ROM_SG9, tag, owner, clock)
{
}


a78_rom_abs_device::a78_rom_abs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a78_rom_device(mconfig, A78_ROM_ABSOLUTE, tag, owner, clock)
	, m_bank(0)
{
}


a78_rom_act_device::a78_rom_act_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a78_rom_device(mconfig, A78_ROM_ACTIVISION, tag, owner, clock)
	, m_bank(0)
{
}



a78_rom_p450_device::a78_rom_p450_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a78_rom_device(mconfig, A78_ROM_P450, tag, owner, clock)
	, m_pokey450(*this, "pokey450")
{
}

a78_rom_p450_pokey_device::a78_rom_p450_pokey_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a78_rom_pokey_device(mconfig, A78_ROM_P450_POKEY, tag, owner, clock)
	, m_pokey450(*this, "pokey450")
{
}

a78_rom_p450_sg_ram_device::a78_rom_p450_sg_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a78_rom_sg_ram_device(mconfig, A78_ROM_P450_SG_RAM, tag, owner, clock)
	, m_pokey450(*this, "pokey450")
{
}

a78_rom_p450_sg9_device::a78_rom_p450_sg9_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a78_rom_sg9_device(mconfig, A78_ROM_P450_SG9, tag, owner, clock)
	, m_pokey450(*this, "pokey450")
{
}



void a78_rom_device::device_start()
{
}

void a78_rom_device::device_reset()
{
}

void a78_rom_sg_device::device_start()
{
	save_item(NAME(m_bank));
}

void a78_rom_sg_device::device_reset()
{
	m_bank = 0;
}

void a78_rom_abs_device::device_start()
{
	save_item(NAME(m_bank));
}

void a78_rom_abs_device::device_reset()
{
	m_bank = 0;
}

void a78_rom_act_device::device_start()
{
	save_item(NAME(m_bank));
}

void a78_rom_act_device::device_reset()
{
	m_bank = 0;
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Carts with no bankswitch (8K to 48K)

 GAMES: Asteroids, Centipede, Dig Dug and many more

 -------------------------------------------------*/

READ8_MEMBER(a78_rom_device::read_40xx)
{
	if (offset + 0x4000 < m_base_rom)
		return 0xff;
	else
		return m_rom[offset + 0x4000 - m_base_rom];
}

/*-------------------------------------------------

 Carts with no bankswitch + POKEY chip
 The Pokey chips is accessed by writing at
 0x4000-0x7fff.

 GAMES: Ballblazer, Beef Drop (homebrew)

 -------------------------------------------------*/

READ8_MEMBER(a78_rom_pokey_device::read_40xx)
{
	if (offset < 0x4000)
		return m_pokey->read(offset & 0x0f);

	if (offset + 0x4000 < m_base_rom)
		return 0xff;
	else
		return m_rom[offset + 0x4000 - m_base_rom];
}

WRITE8_MEMBER(a78_rom_pokey_device::write_40xx)
{
	if (offset < 0x4000)
		m_pokey->write(offset & 0x0f, data);
}

// TO DO: do we need a PAL variant?!?
void a78_rom_pokey_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "addon").front_center();

	POKEY(config, m_pokey, XTAL(14'318'181)/8).add_route(ALL_OUTPUTS, "addon", 1.00);
}

/*-------------------------------------------------

 Carts with no bankswitch + mRAM chip
 The RAM chips are accessed by writing at
 0x4000-0x7fff.
 The A8 line of the RAM chip isn't used, to create
 mirrors of even pages at odd page locations.

 GAMES: Rescue on Fractalus (proto)

 -------------------------------------------------*/


READ8_MEMBER(a78_rom_mram_device::read_40xx)
{
	if (offset < 0x4000)
		return m_ram[offset & 0xfeff];
	if (offset + 0x4000 < m_base_rom)
		return 0xff;
	else
		return m_rom[offset + 0x4000 - m_base_rom];
}

WRITE8_MEMBER(a78_rom_mram_device::write_40xx)
{
	if (offset < 0x4000)
		m_ram[offset&0xfeff] = data;
}


/*-------------------------------------------------

 Carts with SuperGame bankswitch:
 8 x 16K banks mappable in 0x8000-0xbfff
 bank 7 is always mapped in 0xc000-0xffff
 range 0x4000-0x7fff is not clear: some games
 expect bank 6 to be mapped there, others
 have open bus (we assume the former until
 a game requires more precise behavior or
 some test is run)
 Note that the code is written so that also
 homebrew games with larger ROMs work!

 GAMES: Crack'd, Fatal Run, Ikari Warriors...

 -------------------------------------------------*/

READ8_MEMBER(a78_rom_sg_device::read_40xx)
{
	if (offset < 0x4000)
		return m_rom[(offset & 0x3fff) + ((m_bank_mask - 1) * 0x4000)]; // second to last bank (is this always ok?!?)
	else if (offset < 0x8000)
		return m_rom[(offset & 0x3fff) + (m_bank * 0x4000)];
	else
		return m_rom[(offset & 0x3fff) + (m_bank_mask * 0x4000)];   // last bank
}

WRITE8_MEMBER(a78_rom_sg_device::write_40xx)
{
	if (offset >= 0x4000 && offset < 0x8000)
		m_bank = data & m_bank_mask;
}


/*-------------------------------------------------

 Carts with SuperGame bankswitch + POKEY chip
 As above + Pokey chip access

 GAMES: Commando

 -------------------------------------------------*/

READ8_MEMBER(a78_rom_sg_pokey_device::read_40xx)
{
	if (offset < 0x4000)
		return m_pokey->read(offset & 0x0f);
	else if (offset < 0x8000)
		return m_rom[(offset & 0x3fff) + (m_bank * 0x4000)];
	else
		return m_rom[(offset & 0x3fff) + (m_bank_mask * 0x4000)];   // last bank
}

WRITE8_MEMBER(a78_rom_sg_pokey_device::write_40xx)
{
	if (offset < 0x4000)
		m_pokey->write(offset & 0x0f, data);
	else if (offset < 0x8000)
		m_bank = data & m_bank_mask;
}

void a78_rom_sg_pokey_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "addon").front_center();

	POKEY(config, m_pokey, XTAL(14'318'181)/8).add_route(ALL_OUTPUTS, "addon", 1.00);
}


/*-------------------------------------------------

 Carts with SuperGame bankswitch + 16K RAM
 FIXME: Some games contained only 8K of RAM, but
 for the moment we treat all as 16K of RAM even if
 from softlist we shall differentiate between them.

 GAMES: Impossible Mission, Jinks and some protos

 -------------------------------------------------*/

READ8_MEMBER(a78_rom_sg_ram_device::read_40xx)
{
	if (offset < 0x4000)
		return m_ram[offset];
	else if (offset < 0x8000)
		return m_rom[(offset & 0x3fff) + (m_bank * 0x4000)];
	else
		return m_rom[(offset & 0x3fff) + (m_bank_mask * 0x4000)];   // last bank
}

WRITE8_MEMBER(a78_rom_sg_ram_device::write_40xx)
{
	if (offset < 0x4000)
		m_ram[offset] = data;
	else if (offset < 0x8000)
		m_bank = data & m_bank_mask;
}


/*-------------------------------------------------

 Carts with SuperGame bankswitch 9banks:
 9 x 16K banks mappable in 0x8000-0xbfff
 bank 7 is always mapped in 0xc000-0xffff

 GAMES: Alien Brigade & Crossbow + some homebrew
 like Donkey Kong XM demo, Bentley Bear's Crystal
 Quest

 -------------------------------------------------*/

READ8_MEMBER(a78_rom_sg9_device::read_40xx)
{
	if (offset < 0x4000)
		return m_rom[(offset & 0x3fff)];
	else if (offset < 0x8000)
		return m_rom[(offset & 0x3fff) + (m_bank * 0x4000)];
	else
		return m_rom[(offset & 0x3fff) + ((m_bank_mask + 1) * 0x4000)]; // last bank
}

WRITE8_MEMBER(a78_rom_sg9_device::write_40xx)
{
	if (offset >= 0x4000 && offset < 0x8000)
		m_bank = (data & m_bank_mask) + 1;
}

/*-------------------------------------------------

 Carts with Absolute bankswitch:
 64K games. Lower 32K are 2 banks of 16K to be mapped
 in 0x4000-0x7fff, depending on the value written
 at 0x8000. Higher 32K are fixed in 0x8000-0xffff

 GAMES: F-18 Hornet

 -------------------------------------------------*/

READ8_MEMBER(a78_rom_abs_device::read_40xx)
{
	if (offset < 0x4000)
		return m_rom[(offset & 0x3fff) + (m_bank * 0x4000)];
	else
	{
		offset -= 0x4000;
		return m_rom[offset + 0x8000];
	}
}

WRITE8_MEMBER(a78_rom_abs_device::write_40xx)
{
	if (offset == 0x4000)
	{
		if (data & 1)
			m_bank = 0;
		else if (data & 2)
			m_bank = 1;
	}
}

/*-------------------------------------------------

 Carts with Activision bankswitch:
 128K games. 8 x 16K banks (0-7) to be mapped at
 0xa000-0xdfff. Bank is selected depending on the
 address written in 0xff80-0xff87.
 The rest of the memory is as follows:
 0x4000-0x5fff second 8kb of bank 6
 0x6000-0x7fff first 8kb of bank 6
 0x8000-0x9fff second 8kb of bank 7
 0xe000-0xffff first 8kb of bank 7

 GAMES: Double Dragon, Rampage.

 -------------------------------------------------*/

READ8_MEMBER(a78_rom_act_device::read_40xx)
{
	uint8_t data = 0xff;
	uint16_t addr = offset & 0x1fff;

	// offset goes from 0 to 0xc000
	switch (offset & 0xe000)
	{
		case 0x0000:
			data = m_rom[addr + 0x1a000];
			break;
		case 0x2000:
			data = m_rom[addr + 0x18000];
			break;
		case 0x4000:
			data = m_rom[addr + 0x1e000];
			break;
		case 0x6000:
			data = m_rom[addr + (m_bank * 0x4000)];
			break;
		case 0x8000:
			data = m_rom[addr + (m_bank * 0x4000) + 0x2000];
			break;
		case 0xa000:
			data = m_rom[addr + 0x1c000];
			break;
	}

	return data;
}

WRITE8_MEMBER(a78_rom_act_device::write_40xx)
{
	if (offset >= 0xbf80 && offset <= 0xbf8f)
		m_bank = offset & 7;
}


// Machine configs for PCB variants with a POKEY at $0450

void a78_rom_p450_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "pokey_450").front_center();

	POKEY(config, m_pokey450, XTAL(14'318'181)/8).add_route(ALL_OUTPUTS, "pokey_450", 1.00);
}

void a78_rom_p450_pokey_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "addon").front_center();

	POKEY(config, m_pokey, XTAL(14'318'181)/8).add_route(ALL_OUTPUTS, "addon", 1.00);

	POKEY(config, m_pokey450, XTAL(14'318'181)/8).add_route(ALL_OUTPUTS, "addon", 1.00);
}


void a78_rom_p450_sg_ram_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "pokey_450").front_center();

	POKEY(config, m_pokey450, XTAL(14'318'181)/8).add_route(ALL_OUTPUTS, "pokey_450", 1.00);
}

void a78_rom_p450_sg9_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "pokey_450").front_center();

	POKEY(config, m_pokey450, XTAL(14'318'181)/8).add_route(ALL_OUTPUTS, "pokey_450", 1.00);
}
