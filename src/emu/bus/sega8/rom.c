// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************

 Sega 8-bit cart emulation (for Master System, GameGear and SG-1000)

 TODO: implement proper ROM & RAM mirroring when the cart size is not a power of 2K (e.g. 24K or 48K)

 ***********************************************************************************************************/


#include "emu.h"
#include "rom.h"


//-------------------------------------------------
//  constructors
//-------------------------------------------------

// Base cart type shared across SG-1000, SG-1000 Mark II, SG-1000 Mark III, SMS, GG
// even if in sg1000 rom banks are never changed and ram is never enabled
const device_type SEGA8_ROM_STD = &device_creator<sega8_rom_device>;

// Specific SG-1000 MkI - MkII cart types
const device_type SEGA8_ROM_OTHELLO = &device_creator<sega8_othello_device>;
const device_type SEGA8_ROM_CASTLE = &device_creator<sega8_castle_device>;
const device_type SEGA8_ROM_BASIC_L3 = &device_creator<sega8_basic_l3_device>;
const device_type SEGA8_ROM_MUSIC_EDITOR = &device_creator<sega8_music_editor_device>;
const device_type SEGA8_ROM_TEREBI = &device_creator<sega8_terebi_device>;
const device_type SEGA8_ROM_DAHJEE_TYPEA = &device_creator<sega8_dahjee_typea_device>;
const device_type SEGA8_ROM_DAHJEE_TYPEB = &device_creator<sega8_dahjee_typeb_device>;

// Specific SG-1000 MkIII - SMS - GG cart types
const device_type SEGA8_ROM_EEPROM = &device_creator<sega8_eeprom_device>;
const device_type SEGA8_ROM_CODEMASTERS = &device_creator<sega8_codemasters_device>;
const device_type SEGA8_ROM_4PAK = &device_creator<sega8_4pak_device>;
const device_type SEGA8_ROM_ZEMINA = &device_creator<sega8_zemina_device>;
const device_type SEGA8_ROM_NEMESIS = &device_creator<sega8_nemesis_device>;
const device_type SEGA8_ROM_JANGGUN = &device_creator<sega8_janggun_device>;
const device_type SEGA8_ROM_HICOM = &device_creator<sega8_hicom_device>;
const device_type SEGA8_ROM_KOREAN = &device_creator<sega8_korean_device>;
const device_type SEGA8_ROM_KOREAN_NB = &device_creator<sega8_korean_nb_device>;



sega8_rom_device::sega8_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						device_sega8_cart_interface( mconfig, *this )
{
}

sega8_rom_device::sega8_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, SEGA8_ROM_STD, "Mark III, SMS & GG Carts", tag, owner, clock, "sega8_rom", __FILE__),
						device_sega8_cart_interface( mconfig, *this )
{
}




sega8_othello_device::sega8_othello_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: sega8_rom_device(mconfig, SEGA8_ROM_OTHELLO, "SG-1000 Othello Cart", tag, owner, clock, "sega8_othello", __FILE__)
{
}


sega8_castle_device::sega8_castle_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: sega8_rom_device(mconfig, SEGA8_ROM_CASTLE, "SG-1000 The Castle Cart", tag, owner, clock, "sega8_castle", __FILE__)
{
}


sega8_basic_l3_device::sega8_basic_l3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: sega8_rom_device(mconfig, SEGA8_ROM_BASIC_L3, "SC-3000 BASIC Level III Cart", tag, owner, clock, "sega8_basicl3", __FILE__)
{
}


sega8_music_editor_device::sega8_music_editor_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: sega8_rom_device(mconfig, SEGA8_ROM_MUSIC_EDITOR, "SC-3000 Music Editor Cart", tag, owner, clock, "sega8_music", __FILE__)
{
}


sega8_terebi_device::sega8_terebi_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: sega8_rom_device(mconfig, SEGA8_ROM_TEREBI, "SG-1000 Terebi Oekaki Cart", tag, owner, clock, "sega8_terebi", __FILE__),
						m_tvdraw_x(*this, "TVDRAW_X"),
						m_tvdraw_y(*this, "TVDRAW_Y"),
						m_tvdraw_pen(*this, "TVDRAW_PEN")
{
}


