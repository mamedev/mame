/***********************************************************************************************************

 A7800 ROM cart emulation

 For the moment we use separate devices for each combination of hardware
 - bankswitch or not
 - pokey or not
 - 9 banks or not
 etc...
 But we might merge many of these if they become too many (e.g. could there be banked 32L RAM also
 in a 9banks cart or in a cart with no bankswitch?)

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
const device_type A78_ROM_BANKRAM = &device_creator<a78_rom_bankram_device>;
const device_type A78_ROM_SG_9BANKS = &device_creator<a78_rom_sg_9banks_device>;
const device_type A78_ROM_XM = &device_creator<a78_rom_xm_device>;
const device_type A78_ROM_ABSOLUTE = &device_creator<a78_rom_abs_device>;
const device_type A78_ROM_ACTIVISION = &device_creator<a78_rom_act_device>;


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

a78_rom_pokey_device::a78_rom_pokey_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: a78_rom_device(mconfig, A78_ROM_POKEY, "Atari 7800 ROM Carts w/no Bankswitch + POKEY", tag, owner, clock, "a78_rom_pok", __FILE__),
						m_pokey(*this, "pokey")
{
}


a78_rom_sg_device::a78_rom_sg_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: a78_rom_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

a78_rom_sg_device::a78_rom_sg_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: a78_rom_device(mconfig, A78_ROM_SG, "Atari 7800 ROM Carts w/SuperGame Bankswitch", tag, owner, clock, "a78_rom_sg", __FILE__)
{
}

a78_rom_sg_pokey_device::a78_rom_sg_pokey_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: a78_rom_sg_device(mconfig, A78_ROM_SG_POKEY, "Atari 7800 ROM Carts w/SuperGame Bankswitch + POKEY", tag, owner, clock, "a78_rom_sgp", __FILE__),
						m_pokey(*this, "pokey")
{
}


a78_rom_sg_ram_device::a78_rom_sg_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: a78_rom_sg_device(mconfig, A78_ROM_SG_RAM, "Atari 7800 ROM Carts w/SuperGame Bankswitch + RAM", tag, owner, clock, "a78_rom_sgr", __FILE__)
{
}


a78_rom_bankram_device::a78_rom_bankram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: a78_rom_sg_device(mconfig, A78_ROM_BANKRAM, "Atari 7800 ROM Carts w/SuperGame Bankswitch + Banked RAM", tag, owner, clock, "a78_rom_bankram", __FILE__)
{
}


a78_rom_sg_9banks_device::a78_rom_sg_9banks_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: a78_rom_sg_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

a78_rom_sg_9banks_device::a78_rom_sg_9banks_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: a78_rom_sg_device(mconfig, A78_ROM_SG_9BANKS, "Atari 7800 ROM Carts w/SuperGame 9Banks", tag, owner, clock, "a78_rom_sg9", __FILE__)
{
}


a78_rom_xm_device::a78_rom_xm_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: a78_rom_sg_9banks_device(mconfig, A78_ROM_XM, "Atari 7800 ROM Carts w/SuperGame 9Banks + POKEY (XM demo)", tag, owner, clock, "a78_rom_xm", __FILE__),
						m_pokey(*this, "pokey")
{
}


a78_rom_abs_device::a78_rom_abs_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: a78_rom_device(mconfig, A78_ROM_ABSOLUTE, "Atari 7800 ROM Carts w/Absolute Bankswitch", tag, owner, clock, "a78_rom_abs", __FILE__)
{
}


a78_rom_act_device::a78_rom_act_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: a78_rom_device(mconfig, A78_ROM_ACTIVISION, "Atari 7800 ROM Carts w/Activision Bankswitch", tag, owner, clock, "a78_rom_act", __FILE__)
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

void a78_rom_bankram_device::device_start()
{
	save_item(NAME(m_bank));
	save_item(NAME(m_ram_bank));
}

void a78_rom_bankram_device::device_reset()
{
	m_bank = 0;
	m_ram_bank = 0;
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


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Carts with no bankswitch (8K to 48K)

 GAMES:

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
 The Pokey chips is accessed at 0x0450-0x045f or
 by writing at 0x4000-0x7fff in some games.
 
 GAMES:
 
 -------------------------------------------------*/

WRITE8_MEMBER(a78_rom_pokey_device::write_40xx)
{
	if (offset < 0x4000)
		m_pokey->write(space, offset & 0x0f, data);
}

READ8_MEMBER(a78_rom_pokey_device::read_04xx)
{
	if (offset >= 0x50 && offset < 0x60)
		return m_pokey->read(space, offset & 0x0f);
	else
		return 0xff;
}

