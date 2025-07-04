// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Nigel Barnes
/******************************************************************************

Hardware Overview
-----------------
The Acorn Electron is a budget version of the BBC Micro home computer
Basic specs are:
6502 CPU @ 2MHz or 1MHz
32k RAM and 32k ROM
Text modes: 20x32, 40x25, 40x32, 80x25, 80x32
Graphics modes: 160x256 (4 or 16 colours), 320x256 (2 or 4 colours), 640x256 (2 colours), 320x200 (2 colors), 640x200 (2 colors)
Colours: 8 colours (either solid or flashing)
Sound: 1 channel, 7 octaves; built-in speaker
Internal ports for cassette storage, RGB/CVBS monitors and TV output
Various expansions can be added via the rear expansion port

PCB Layout
----------                           |-------------------|
                            |--------|  EXPANSION PORT   |
|-----|---------------------|                            |---||---------|
|MOD  | SPKR     16MHz                            LS169 18VAC||         []18VAC INPUT
|     |17.7345MHz               ROM                     18VAC|| POWER   |
|-----|       74S04                             |----|       || SUPPLY  |
|          KBD_CONN                    6502     |ULA |       ||         |
|CVBS                                           |----|       ||  +5VDC  |
|RGB                  LS00 LS86                              ||  -5VDC  |
|                                                         PWR||         |
|CASS         LM324   LS00 LS86 S74 LS74 4164 4164 4164 4164 ||         |
|------------------------------------------------------------||---------|
Notes: (all IC's shown. Only 16 ICs are used)
     6502 - 6502 CPU, clock input 2.000MHz [16/8]
      ULA - Custom logic chip 12CO21, also containing most of the BBC Micro circuitry
            Early PCB revisions used a PLCC68 chip in a socket. Later revisions used a
            PGA68 chip soldered directly into the motherboard
     4164 - 4164 64k x4-bit DRAM (4 chips for 32kbytes total)
      ROM - Hitachi HN613256 32k x8-bit mask ROM containing OS & BASIC
    LM324 - Texas Instruments LM324 Operational Amplifier
      MOD - UHF TV modulator UM1233-E36
     CVBS - Composite colour video output socket
      RGB - RGB monitor video output socket
     CASS - Cassette port
      PWR - 3-pin power input from internal power supply
 KBD_CONN - 22-pin keyboard connector
     SPKR - 2-pin internal speaker connector


Master RAM Board
----------------
The Master RAM Board (MRB) is an internal expansion board from Slogger, and could
also be purchased pre-fitted in the form of the Electron 64.
The MRB comes with a new MOS which provides three modes of operation, selected by a
switch, which are Normal, Turbo, and 64K. The 64K mode was the first to provide a
'shadow mode' on the Electron.


Stop Press 64i
--------------
The Stop Press 64i (SP64i) is another internal expansion board from Slogger, which
also requires the MRB to fitted. Being released in the early 90's there are no known
working examples, but bare prototype boards and ROMs were discovered during a Slogger
workshop clearout.
It provides a re-written AMX Stop Press (Desktop Publishing) for the Electron to take
advantage of the extra memory provided by the Master RAM Board, and also offers
ROM/RAM sockets and User Port for a mouse.

******************************************************************************
Emulation notes:

Incomplete:
    - 1 MHz bus is not emulated
    - Bus claiming by ULA is not implemented

Other internal boards to emulate:
    - Slogger Turbo Driver
    - Jafa Mode7 Mk2 Display Unit
    - move MRB and SP64i to an internal slot device?

******************************************************************************/

#include "emu.h"
#include "electron_ula.h"