sega8_dahjee_typea_device::sega8_dahjee_typea_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: sega8_rom_device(mconfig, SEGA8_ROM_DAHJEE_TYPEA, "SG-1000 Dahjee RAM expansion + Cart (Type A)", tag, owner, clock, "sega8_dahjeea", __FILE__)
{
}


sega8_dahjee_typeb_device::sega8_dahjee_typeb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: sega8_rom_device(mconfig, SEGA8_ROM_DAHJEE_TYPEB, "SG-1000 Dahjee RAM expansion + Cart (Type B)", tag, owner, clock, "sega8_dahjeeb", __FILE__)
{
}




sega8_eeprom_device::sega8_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, SEGA8_ROM_EEPROM, "GG Carts + EEPROM", tag, owner, clock, "sega8_eeprom", __FILE__),
						device_sega8_cart_interface( mconfig, *this ),
						m_eeprom(*this, "eeprom")
{
}


sega8_codemasters_device::sega8_codemasters_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, SEGA8_ROM_CODEMASTERS, "Mark III, SMS & GG Codemasters Carts", tag, owner, clock, "sega8_codemasters", __FILE__),
						device_sega8_cart_interface( mconfig, *this )
{
}


sega8_4pak_device::sega8_4pak_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: sega8_rom_device(mconfig, SEGA8_ROM_4PAK, "SMS 4-Pak Cart", tag, owner, clock, "sega8_4pak", __FILE__)
{
}


sega8_zemina_device::sega8_zemina_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						device_sega8_cart_interface( mconfig, *this )
{
}

sega8_zemina_device::sega8_zemina_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, SEGA8_ROM_ZEMINA, "SMS Zemina Carts", tag, owner, clock, "sega8_zemina", __FILE__),
						device_sega8_cart_interface( mconfig, *this )
{
}


sega8_nemesis_device::sega8_nemesis_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: sega8_zemina_device(mconfig, SEGA8_ROM_NEMESIS, "SMS Nemesis Cart", tag, owner, clock, "sega8_nemesis", __FILE__)
{
}


sega8_janggun_device::sega8_janggun_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, SEGA8_ROM_JANGGUN, "SMS Janggun Cart", tag, owner, clock, "sega8_janggun", __FILE__),
						device_sega8_cart_interface( mconfig, *this )
{
}


sega8_hicom_device::sega8_hicom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: sega8_rom_device(mconfig, SEGA8_ROM_HICOM, "SMS Hi-Com Carts", tag, owner, clock, "sega8_hicom", __FILE__)
{
}


sega8_korean_device::sega8_korean_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: sega8_rom_device(mconfig, SEGA8_ROM_KOREAN, "SMS Korean Carts", tag, owner, clock, "sega8_korean", __FILE__)
{
}


sega8_korean_nb_device::sega8_korean_nb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: sega8_rom_device(mconfig, SEGA8_ROM_KOREAN_NB, "SMS Korean No-Bank Mapper Carts", tag, owner, clock, "sega8_korean_nb", __FILE__)
{
}


void sega8_rom_device::device_start()
{
	save_item(NAME(m_rom_bank_base));
	save_item(NAME(m_ram_base));
	save_item(NAME(m_ram_enabled));
}

void sega8_rom_device::device_reset()
{
	m_ram_base = 0;
	m_ram_enabled = 0;
}



void sega8_terebi_device::device_start()
{
	save_item(NAME(m_rom_bank_base));
	save_item(NAME(m_tvdraw_data));
}

void sega8_terebi_device::device_reset()
{
	m_tvdraw_data = 0;
}




void sega8_eeprom_device::device_start()
{
	save_item(NAME(m_rom_bank_base));
	save_item(NAME(m_93c46_enabled));
	save_item(NAME(m_93c46_lines));
}

void sega8_eeprom_device::device_reset()
{
	m_93c46_lines = 0;
	m_93c46_enabled = 0;
}


void sega8_codemasters_device::device_start()
{
	save_item(NAME(m_rom_bank_base));
	save_item(NAME(m_ram_base));
	save_item(NAME(m_ram_enabled));
}

void sega8_codemasters_device::device_reset()
{
	m_ram_base = 0;
	m_ram_enabled = 0;
}

void sega8_4pak_device::device_start()
{
	save_item(NAME(m_rom_bank_base));
	save_item(NAME(m_reg));
}

void sega8_4pak_device::device_reset()
{
	memset(m_reg, 0, sizeof(m_reg));
}