WRITE8_MEMBER(a78_rom_pokey_device::write_04xx)
{
	if (offset >= 0x50 && offset < 0x60)
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
 
 GAMES:
 
 -------------------------------------------------*/

READ8_MEMBER(a78_rom_sg_device::read_40xx)
{
	if (offset < 0x4000)
		return m_rom[(offset & 0x3fff) + ((m_bank_mask - 1) * 0x4000)];	// second to last bank (is this always ok?!?)
	else if (offset < 0x8000)
		return m_rom[(offset & 0x3fff) + (m_bank * 0x4000)];
	else
		return m_rom[(offset & 0x3fff) + (m_bank_mask * 0x4000)];	// last bank
}

WRITE8_MEMBER(a78_rom_sg_device::write_40xx)
{
	if (offset >= 0x4000 && offset < 0x8000)
		m_bank = data & m_bank_mask;
}


/*-------------------------------------------------

 Carts with SuperGame bankswitch + POKEY chip
 As above, the Pokey chips is accessed at
 
 GAMES:
 
 -------------------------------------------------*/

WRITE8_MEMBER(a78_rom_sg_pokey_device::write_40xx)
{
	if (offset < 0x4000)
		m_pokey->write(space, offset & 0x0f, data);
	else if (offset < 0x8000)
		m_bank = data & m_bank_mask;
}

READ8_MEMBER(a78_rom_sg_pokey_device::read_04xx)
{
	if (offset >= 0x50 && offset < 0x60)
		return m_pokey->read(space, offset & 0x0f);
	else
		return 0xff;
}

WRITE8_MEMBER(a78_rom_sg_pokey_device::write_04xx)
{
	if (offset >= 0x50 && offset < 0x60)
		m_pokey->write(space, offset & 0x0f, data);
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
 
 GAMES:
 
 -------------------------------------------------*/

READ8_MEMBER(a78_rom_sg_ram_device::read_40xx)
{
	if (offset < 0x4000)
		return m_ram[offset];
	else if (offset < 0x8000)
		return m_rom[(offset & 0x3fff) + (m_bank * 0x4000)];
	else
		return m_rom[(offset & 0x3fff) + (m_bank_mask * 0x4000)];	// last bank
}

WRITE8_MEMBER(a78_rom_sg_ram_device::write_40xx)
{
	if (offset < 0x4000)
		m_ram[offset] = data;
	else if (offset < 0x8000)
		m_bank = data & m_bank_mask;
}


/*-------------------------------------------------
 
 Carts with SuperGame bankswitch + 32K RAM:
 RAM bank is selected by writing with bit5 enabled
 in 0x4000-0x7fff range (bit0-bit4 give the ROM bank)
 
 GAMES:
 
 -------------------------------------------------*/

READ8_MEMBER(a78_rom_bankram_device::read_40xx)
{
	if (offset < 0x4000)
		return m_ram[offset + (m_ram_bank * 0x4000)];
	else if (offset < 0x8000)
		return m_rom[(offset & 0x3fff) + (m_bank * 0x4000)];
	else
		return m_rom[(offset & 0x3fff) + (m_bank_mask * 0x4000)];	// last bank
}

WRITE8_MEMBER(a78_rom_bankram_device::write_40xx)
{
	if (offset < 0x4000)
		m_ram[offset] = data;
	else if (offset < 0x8000)
	{
		m_bank = data & m_bank_mask;
		m_ram_bank = BIT(data, 5);
	}
}


/*-------------------------------------------------

 Carts with SuperGame bankswitch 9banks:
 9 x 16K banks mappable in 0x8000-0xbfff
 bank 7 is always mapped in 0xc000-0xffff
 
 GAMES:
 
 -------------------------------------------------*/

READ8_MEMBER(a78_rom_sg_9banks_device::read_40xx)
{
	if (offset < 0x4000)
		return m_rom[(offset & 0x3fff)];
	else if (offset < 0x8000)
		return m_rom[(offset & 0x3fff) + (m_bank * 0x4000)];
	else
		return m_rom[(offset & 0x3fff) + ((m_bank_mask + 1) * 0x4000)];	// last bank
}

WRITE8_MEMBER(a78_rom_sg_9banks_device::write_40xx)
{
	if (offset >= 0x4000 && offset < 0x8000)
		m_bank = (data & m_bank_mask) + 1;
}

/*-------------------------------------------------
 
 Carts using XM expansion module or XBoarD expansion
 The only game using this (Donkey Kong XM demo) is
 144K + POKEY, so that it's like the above with the
 addition of the POKEY.
 
 GAMES: Donkey Kong XM demo
 
 -------------------------------------------------*/

WRITE8_MEMBER(a78_rom_xm_device::write_40xx)
{
	if (offset < 0x4000)
		m_pokey->write(space, offset & 0x0f, data);
	else if (offset < 0x8000)
		m_bank = (data & m_bank_mask) + 1;
}

READ8_MEMBER(a78_rom_xm_device::read_04xx)
{
	if (offset >= 0x50 && offset < 0x60)
		return m_pokey->read(space, offset & 0x0f);
	else
		return 0xff;
}

WRITE8_MEMBER(a78_rom_xm_device::write_04xx)
{
	if (offset >= 0x50 && offset < 0x60)
		m_pokey->write(space, offset & 0x0f, data);
}

machine_config_constructor a78_rom_xm_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( a78_pokey );
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
	if (offset >= 0xbf80 && offset <= 0xbf87)
		m_bank = offset & 7;
}
