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


//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type A78_ROM = &device_creator<a78_rom_device>;
const device_type A78_ROM_SG = &device_creator<a78_rom_sg_device>;
const device_type A78_ROM_POKEY = &device_creator<a78_rom_pokey_device>;
const device_type A78_ROM_SG_POKEY = &device_creator<a78_rom_sg_pokey_device>;
const device_type A78_ROM_SG_RAM = &device_creator<a78_rom_sg_ram_device>;
const device_type A78_ROM_SG9 = &device_creator<a78_rom_sg9_device>;
const device_type A78_ROM_ABSOLUTE = &device_creator<a78_rom_abs_device>;
const device_type A78_ROM_ACTIVISION = &device_creator<a78_rom_act_device>;


const device_type A78_ROM_P450 = &device_creator<a78_rom_p450_device>;
const device_type A78_ROM_P450_POKEY = &device_creator<a78_rom_p450_pokey_device>;
const device_type A78_ROM_P450_SG_RAM = &device_creator<a78_rom_p450_sg_ram_device>;
const device_type A78_ROM_P450_SG9 = &device_creator<a78_rom_p450_sg9_device>;


a78_rom_device::a78_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						device_a78_cart_interface( mconfig, *this )
{
}

a78_rom_device::a78_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, A78_ROM, "Atari 7800 ROM Carts w/no Bankswitch", tag, owner, clock, "a78_rom", __FILE__),
						device_a78_cart_interface( mconfig, *this )
{
}

a78_rom_pokey_device::a78_rom_pokey_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: a78_rom_device(mconfig, type, name, tag, owner, clock, shortname, source),
						m_pokey(*this, "pokey")
{
}

a78_rom_pokey_device::a78_rom_pokey_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: a78_rom_device(mconfig, A78_ROM_POKEY, "Atari 7800 ROM Carts w/no Bankswitch + POKEY", tag, owner, clock, "a78_rom_pok", __FILE__),
						m_pokey(*this, "pokey")
{
}


a78_rom_sg_device::a78_rom_sg_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: a78_rom_device(mconfig, type, name, tag, owner, clock, shortname, source), m_bank(0)
				{
}

a78_rom_sg_device::a78_rom_sg_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: a78_rom_device(mconfig, A78_ROM_SG, "Atari 7800 ROM Carts w/SuperGame Bankswitch", tag, owner, clock, "a78_rom_sg", __FILE__), m_bank(0)
				{
}

a78_rom_sg_pokey_device::a78_rom_sg_pokey_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: a78_rom_sg_device(mconfig, A78_ROM_SG_POKEY, "Atari 7800 ROM Carts w/SuperGame Bankswitch + POKEY", tag, owner, clock, "a78_rom_sgp", __FILE__),
						m_pokey(*this, "pokey")
{
}


a78_rom_sg_ram_device::a78_rom_sg_ram_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: a78_rom_sg_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

a78_rom_sg_ram_device::a78_rom_sg_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: a78_rom_sg_device(mconfig, A78_ROM_SG_RAM, "Atari 7800 ROM Carts w/SuperGame Bankswitch + RAM", tag, owner, clock, "a78_rom_sgr", __FILE__)
{
}


a78_rom_sg9_device::a78_rom_sg9_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: a78_rom_sg_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

a78_rom_sg9_device::a78_rom_sg9_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: a78_rom_sg_device(mconfig, A78_ROM_SG9, "Atari 7800 ROM Carts w/SuperGame 9Banks", tag, owner, clock, "a78_rom_sg9", __FILE__)
{
}


a78_rom_abs_device::a78_rom_abs_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: a78_rom_device(mconfig, A78_ROM_ABSOLUTE, "Atari 7800 ROM Carts w/Absolute Bankswitch", tag, owner, clock, "a78_rom_abs", __FILE__), m_bank(0)
				{
}


a78_rom_act_device::a78_rom_act_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: a78_rom_device(mconfig, A78_ROM_ACTIVISION, "Atari 7800 ROM Carts w/Activision Bankswitch", tag, owner, clock, "a78_rom_act", __FILE__), m_bank(0)
				{
}