void sega8_zemina_device::device_start()
{
	save_item(NAME(m_rom_bank_base));
	save_item(NAME(m_ram_base));
	save_item(NAME(m_ram_enabled));
}

void sega8_zemina_device::device_reset()
{
	m_ram_base = 0;
	m_ram_enabled = 0;
}


// initial bank setup needs to know how many 16K banks are available, so it needs to be called during cart loading...

void sega8_rom_device::late_bank_setup()
{
	m_rom_bank_base[0] = 0;
	m_rom_bank_base[1] = 1 % m_rom_page_count;
	m_rom_bank_base[2] = 2 % m_rom_page_count;
}

void sega8_eeprom_device::late_bank_setup()
{
	m_rom_bank_base[0] = 0;
	m_rom_bank_base[1] = 1 % m_rom_page_count;
	m_rom_bank_base[2] = 2 % m_rom_page_count;
}

void sega8_codemasters_device::late_bank_setup()
{
	m_rom_bank_base[0] = 0;
	m_rom_bank_base[1] = 1 % m_rom_page_count;
	m_rom_bank_base[2] = 0;
}

void sega8_zemina_device::late_bank_setup()
{
	m_rom_bank_base[0] = 0;
	m_rom_bank_base[1] = 1 % (m_rom_page_count * 2);
	m_rom_bank_base[2] = 2 % (m_rom_page_count * 2);
	m_rom_bank_base[3] = 3 % (m_rom_page_count * 2);
	m_rom_bank_base[4] = 4 % (m_rom_page_count * 2);
	m_rom_bank_base[5] = 5 % (m_rom_page_count * 2);
}

void sega8_nemesis_device::late_bank_setup()
{
	// Nemesis starts with last 8kb bank in page 0 (m_rom_page_count is for 16kb pages)
	m_rom_bank_base[0] = m_rom_page_count * 2 - 1;
	m_rom_bank_base[1] = 1 % (m_rom_page_count * 2);
	m_rom_bank_base[2] = 2 % (m_rom_page_count * 2);
	m_rom_bank_base[3] = 3 % (m_rom_page_count * 2);
	m_rom_bank_base[4] = 4 % (m_rom_page_count * 2);
	m_rom_bank_base[5] = 5 % (m_rom_page_count * 2);
}

void sega8_janggun_device::late_bank_setup()
{
	m_rom_bank_base[0] = 0;
	m_rom_bank_base[1] = 1;
	m_rom_bank_base[2] = 2;
	m_rom_bank_base[3] = 3;
	m_rom_bank_base[4] = 4;
	m_rom_bank_base[5] = 5;
}

void sega8_hicom_device::late_bank_setup()
{
	m_rom_bank_base = 0;
}