#include "bus/bbc/userport/userport.h"
#include "bus/electron/exp.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "cpu/m6502/m6502.h"
#include "formats/uef_cas.h"
#include "formats/csw_cas.h"
#include "imagedev/cassette.h"
#include "machine/6522via.h"
#include "machine/input_merger.h"
#include "machine/ram.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class electron_state : public driver_device
{
public:
	electron_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_irqs(*this, "irqs")
		, m_screen(*this, "screen")
		, m_cassette(*this, "cassette")
		, m_ula(*this, "ula")
		, m_region_mos(*this, "mos")
		, m_keybd(*this, "LINE.%u", 0)
		, m_exp(*this, "exp")
		, m_ram(*this, RAM_TAG)
		, m_mrb(*this, "MRB")
		, m_capslock_led(*this, "capslock_led")
	{ }

	void electron(machine_config &config);
	void btm2105(machine_config &config);

	void electron64(machine_config &config);

	static void plus3_default(device_t* device);

	DECLARE_INPUT_CHANGED_MEMBER( trigger_reset );

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint8_t fetch_r(offs_t offset);
	uint8_t ram_r(offs_t offset);
	void ram_w(offs_t offset, uint8_t data);
	virtual uint8_t rom_r(offs_t offset);
	virtual void rom_w(offs_t offset, uint8_t data);
	virtual uint8_t io_r(offs_t offset);
	virtual void io_w(offs_t offset, uint8_t data);
	uint8_t keyboard_r(offs_t offset);

	void mem_map(address_map &map) ATTR_COLD;
	void opcodes_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<input_merger_device> m_irqs;
	required_device<screen_device> m_screen;
	required_device<cassette_image_device> m_cassette;
	required_device<electron_ula_device> m_ula;
	required_memory_region m_region_mos;
	required_ioport_array<14> m_keybd;
	required_device<electron_expansion_slot_device> m_exp;
	required_device<ram_device> m_ram;
	optional_ioport m_mrb;
	output_finder<> m_capslock_led;

	bool m_mrb_mapped = false;
	bool m_vdu_drivers = false;
};


class electronsp_state : public electron_state
{
public:
	electronsp_state(const machine_config &mconfig, device_type type, const char *tag)
		: electron_state(mconfig, type, tag)
		, m_region_sp64(*this, "sp64")
		, m_via(*this, "via6522")
		, m_userport(*this, "userport")
		, m_romi(*this, "romi%u", 1)
		, m_rompages(*this, "ROMPAGES")
	{ }

	void electronsp(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

	virtual uint8_t rom_r(offs_t offset) override;
	virtual void rom_w(offs_t offset, uint8_t data) override;
	virtual uint8_t io_r(offs_t offset) override;
	virtual void io_w(offs_t offset, uint8_t data) override;

private:
	required_memory_region m_region_sp64;
	required_device<via6522_device> m_via;
	required_device<bbc_userport_slot_device> m_userport;
	required_device_array<generic_slot_device, 2> m_romi;
	required_ioport m_rompages;

	uint8_t m_rompage = 0;
	uint8_t m_sp64_bank = 0;
	std::unique_ptr<uint8_t[]> m_sp64_ram;

	std::pair<std::error_condition, std::string> load_rom(device_image_interface &image, generic_slot_device *slot);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom1_load) { return load_rom(image, m_romi[0]); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(rom2_load) { return load_rom(image, m_romi[1]); }
};


void electron_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rw(FUNC(electron_state::ram_r), FUNC(electron_state::ram_w));          /* 32KB of RAM */
	map(0x8000, 0xffff).rw(FUNC(electron_state::rom_r), FUNC(electron_state::rom_w));          /* 32KB of ROM */
	map(0xfc00, 0xfeff).rw(FUNC(electron_state::io_r), FUNC(electron_state::io_w));            /* IO */
}

void electron_state::opcodes_map(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(electron_state::fetch_r));
}


uint8_t electron_state::keyboard_r(offs_t offset)
{
	uint8_t data = 0;

	for (int i = 0; i < 14; i++)
	{
		if (!BIT(offset, i))
			data |= m_keybd[i]->read() & 0x0f;
	}

	return data;
}


uint8_t electron_state::fetch_r(offs_t offset)
{
	m_vdu_drivers = (offset & 0xe000) == 0xc000 ? true : false;

	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}


uint8_t electron_state::ram_r(offs_t offset)
{
	uint8_t data = 0xff;

	data &= m_exp->expbus_r(offset);

	switch (m_mrb.read_safe(0))
	{
	case 0x00: /* Normal */
		data &= m_ula->read(offset);
		break;

	case 0x01: /* Turbo */
		if (m_mrb_mapped && offset < 0x3000) offset += 0x8000;
		data &= m_ram->read(offset);
		break;

	case 0x02: /* Shadow */
		if (m_mrb_mapped && (offset < 0x3000 || !m_vdu_drivers)) offset += 0x8000;
		data &= m_ram->read(offset);
		break;
	}

	return data;
}