a78_rom_p450_device::a78_rom_p450_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: a78_rom_device(mconfig, A78_ROM_P450, "Atari 7800 ROM Carts w/POKEY @ 0x0450", tag, owner, clock, "a78_p450_t0", __FILE__),
						m_pokey450(*this, "pokey450")
{
}

a78_rom_p450_pokey_device::a78_rom_p450_pokey_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: a78_rom_pokey_device(mconfig, A78_ROM_P450_POKEY, "Atari 7800 ROM Carts w/no Bankswitch + POKEY + POKEY @ 0x0450", tag, owner, clock, "a78_p450_t1", __FILE__),
						m_pokey450(*this, "pokey450")
{
}

a78_rom_p450_sg_ram_device::a78_rom_p450_sg_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: a78_rom_sg_ram_device(mconfig, A78_ROM_P450_SG_RAM, "Atari 7800 ROM Carts w/SuperGame Bankswitch + RAM + POKEY @ 0x0450", tag, owner, clock, "a78_p450_t6", __FILE__),
						m_pokey450(*this, "pokey450")
{
}

a78_rom_p450_sg9_device::a78_rom_p450_sg9_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: a78_rom_sg9_device(mconfig, A78_ROM_P450_SG9, "Atari 7800 ROM Carts w/SuperGame 9Banks + POKEY @ 0x0450", tag, owner, clock, "a78_p450_ta", __FILE__),
						m_pokey450(*this, "pokey450")
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

// TO DO: do we need a PAL variant?!?
static MACHINE_CONFIG_FRAGMENT( a78_pokey )
	MCFG_SPEAKER_STANDARD_MONO("addon")

	MCFG_SOUND_ADD("pokey", POKEY, XTAL_14_31818MHz/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "addon", 1.00)
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( a78_pokey450 )
	MCFG_SPEAKER_STANDARD_MONO("pokey_450")

	MCFG_SOUND_ADD("pokey450", POKEY, XTAL_14_31818MHz/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "pokey_450", 1.00)
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( a78_2pokeys )
	MCFG_SPEAKER_STANDARD_MONO("addon")

	MCFG_SOUND_ADD("pokey", POKEY, XTAL_14_31818MHz/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "addon", 1.00)

	MCFG_SOUND_ADD("pokey450", POKEY, XTAL_14_31818MHz/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "addon", 1.00)
MACHINE_CONFIG_END


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
		return m_pokey->read(space, offset & 0x0f);

	if (offset + 0x4000 < m_base_rom)
		return 0xff;
	else
		return m_rom[offset + 0x4000 - m_base_rom];
}

WRITE8_MEMBER(a78_rom_pokey_device::write_40xx)
{
	if (offset < 0x4000)
		m_pokey->write(space, offset & 0x0f, data);
}

machine_config_constructor a78_rom_pokey_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( a78_pokey );
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
		return m_pokey->read(space, offset & 0x0f);
	else if (offset < 0x8000)
		return m_rom[(offset & 0x3fff) + (m_bank * 0x4000)];
	else
		return m_rom[(offset & 0x3fff) + (m_bank_mask * 0x4000)];   // last bank
}

WRITE8_MEMBER(a78_rom_sg_pokey_device::write_40xx)
{
	if (offset < 0x4000)
		m_pokey->write(space, offset & 0x0f, data);
	else if (offset < 0x8000)
		m_bank = data & m_bank_mask;
}

machine_config_constructor a78_rom_sg_pokey_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( a78_pokey );
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
	UINT8 data = 0xff;
	UINT16 addr = offset & 0x1fff;

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

machine_config_constructor a78_rom_p450_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( a78_pokey450 );
}

machine_config_constructor a78_rom_p450_pokey_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( a78_2pokeys );
}

machine_config_constructor a78_rom_p450_sg_ram_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( a78_pokey450 );
}

machine_config_constructor a78_rom_p450_sg9_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( a78_pokey450 );
}