void sega8_korean_device::late_bank_setup()
{
	m_rom_bank_base[0] = 0;
	m_rom_bank_base[1] = 1;
	m_rom_bank_base[2] = 2;
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Base Sega 8bit carts, possibly with bankswitch
 (only used by Mark III, SMS and GG games)

 -------------------------------------------------*/

READ8_MEMBER(sega8_rom_device::read_cart)
{
	int bank = offset / 0x4000;

	if (bank == 2 && !m_ram.empty() && m_ram_enabled)
		return m_ram[(m_ram_base * 0x4000 + (offset & 0x3fff)) % m_ram.size()];

	if (offset < 0x400) // first 1k is hardcoded
		return m_rom[offset];

	return m_rom[m_rom_bank_base[bank] * 0x4000 + (offset & 0x3fff)];
}

WRITE8_MEMBER(sega8_rom_device::write_cart)
{
	int bank = offset / 0x4000;

	if (bank == 2 && !m_ram.empty() && m_ram_enabled)
		m_ram[(m_ram_base * 0x4000 + (offset & 0x3fff)) % m_ram.size()] = data;
}

WRITE8_MEMBER(sega8_rom_device::write_mapper)
{
	switch (offset)
	{
		case 0:
			if (data & 0x08)
			{
				// hack: if we were loading from fullpath (hence allowing m_late_battery_enable to be true)
				// and the RAM is ever enabled by the game, then we save it at exit...
				if (m_late_battery_enable)
					has_battery = 1;
				m_ram_enabled = 1;
				m_ram_base = (data & 0x04) >> 2;
			}
			else
				m_ram_enabled = 0;
			break;

		case 1: // Select 16k ROM bank for 0000-3fff
		case 2: // Select 16k ROM bank for 4000-7fff
		case 3: // Select 16k ROM bank for 8000-bfff
			m_rom_bank_base[offset - 1] = data  % m_rom_page_count;
			break;
	}
}


/*-------------------------------------------------

 Othello is a SG-1000 game featuring 2K of
 oncart RAM, mapped at 0x8000-0x9fff.
 Is RAM mirrored? For now we assume so...

 -------------------------------------------------*/

READ8_MEMBER(sega8_othello_device::read_cart)
{
	// 8K of RAM sits in 0x8000-0x9fff
	if (offset >= 0x8000 && offset < 0xa000)
		return m_ram[offset & 0x7ff];

	return m_rom[offset % m_rom_size];
}

WRITE8_MEMBER(sega8_othello_device::write_cart)
{
	// 2K of RAM sits in 0x8000-0x9fff
	if (offset >= 0x8000 && offset < 0xa000)
		m_ram[offset & 0x7ff] = data;
}


/*-------------------------------------------------

 The Castle is a SG-1000 game featuring 8K of
 oncart RAM, mapped at 0x8000-0x9fff

 -------------------------------------------------*/

READ8_MEMBER(sega8_castle_device::read_cart)
{
	// 8K of RAM sits in 0x8000-0x9fff
	if (offset >= 0x8000 && offset < 0xa000)
		return m_ram[offset & 0x1fff];

	return m_rom[offset % m_rom_size];
}

WRITE8_MEMBER(sega8_castle_device::write_cart)
{
	// 8K of RAM sits in 0x8000-0x9fff
	if (offset >= 0x8000 && offset < 0xa000)
		m_ram[offset & 0x1fff] = data;
}


/*-------------------------------------------------

 BASIC Level III cart featured 32K of
 oncart RAM, mapped at 0x8000-0xffff?

 -------------------------------------------------*/

READ8_MEMBER(sega8_basic_l3_device::read_cart)
{
	// 8K of RAM sits in 0x8000-0x9fff
	if (offset >= 0x8000)
		return m_ram[offset & 0x3fff];

	return m_rom[offset % m_rom_size];
}

WRITE8_MEMBER(sega8_basic_l3_device::write_cart)
{
	// 8K of RAM sits in 0x8000-0x9fff
	if (offset >= 0x8000)
		m_ram[offset & 0x3fff] = data;
}

READ8_MEMBER(sega8_basic_l3_device::read_ram)
{
	return m_ram[0x4000 + (offset & 0x3fff)];
}

WRITE8_MEMBER(sega8_basic_l3_device::write_ram)
{
	m_ram[0x4000 + (offset & 0x3fff)] = data;
}


/*-------------------------------------------------

 Music Editor cart featured 10K of oncart RAM, mapped
 in 0x8000-0x9fff and 0xc000-0xffff

 -------------------------------------------------*/

READ8_MEMBER(sega8_music_editor_device::read_cart)
{
	// 8K of RAM sits in 0x8000-0x9fff
	if (offset >= 0x8000 && offset < 0xa000)
		return m_ram[offset & 0x1fff];

	return m_rom[offset % m_rom_size];
}

WRITE8_MEMBER(sega8_music_editor_device::write_cart)
{
	// 8K of RAM sits in 0x8000-0x9fff
	if (offset >= 0x8000 && offset < 0xa000)
		m_ram[offset & 0x1fff] = data;
}

READ8_MEMBER(sega8_music_editor_device::read_ram)
{
	// 2K more of RAM sits in 0xc000-0xc3ff (and mirrored up to 0xffff)
	// or should it simply go to the 2K of SC3000 RAM???
	return m_ram[0x2000 + (offset & 0x7ff)];
}

WRITE8_MEMBER(sega8_music_editor_device::write_ram)
{
	// 2K more of RAM sits in 0xc000-0xc3ff (and mirrored up to 0xffff)
	// or should it simply go to the 2K of SC3000 RAM???
	m_ram[0x2000 + (offset & 0x7ff)] = data;
}


/*-------------------------------------------------

 SG-1000 Terebi Oekaki using a Tablet input device

 -------------------------------------------------*/

/*

 Terebi Oekaki (TV Draw)

 Address Access  Bits
 7       6   5   4   3   2   1   0
 $6000   W       -       -   -   -   -   -   -   AXIS
 $8000   R       BUSY    -   -   -   -   -   -   PRESS
 $A000   R/W     DATA

 AXIS: write 0 to select X axis, 1 to select Y axis.
 BUSY: reads 1 when graphic board is busy sampling position, else 0.
 PRESS: reads 0 when pen is touching graphic board, else 1.
 DATA: when pen is touching graphic board, return 8-bit sample position for currently selected axis (X is in the 0-255 range, Y in the 0-191 range). Else, return 0.

 */


READ8_MEMBER(sega8_terebi_device::read_cart)
{
	int bank = offset / 0x4000;

	if (offset == 0x8000)
		return m_tvdraw_pen->read();
	if (offset == 0xa000)
		return m_tvdraw_data;

	return m_rom[m_rom_bank_base[bank] * 0x4000 + (offset & 0x3fff)];
}

WRITE8_MEMBER(sega8_terebi_device::write_cart)
{
	switch (offset)
	{
		case 0x6000:
			if (data & 0x01)
			{
				m_tvdraw_data = m_tvdraw_x->read();

				if (m_tvdraw_data < 4) m_tvdraw_data = 4;
				if (m_tvdraw_data > 251) m_tvdraw_data = 251;
			}
			else
				m_tvdraw_data = m_tvdraw_y->read() + 0x20;
			break;
		case 0xa000:
			// effect unknown
			break;
	}
}

static INPUT_PORTS_START( tvdraw )
	PORT_START("TVDRAW_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_NAME("Tablet - X Axis") PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("TVDRAW_Y")
	PORT_BIT( 0xff, 0x60, IPT_LIGHTGUN_Y ) PORT_NAME("Tablet - Y Axis") PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(0, 191) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("TVDRAW_PEN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Tablet - Pen")
INPUT_PORTS_END

ioport_constructor sega8_terebi_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( tvdraw );
}


/*-------------------------------------------------

 Dahjee carts were sold with a RAM expansion pass-through
 cart (which we don't emulate separately for the
 moment) which allowed to play on old SG1000 machines
 some MSX conversions requiring more RAM than available

 Two kind of expansion existed (for different games),
 one with 9K of RAM (Type A) and one with 8K of
 RAM (Type B).

 -------------------------------------------------*/

// TYPE A
READ8_MEMBER(sega8_dahjee_typea_device::read_cart)
{
	// 8K of RAM sits in 0x2000-0x3fff
	if (offset >= 0x2000 && offset < 0x4000)
		return m_ram[offset & 0x1fff];

	return m_rom[offset % m_rom_size];
}

WRITE8_MEMBER(sega8_dahjee_typea_device::write_cart)
{
	// 8K of RAM sits in 0x2000-0x3fff
	if (offset >= 0x2000 && offset < 0x4000)
		m_ram[offset & 0x1fff] = data;
}

READ8_MEMBER(sega8_dahjee_typea_device::read_ram)
{
	// 1K more of RAM sits in 0xc000-0xc3ff (and mirrored up to 0xffff
	// or should it simply go to the 1K of SG1000 RAM???
	return m_ram[0x2000 + (offset & 0x3ff)];
}

WRITE8_MEMBER(sega8_dahjee_typea_device::write_ram)
{
	// 1K more of RAM sits in 0xc000-0xc3ff (and mirrored up to 0xffff
	// or should it simply go to the 1K of SG1000 RAM???
	m_ram[0x2000 + (offset & 0x3ff)] = data;
}


// TYPE B
READ8_MEMBER(sega8_dahjee_typeb_device::read_cart)
{
	return m_rom[offset % m_rom_size];
}

READ8_MEMBER(sega8_dahjee_typeb_device::read_ram)
{
	// 8K more of RAM sits in 0xc000-0xffff
	return m_ram[offset & 0x1fff];
}

WRITE8_MEMBER(sega8_dahjee_typeb_device::write_ram)
{
	// 8K more of RAM sits in 0xc000-0xffff
	m_ram[offset & 0x1fff] = data;
}




/*-------------------------------------------------

 Sega carts + EEPROM, used for some GameGear baseball
 games

 -------------------------------------------------*/


READ8_MEMBER(sega8_eeprom_device::read_cart)
{
	int bank = offset / 0x4000;

	if (offset == 0x8000 && m_93c46_enabled)
	{
		UINT8 value = (m_93c46_lines & 0xfc) | 0x02;
		value |= m_eeprom->do_read() ? 1 : 0;
		return value;
	}

	if (offset < 0x400) // first 1k is hardcoded
		return m_rom[offset];

	return m_rom[m_rom_bank_base[bank] * 0x4000 + (offset & 0x3fff)];
}

WRITE8_MEMBER(sega8_eeprom_device::write_cart)
{
	if (offset == 0x8000 && m_93c46_enabled)
	{
		m_93c46_lines = data;
		m_eeprom->di_write((data & 0x01) ? ASSERT_LINE : CLEAR_LINE);
		m_eeprom->cs_write((data & 0x04) ? ASSERT_LINE : CLEAR_LINE);
		m_eeprom->clk_write((data & 0x02) ? ASSERT_LINE : CLEAR_LINE);
	}
}

WRITE8_MEMBER(sega8_eeprom_device::write_mapper)
{
	switch (offset)
	{
		case 0:
			if (data & 0x80)
			{
				m_eeprom->reset();
				logerror("eeprom CS = 1\n");
				m_eeprom->cs_write(CLEAR_LINE);
			}
			m_93c46_enabled = BIT(data, 3);
			logerror("eeprom %s\n", m_93c46_enabled ? "enabled" : "disabled");
			break;

		case 1: // Select 16k ROM bank for 0000-3fff
		case 2: // Select 16k ROM bank for 4000-7fff
		case 3: // Select 16k ROM bank for 8000-bfff
			m_rom_bank_base[offset - 1] = data % m_rom_page_count;
			break;
	}
}

MACHINE_CONFIG_FRAGMENT( gg_eeprom )
	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")
MACHINE_CONFIG_END

machine_config_constructor sega8_eeprom_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( gg_eeprom );
}