void electron_state::ram_w(offs_t offset, uint8_t data)
{
	m_exp->expbus_w(offset, data);

	switch (m_mrb.read_safe(0))
	{
	case 0x00: /* Normal */
		m_ula->write(offset, data);
		break;

	case 0x01: /* Turbo */
		if (m_mrb_mapped && offset < 0x3000) offset += 0x8000;
		m_ram->write(offset, data);
		break;

	case 0x02: /* Shadow */
		if (m_mrb_mapped && (offset < 0x3000 || !m_vdu_drivers)) offset += 0x8000;
		m_ram->write(offset, data);
		break;
	}
}


uint8_t electron_state::rom_r(offs_t offset)
{
	/* 0,1   Second external socket on the expansion module (SK2) */
	/* 2,3   First external socket on the expansion module (SK1)  */
	/* 4     Disc                                                 */
	/* 5,6   USER applications                                    */
	/* 7     Modem interface ROM                                  */
	/* 8,9   Keyboard                                             */
	/* 10,11 BASIC                                                */
	/* 12    Expansion module operating system                    */
	/* 13    High priority slot in expansion module               */
	/* 14    ECONET                                               */
	/* 15    Reserved                                             */

	uint8_t data = 0xff;

	offset += 0x8000;

	data &= m_ula->read(offset);
	data &= m_exp->expbus_r(offset);

	return data;
}

void electron_state::rom_w(offs_t offset, uint8_t data)
{
	offset += 0x8000;

	m_ula->write(offset, data);
	m_exp->expbus_w(offset, data);
}

uint8_t electronsp_state::rom_r(offs_t offset)
{
	uint8_t data = electron_state::rom_r(offset);

	switch (offset & 0x4000)
	{
	case 0x0000:
		if ((m_rompage & 0x0e) == m_rompages->read())
		{
			data = m_romi[m_rompage & 0x01]->read_rom(offset);
		}
		else
		{
			switch (m_rompage)
			{
			case 10:
				/* SP64 ROM utilises the spare BASIC ROM page */
				if (BIT(m_sp64_bank, 7) && (offset & 0x2000))
				{
					data = m_sp64_ram[offset & 0x1fff];
				}
				else
				{
					data = m_region_sp64->base()[(!BIT(m_sp64_bank, 0) << 14) | offset];
				}
				break;
			}
		}
		break;
	}

	return data;
}

void electronsp_state::rom_w(offs_t offset, uint8_t data)
{
	electron_state::rom_w(offset, data);

	switch (offset & 0x4000)
	{
	case 0x0000:
		if ((m_rompage & 0x0e) == m_rompages->read())
		{
			/* TODO: sockets are writeable if RAM */
		}
		else
		{
			switch (m_rompage)
			{
			case 10:
				/* SP64 ROM utilises the spare BASIC ROM page */
				if (BIT(m_sp64_bank, 7) && (offset & 0x2000))
				{
					m_sp64_ram[offset & 0x1fff] = data;
				}
				break;
			}
		}
		break;
	}
}


uint8_t electron_state::io_r(offs_t offset)
{
	uint8_t data = 0xff;

	offset += 0xfc00;

	data &= m_ula->read(offset);
	data &= m_exp->expbus_r(offset);

	return data;
}

void electron_state::io_w(offs_t offset, uint8_t data)
{
	m_ula->write(0xfc00 + offset, data);

	offset += 0xfc00;

	/* Master RAM Board */
	if (offset == 0xfc7f)
	{
		m_mrb_mapped = !BIT(data, 7);
	}

	m_exp->expbus_w(offset, data);
}


uint8_t electronsp_state::io_r(offs_t offset)
{
	uint8_t data = electron_state::io_r(offset);

	offset += 0xfc00;

	if ((offset & 0xfff0) == 0xfcb0)
	{
		data = m_via->read(offset & 0x0f);
	}

	return data;
}

void electronsp_state::io_w(offs_t offset, uint8_t data)
{
	electron_state::io_w(offset, data);

	offset += 0xfc00;

	if ((offset & 0xfff0) == 0xfcb0)
	{
		m_via->write(offset & 0x0f, data);
	}
	else if (offset == 0xfcfa)
	{
		m_sp64_bank = data;
	}
	else if ((offset == 0xfe05) && !(data & 0xf0))
	{
		m_rompage = data & 0x0f;
	}
}


/**************************************
   Machine Initialisation functions
***************************************/

void electron_state::machine_start()
{
	m_capslock_led.resolve();

	/* set ULA RAM/ROM pointers */
	m_ula->set_ram(m_ram->pointer());
	m_ula->set_rom(m_region_mos->base());

	/* register save states */
	save_item(NAME(m_mrb_mapped));
	save_item(NAME(m_vdu_drivers));
}