/*-------------------------------------------------

 Codemasters carts, possibly having on cart RAM
 (Ernie Els Golf)

 -------------------------------------------------*/

READ8_MEMBER(sega8_codemasters_device::read_cart)
{
	int bank = offset / 0x2000;

	if (bank == 5 && !m_ram.empty() && m_ram_enabled)
		return m_ram[(m_ram_base * 0x2000 + (offset & 0x1fff)) % m_ram.size()];

	return m_rom[m_rom_bank_base[bank/2] * 0x4000 + (offset & 0x3fff)];
}

WRITE8_MEMBER(sega8_codemasters_device::write_cart)
{
	int bank = offset / 0x2000;

	switch (offset)
	{
		case 0x0000:
			m_rom_bank_base[0] = data % m_rom_page_count;
			break;
		case 0x4000:
			if (data & 0x80)
			{
				m_ram_enabled = 1;
				m_ram_base = data & 0x07;
			}
			else
			{
				m_ram_enabled = 0;
				m_rom_bank_base[1] = data % m_rom_page_count;
			}
			break;
		case 0x8000:
			m_rom_bank_base[2] = data % m_rom_page_count;
			break;
	}

	if (bank == 5 && !m_ram.empty() && m_ram_enabled)
		m_ram[(m_ram_base * 0x2000 + (offset & 0x1fff)) % m_ram.size()] = data;
}

/*-------------------------------------------------

 HES 4 PAK All Action cart

 -------------------------------------------------*/


READ8_MEMBER(sega8_4pak_device::read_cart)
{
	int bank = offset / 0x4000;

	return m_rom[m_rom_bank_base[bank] * 0x4000 + (offset & 0x3fff)];
}


WRITE8_MEMBER(sega8_4pak_device::write_cart)
{
	switch (offset)
	{
		case 0x3ffe:
			m_reg[0] = data;
			m_rom_bank_base[0] = data % m_rom_page_count;
			m_rom_bank_base[2] = ((m_reg[0] & 0x30) + m_reg[2]) % m_rom_page_count;
			break;
		case 0x7fff:
			m_reg[1] = data;
			m_rom_bank_base[1] = data % m_rom_page_count;
			break;
		case 0xbfff:
			m_reg[2] = data;
			m_rom_bank_base[2] = ((m_reg[0] & 0x30) + m_reg[2]) % m_rom_page_count;
			break;
	}
}


/*-------------------------------------------------

 base Zemina carts, allowing for 8K bankswitch

 -------------------------------------------------*/

READ8_MEMBER(sega8_zemina_device::read_cart)
{
	int bank = offset / 0x2000;

	if (bank >= 4 && !m_ram.empty() && m_ram_enabled)
		return m_ram[(m_ram_base * 0x2000 + (offset & 0x1fff)) % m_ram.size()];

	return m_rom[m_rom_bank_base[bank] * 0x2000 + (offset & 0x1fff)];
}