void electron_state::machine_reset()
{
	m_mrb_mapped = true;
	m_vdu_drivers = false;
}

void electronsp_state::machine_start()
{
	electron_state::machine_start();

	m_sp64_ram = std::make_unique<uint8_t[]>(0x2000);

	/* register save states */
	save_item(NAME(m_sp64_bank));
	save_pointer(NAME(m_sp64_ram), 0x2000);
}


std::pair<std::error_condition, std::string> electronsp_state::load_rom(device_image_interface &image, generic_slot_device *slot)
{
	uint32_t size = slot->common_get_size("rom");

	// socket accepts 8K and 16K ROM only
	if (size != 0x2000 && size != 0x4000)
		return std::make_pair(image_error::INVALIDLENGTH, "Invalid size: Only 8K/16K is supported");

	slot->rom_alloc(0x4000, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	slot->common_load_rom(slot->get_rom_base(), size, "rom");

	// mirror 8K ROMs
	uint8_t *crt = slot->get_rom_base();
	if (size <= 0x2000) memcpy(crt + 0x2000, crt, 0x2000);

	return std::make_pair(std::error_condition(), std::string());
}


INPUT_CHANGED_MEMBER(electron_state::trigger_reset)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);
	if (newval)
	{
		m_exp->reset();
	}
}

static INPUT_PORTS_START( electron )
	PORT_START("LINE.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(u8"\u2192 | \\") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CHAR('|') PORT_CHAR('\\') // on the real keyboard, this would be on the 1st row, the 3rd key after 0
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("COPY") PORT_CODE(KEYCODE_END)  PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_CHAR('[') PORT_CHAR(']')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')

	PORT_START("LINE.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(u8"\u2190 ^ ~") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(u8"\u2193 _ }") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CHAR('_') PORT_CHAR('}')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER)     PORT_CHAR(13)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DELETE") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)

	PORT_START("LINE.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(u8"\u2191 £ {") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_CHAR(0xA3) PORT_CHAR('{')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("LINE.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)          PORT_CHAR('0') PORT_CHAR('@')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)          PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("LINE.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)          PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)          PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')

	PORT_START("LINE.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)          PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)          PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')

	PORT_START("LINE.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)          PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)          PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)          PORT_CHAR('M') PORT_CHAR('m')

	PORT_START("LINE.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)          PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)          PORT_CHAR('N') PORT_CHAR('n')

	PORT_START("LINE.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)          PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)          PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)          PORT_CHAR('B') PORT_CHAR('b')

	PORT_START("LINE.9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)          PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)          PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)          PORT_CHAR('V') PORT_CHAR('v')

	PORT_START("LINE.10")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)          PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)          PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)          PORT_CHAR('C') PORT_CHAR('c')

	PORT_START("LINE.11")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)          PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)          PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)          PORT_CHAR('X') PORT_CHAR('x')

	PORT_START("LINE.12")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)          PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_CHAR('Z') PORT_CHAR('z')

	PORT_START("LINE.13")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ESCAPE") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("FUNC") PORT_CODE(KEYCODE_TAB) PORT_CHAR(UCHAR_MAMEKEY(LALT))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT) PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("BRK")       /* BREAK */
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("BREAK") PORT_CODE(KEYCODE_F12) PORT_CHAR(UCHAR_MAMEKEY(F12)) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(electron_state::trigger_reset), 0)
INPUT_PORTS_END

static INPUT_PORTS_START( electron64 )
	PORT_INCLUDE(electron)

	PORT_START("MRB")
	PORT_CONFNAME(0x03, 0x02, "MRB Mode")
	PORT_CONFSETTING(0x00, "Normal")
	PORT_CONFSETTING(0x01, "Turbo")
	PORT_CONFSETTING(0x02, "64K")
INPUT_PORTS_END

static INPUT_PORTS_START(electronsp)
	PORT_INCLUDE(electron64)

	PORT_START("ROMPAGES")
	/* TODO: Actual 4bit dip settings are unknown, require a manual */
	PORT_DIPNAME(0x0f, 0x06, "SP64i ROM Pages")
	PORT_DIPSETTING(0x00, "0&1")
	PORT_DIPSETTING(0x02, "2&3")
	PORT_DIPSETTING(0x04, "4&5")
	PORT_DIPSETTING(0x06, "6&7")
	PORT_DIPSETTING(0x08, "8&9 (reserved)")
	PORT_DIPSETTING(0x0a, "10&11 (reserved)")
	PORT_DIPSETTING(0x0c, "12&13")
	PORT_DIPSETTING(0x0e, "14&15")