WRITE8_MEMBER(sega8_zemina_device::write_cart)
{
	int bank = offset / 0x2000;

	if (bank >= 4 && !m_ram.empty() && m_ram_enabled)
		m_ram[(m_ram_base * 0x2000 + (offset & 0x1fff)) % m_ram.size()] = data;

	if (offset < 4)
	{
		switch (offset & 3)
		{
			case 0:
				m_rom_bank_base[4] = data % (m_rom_page_count * 2);
				break;
			case 1:
				m_rom_bank_base[5] = data % (m_rom_page_count * 2);
				break;
			case 2:
				m_rom_bank_base[2] = data % (m_rom_page_count * 2);
				break;
			case 3:
				m_rom_bank_base[3] = data % (m_rom_page_count * 2);
				break;
		}
	}
}

/*-------------------------------------------------

 Zemina cart used for Nemesis: same as above, but
 different bank layout at start (see late_bank_setup)

 -------------------------------------------------*/

/*-------------------------------------------------

 Daiou cart used for SMS Janggun-ui Adeul

 the game expects to access 256 x 8K banks:
 first 64 are just the game, second 64 are a mirror of the first ones
 upper 128 are the same as the previous but with bytes in reverse order
 probably as a shortcut to get sprite flipping for free from hw
 so if bit7 of current bank page is set, we swap the result

 -------------------------------------------------*/

READ8_MEMBER(sega8_janggun_device::read_cart)
{
	int bank = offset / 0x2000;

	if (m_rom_bank_base[bank] < 0x80)
		return m_rom[(m_rom_bank_base[bank] & 0x3f) * 0x2000 + (offset & 0x1fff)];
	else
		return BITSWAP8(m_rom[(m_rom_bank_base[bank] & 0x3f) * 0x2000 + (offset & 0x1fff)], 0, 1, 2, 3, 4, 5, 6, 7);
}


WRITE8_MEMBER(sega8_janggun_device::write_cart)
{
	switch (offset)
	{
		case 0x4000:
			m_rom_bank_base[2] = data;
			break;
		case 0x6000:
			m_rom_bank_base[3] = data;
			break;
		case 0x8000:
			m_rom_bank_base[4] = data;
			break;
		case 0xa000:
			m_rom_bank_base[5] = data;
			break;
	}
}

WRITE8_MEMBER(sega8_janggun_device::write_mapper)
{
	switch (offset)
	{
		case 0:
			break;

		case 1: // Select 16k ROM bank for 0000-3fff
		case 2: // Select 16k ROM bank for 4000-7fff
		case 3: // Select 16k ROM bank for 8000-bfff
			m_rom_bank_base[(offset - 1) * 2] = (data  % m_rom_page_count) * 2;
			m_rom_bank_base[(offset - 1) * 2 + 1] = (data  % m_rom_page_count) * 2 + 1;
			break;
	}
}


/*-------------------------------------------------

 Hi-Com X-in-1 cart, uses writes to 0xffff to
 change program bank in 0x0000-0x7fff

 -------------------------------------------------*/

READ8_MEMBER(sega8_hicom_device::read_cart)
{
	if (offset >= 0x8000)
		return m_rom[offset & 0x3fff];

	return m_rom[(m_rom_bank_base * 0x8000) + offset];
}

WRITE8_MEMBER(sega8_hicom_device::write_mapper)
{
	if (offset == 0x03)
		m_rom_bank_base = data % (m_rom_page_count << 1);
}

/*-------------------------------------------------

 Korean cart, used e.g. in Dodgeball King,
 uses writes to 0xa000 for bankswitch

 -------------------------------------------------*/

WRITE8_MEMBER(sega8_korean_device::write_cart)
{
	int bank = offset / 0x4000;

	if (bank == 2 && !m_ram.empty() && m_ram_enabled)
		m_ram[m_ram_base * 0x4000 + (offset & 0x3fff)] = data;

	if (offset == 0xa000)
		m_rom_bank_base[2] = data % m_rom_page_count;
}