INPUT_PORTS_END

void electron_state::plus3_default(device_t* device)
{
	device->subdevice<electron_expansion_slot_device>("exp")->set_default_option("plus1");
}

void electron_state::electron(machine_config &config)
{
	M6502(config, m_maincpu, 16_MHz_XTAL / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &electron_state::mem_map);
	config.set_perfect_quantum(m_maincpu);

	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(16_MHz_XTAL, 1024, 0, 640, 312, 0, 256);
	m_screen->set_screen_update(m_ula, FUNC(electron_ula_device::screen_update));
	m_screen->set_video_attributes(VIDEO_UPDATE_SCANLINE);

	PALETTE(config, "palette").set_entries(8);

	SPEAKER(config, "mono").front_center();

	RAM(config, m_ram).set_default_size("32K");

	ELECTRON_ULA(config, m_ula, 16_MHz_XTAL);
	m_ula->set_cpu_tag("maincpu");
	m_ula->kbd_cb().set(FUNC(electron_state::keyboard_r));
	m_ula->caps_lock_cb().set([this](int state) { m_capslock_led = state; });
	m_ula->cas_mo_cb().set([this](int state) { m_cassette->set_motor(state); });
	m_ula->cas_in_cb("cassette", FUNC(cassette_image_device::input));
	m_ula->cas_out_cb("cassette", FUNC(cassette_image_device::output));
	m_ula->add_route(ALL_OUTPUTS, "mono", 0.25);
	m_ula->irq_cb().set(m_irqs, FUNC(input_merger_device::in_w<0>));

	CASSETTE(config, m_cassette);
	m_cassette->set_formats(bbc_cassette_formats);
	m_cassette->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED);
	m_cassette->set_interface("bbc_cass");

	/* expansion port */
	ELECTRON_EXPANSION_SLOT(config, m_exp, 16_MHz_XTAL, electron_expansion_devices, "plus3");
	m_exp->set_option_machine_config("plus3", plus3_default);
	m_exp->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<1>));
	m_exp->nmi_handler().set_inputline(m_maincpu, M6502_NMI_LINE);

	/* software lists */
	SOFTWARE_LIST(config, "cass_list").set_original("electron_cass");
	SOFTWARE_LIST(config, "cass_list_bbc").set_compatible("bbc_cass").set_filter("ELK");
	SOFTWARE_LIST(config, "cart_list").set_original("electron_cart");
	SOFTWARE_LIST(config, "flop_list").set_original("electron_flop");
	SOFTWARE_LIST(config, "rom_list").set_original("electron_rom");
}


void electron_state::btm2105(machine_config &config)
{
	electron(config);

	m_screen->set_color(rgb_t::amber());

	/* expansion port */
	m_exp->set_default_option("m2105");
	m_exp->set_fixed(true);

	/* software lists */
	config.device_remove("cass_list");
	config.device_remove("cart_list");
	config.device_remove("flop_list");
	config.device_remove("rom_list");
}


void electron_state::electron64(machine_config &config)
{
	electron(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &electron_state::mem_map);
	m_maincpu->set_addrmap(AS_OPCODES, &electron_state::opcodes_map);

	m_ram->set_default_size("64K");
}


void electronsp_state::electronsp(machine_config &config)
{
	/* install mrb board */
	electron64(config);

	/* rom sockets */
	GENERIC_SOCKET(config, m_romi[0], generic_plain_slot, "electron_rom", "bin,rom");
	m_romi[0]->set_device_load(FUNC(electronsp_state::rom1_load));
	GENERIC_SOCKET(config, m_romi[1], generic_plain_slot, "electron_rom", "bin,rom");
	m_romi[1]->set_device_load(FUNC(electronsp_state::rom2_load));

	/* via */
	MOS6522(config, m_via, 16_MHz_XTAL / 16);
	m_via->readpb_handler().set(m_userport, FUNC(bbc_userport_slot_device::pb_r));
	m_via->writepb_handler().set(m_userport, FUNC(bbc_userport_slot_device::pb_w));
	m_via->cb1_handler().set(m_userport, FUNC(bbc_userport_slot_device::write_cb1));
	m_via->cb2_handler().set(m_userport, FUNC(bbc_userport_slot_device::write_cb2));
	m_via->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<2>));

	/* user port */
	BBC_USERPORT_SLOT(config, m_userport, bbc_userport_devices, "amxmouse");
	m_userport->cb1_handler().set(m_via, FUNC(via6522_device::write_cb1));
	m_userport->cb2_handler().set(m_via, FUNC(via6522_device::write_cb2));
}


ROM_START(electron)
	ROM_REGION( 0x8000, "mos", 0 )
	ROM_LOAD( "os_basic.ic2", 0x0000, 0x8000, CRC(b997f9cb) SHA1(4a66c83aba07d0a8e76ed8a5545af04e11c19fdc) )
ROM_END

ROM_START(electront)
	ROM_REGION( 0x8000, "mos",  0 )
	/* Serial 06-ALA01-0000087 from Centre for Computing History */
	ROM_LOAD( "basic.rom", 0x0000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281) )
	ROM_LOAD( "elk_036.rom", 0x4000, 0x4000, CRC(dd1a99c3) SHA1(87ee1b14895e476909dd002d5ca2346a3a5f3f57) )
ROM_END

ROM_START(electron64)
	ROM_REGION( 0x8000, "mos", 0 )
	ROM_LOAD( "basic.rom", 0x0000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281) )
	ROM_LOAD( "os_300.rom", 0x4000, 0x4000, CRC(f80a0cea) SHA1(165e42ff4164a842e56f08ebd420d5027af99fdd) )
ROM_END

ROM_START(electronsp)
	ROM_REGION( 0x8000, "mos", 0 )
	ROM_LOAD( "basic.rom", 0x0000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281) )
	ROM_LOAD( "os_310.rom", 0x4000, 0x4000, CRC(8b7a9003) SHA1(6d4e2f8ddc1d829b14206d2747749c4c24789568) )
	ROM_REGION( 0x8000, "sp64", 0 )
	ROM_SYSTEM_BIOS( 0, "101_2", "v1.01 (2x16K)" )
	ROMX_LOAD( "sp64_101_1.rom", 0x0000, 0x4000, CRC(07e2c5d6) SHA1(837e3382c376e3cc1ae42f1ca51158657ef2fd73), ROM_BIOS(0) )
	ROMX_LOAD( "sp64_101_2.rom", 0x4000, 0x4000, CRC(3d0e5dc1) SHA1(89743c43b24950d481c150fd4b4de985600cca2d), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "100", "v1.00 (32K)" )
	ROMX_LOAD( "sp64_100.rom", 0x0000, 0x8000, CRC(4918221c) SHA1(f185873106e7e7225b2e0c718803dc1ec4ebc685), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "100_2", "v1.00 (2x16K)" )
	ROMX_LOAD( "sp64_100_1.rom", 0x0000, 0x4000, CRC(6053e5a0) SHA1(6d79e5494349f157672e7c59949f3941e5a3dbdb), ROM_BIOS(2) )
	ROMX_LOAD( "sp64_100_2.rom", 0x4000, 0x4000, CRC(25d11d8e) SHA1(c1bceeb50fee1e11de7505a3b664b844cfb56289), ROM_BIOS(2) )
ROM_END

#define rom_btm2105 rom_electron

} // anonymous namespace


/*     YEAR  NAME        PARENT    COMPAT  MACHINE     INPUT       CLASS             INIT        COMPANY                             FULLNAME                                 FLAGS */
COMP ( 1983, electron,   0,        0,      electron,   electron,   electron_state,   empty_init, "Acorn Computers",                  "Acorn Electron",                        MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
COMP ( 1983, electront,  electron, 0,      electron,   electron,   electron_state,   empty_init, "Acorn Computers",                  "Acorn Electron (Trial)",                MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
COMP ( 1985, btm2105,    electron, 0,      btm2105,    electron,   electron_state,   empty_init, "British Telecom Business Systems", "BT Merlin M2105",                       MACHINE_NOT_WORKING )
COMP ( 1987, electron64, electron, 0,      electron64, electron64, electron_state,   empty_init, "Acorn Computers / Slogger",        "Acorn Electron (64K Master RAM Board)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
COMP ( 1991, electronsp, electron, 0,      electronsp, electronsp, electronsp_state, empty_init, "Acorn Computers / Slogger",        "Acorn Electron (Stop Press 64i)",       MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
