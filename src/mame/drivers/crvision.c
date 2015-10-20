// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Curt Coder
/*
    This 1980s computer was manufactured by Vtech of Hong Kong.
    known as: CreatiVision, Dick Smith Wizzard, Funvision, Rameses, VZ 2000 and possibly others.

    There is also a CreatiVision Mk 2, possibly also known as the Laser 500. This was a hardware variant,
    sort of "evolved" hardware made compatible for the scheduled "ColecoVision expansion module", which
    never actually shipped for CreatiVision, but for the Laser 2001 home computer (successor to
    CreatiVision).

    TODO:

    CreatiVision

    - fix Diagnostic A (video) sprites generator test
    - proper keyboard emulation, need keyboard schematics
    - memory expansion 16K, can be chained
    - centronics control/status port
    - non-working cartridges:
        * Diagnostic B (keyboard)
    - homebrew roms with graphics issues:
        * Christmas Demo 1.0
        * Titanic Frogger Demo 1.0
        * Titanic Frogger Demo 1.1

    Salora Manager

    - keyboard Ctrl+I/J/N don't print out BASIC commands
    - RAM mapping
    - cassette (figure out correct input level)
    - floppy (interface cartridge needed)

*/

/*

Salora Manager

PCB Layout
----------

Main board

35 0352 02
700391F

|-------------------------------------------------------------------------------|
|               |-----CN1-----|             |-----CN2-----|                     |
|                                                                   10.738MHz   --|
|                                   4116                                    CN3   |
|       ROM01                                               VDC                   |
|                                   4116                                        --|
|                                                                               |
|       ROM23                       4116    -                                   |
|                                           |                                   |
|                                   4116    |                                   |
|   LS04                                   CN6                                  --|
|                                   4116    |                  6821               |
|   LS32                                    |                                     |
|                                   4116    -                                     |
|   LS139                                           PSG                           |
|                                   4116                                          |
|   LS138                                                                         |
|                                   4116                                    CN4   |
|   LS244                                                                         |
|                                                                                 |
|   LS245                                                                         |
|                                                                                 |
|                   LS244                                                         |
|       6502                                                                    --|
|                   LS244                                                       |
|                                           |-------------CN5-------------|     |
|-------------------------------------------------------------------------------|

Notes:
All IC's shown. Prototype-ish board, with many jumper wires and extra capacitors.

ROM01   - Toshiba TMM2464P 8Kx8 one-time PROM, labeled "0.1"
ROM23   - Toshiba TMM2464P 8Kx8 one-time PROM, labeled "23"
6502    - Rockwell R6502AP 8-bit Microprocessor
6821    - Hitachi HD468B21P Peripheral Interface Adaptor
VDC     - Texas Instruments TMS9929A Video Display Controller (covered w/heatsink)
PSG     - Texas Instruments SN76489AN Programmable Sound Generator
4116    - Toshiba TMM416P-3 16Kx1 RAM (covered w/heatsink)
CN1     - sub board connector (17x2 pin header)
CN2     - RF board connector (17x1 pin header)
CN3     - printer connector (7x2 PCB edge male)
CN4     - expansion connector (30x2 PCB edge male)
CN5     - cartridge connector (18x2 PCB edge female)
CN6     - keyboard connector (16x1 pin header)


Sub board

700472
35 0473 03

|---------------------------------------|
|   17.73447MHz |-----CN1-----|         |
|                                       |
|   74S04                       4116    |
|                                       |
|   LS90                        4116    |
|                                       |
|   LS10                        4116    |
|                                       |
|   LS367                       4116    |
|                                       |
|   LS393                       4116    |
|                                       |
|   LS244                       4116    |
|                                       |
|   LS257                       4116    |
|                                       |
|   LS257                       4116    |
|                                       |
|                               LS139   |
|                                       |
|---------------------------------------|

Notes:
All IC's shown.

4116    - Toshiba TMM416P-3 16Kx1 RAM
CN1     - main board connector (17x2 pin header)

*/

#include "includes/crvision.h"

/***************************************************************************
    MEMORY MAPS
***************************************************************************/

/*-------------------------------------------------
    ADDRESS_MAP( crvision_map )
-------------------------------------------------*/

static ADDRESS_MAP_START( crvision_map, AS_PROGRAM, 8, crvision_state )
	AM_RANGE(0x0000, 0x03ff) AM_MIRROR(0x0c00) AM_RAM
	AM_RANGE(0x1000, 0x1003) AM_MIRROR(0x0ffc) AM_DEVREADWRITE(PIA6821_TAG, pia6821_device, read, write)
	AM_RANGE(0x2000, 0x2000) AM_MIRROR(0x0ffe) AM_DEVREAD(TMS9929_TAG, tms9928a_device, vram_read)
	AM_RANGE(0x2001, 0x2001) AM_MIRROR(0x0ffe) AM_DEVREAD(TMS9929_TAG, tms9928a_device, register_read)
	AM_RANGE(0x3000, 0x3000) AM_MIRROR(0x0ffe) AM_DEVWRITE(TMS9929_TAG, tms9928a_device, vram_write)
	AM_RANGE(0x3001, 0x3001) AM_MIRROR(0x0ffe) AM_DEVWRITE(TMS9929_TAG, tms9928a_device, register_write)
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK(BANK_ROM2)
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK(BANK_ROM1)
//  AM_RANGE(0xc000, 0xe7ff) AM_RAMBANK(3)
	AM_RANGE(0xe800, 0xe800) AM_DEVWRITE("cent_data_out", output_latch_device, write)
	AM_RANGE(0xe801, 0xe801) AM_DEVREAD("cent_status_in", input_buffer_device, read)
	AM_RANGE(0xe801, 0xe801) AM_DEVWRITE("cent_ctrl_out", output_latch_device, write)
//  AM_RANGE(0xe802, 0xf7ff) AM_RAMBANK(4)
	AM_RANGE(0xf800, 0xffff) AM_ROM AM_REGION(M6502_TAG, 0)
ADDRESS_MAP_END

/*-------------------------------------------------
    ADDRESS_MAP( lasr2001_map )
-------------------------------------------------*/

static ADDRESS_MAP_START( lasr2001_map, AS_PROGRAM, 8, laser2001_state )
	AM_RANGE(0x0000, 0x03ff) AM_MIRROR(0x0c00) AM_RAM
	AM_RANGE(0x1000, 0x1003) AM_MIRROR(0x0ffc) AM_DEVREADWRITE(PIA6821_TAG, pia6821_device, read, write)
	AM_RANGE(0x2000, 0x2000) AM_MIRROR(0x0ffe) AM_DEVREAD(TMS9929_TAG, tms9928a_device, vram_read)
	AM_RANGE(0x2001, 0x2001) AM_MIRROR(0x0ffe) AM_DEVREAD(TMS9929_TAG, tms9928a_device, register_read)
	AM_RANGE(0x3000, 0x3000) AM_MIRROR(0x0ffe) AM_DEVWRITE(TMS9929_TAG, tms9928a_device, vram_write)
	AM_RANGE(0x3001, 0x3001) AM_MIRROR(0x0ffe) AM_DEVWRITE(TMS9929_TAG, tms9928a_device, register_write)
	AM_RANGE(0x4000, 0x7fff) AM_RAMBANK(BANK_ROM2)
	AM_RANGE(0x8000, 0xbfff) AM_RAMBANK(BANK_ROM1)
	AM_RANGE(0xc000, 0xffff) AM_ROM AM_REGION(M6502_TAG, 0)
ADDRESS_MAP_END

/***************************************************************************
    INPUT PORTS
***************************************************************************/

/*-------------------------------------------------
    INPUT_CHANGED_MEMBER( trigger_nmi )
-------------------------------------------------*/

INPUT_CHANGED_MEMBER( crvision_state::trigger_nmi )
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}

/*-------------------------------------------------
    INPUT_PORTS( crvision )
-------------------------------------------------*/

static INPUT_PORTS_START( crvision )
	// Player 1 Joystick

	PORT_START("PA0.0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA0.1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0xfd, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA0.2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0xf3, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA0.3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0xf7, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA0.4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA0.5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0xdf, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA0.6")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA0.7")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Button 2 / CNT'L") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)

	// Player 1 Keyboard

	PORT_START("PA1.0")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x81, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA1.1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT( 0x83, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA1.2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT( 0x87, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA1.3")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x8f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA1.4")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x9f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA1.5")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0xbf, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA1.6")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA1.7")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Button 1 / SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)

	// Player 2 Joystick

	PORT_START("PA2.0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA2.1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0xfd, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA2.2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0xf3, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA2.3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0xf7, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA2.4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA2.5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0xdf, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA2.6")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA2.7")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P2 Button 2 / \xE2\x86\x92") PORT_CODE(KEYCODE_TAB) PORT_CHAR(9) PORT_PLAYER(2)

	// Player 2 Keyboard

	PORT_START("PA3.0")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RET'N") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('\r')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT( 0x81, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA3.1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('@')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x83, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA3.2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0x87, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA3.3")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT( 0x8f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA3.4")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT( 0x9f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA3.5")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0xbf, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA3.6")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA3.7")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 Button 1 / - =") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=') PORT_PLAYER(2)

	PORT_START("NMI")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F10) PORT_CHANGED_MEMBER(DEVICE_SELF, crvision_state, trigger_nmi, 0)
INPUT_PORTS_END

/*-------------------------------------------------
    INPUT_PORTS( manager )
-------------------------------------------------*/

static INPUT_PORTS_START( manager )
	PORT_START("Y.0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('\r')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('-')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')

	PORT_START("Y.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xC3\x84 \xC3\xA4") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(0x00C4) PORT_CHAR(0x00E4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z')

	PORT_START("Y.2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xC3\x85 \xC3\xA5") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(0x00C5) PORT_CHAR(0x00E5)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("Y.3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xC3\x96 \xC3\xB6") PORT_CODE(KEYCODE_COLON) PORT_CHAR(0x00D6) PORT_CHAR(0x00F6)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c')

	PORT_START("Y.4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v')

	PORT_START("Y.5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('=')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b')

	PORT_START("Y.6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n')

	PORT_START("Y.7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m')

	PORT_START("JOY.0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("JOY.1")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_START("JOY.2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("JOY.3")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
INPUT_PORTS_END

/***************************************************************************
    DEVICE CONFIGURATION
***************************************************************************/

WRITE8_MEMBER( crvision_state::pia_pa_w )
{
	/*
	    Signal  Description

	    PA0     Keyboard raster player 1 output (joystick)
	    PA1     Keyboard raster player 1 output (hand keys)
	    PA2     Keyboard raster player 2 output (joystick)
	    PA3     Keyboard raster player 2 output (hand keys)
	    PA4     ?
	    PA5     ?
	    PA6     Cassette motor
	    PA7     Cassette data in/out
	*/

	/* keyboard raster */
	m_keylatch = ~data & 0x0f;

	/* cassette motor */
	m_cassette->change_state(BIT(data,6) ? CASSETTE_MOTOR_DISABLED : CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);

	/* cassette data output */
	m_cassette->output( BIT(data, 7) ? +1.0 : -1.0);
}

UINT8 crvision_state::read_keyboard(int pa)
{
	UINT8 value = 0;

	for (int i = 0; i < 8; i++)
	{
		switch (pa & 3)
		{
			case 0: value = m_inp_pa0[i]->read(); break;
			case 1: value = m_inp_pa1[i]->read(); break;
			case 2: value = m_inp_pa2[i]->read(); break;
			case 3: value = m_inp_pa3[i]->read(); break;
		}

		if (value != 0xff)
		{
			if (value == 0xff - (1 << i))
				return value;
			else
				return value - (1 << i);
		}
	}

	return 0xff;
}

READ8_MEMBER( crvision_state::pia_pa_r )
{
	/*
	    PA0     Keyboard raster player 1 output (joystick)
	    PA1     Keyboard raster player 1 output (hand keys)
	    PA2     Keyboard raster player 2 output (joystick)
	    PA3     Keyboard raster player 2 output (hand keys)
	    PA4     ?
	    PA5     ?
	    PA6     Cassette motor
	    PA7     Cassette data in/out
	*/

	UINT8 data = 0x7f;

	if ((m_cassette)->input() > -0.1469) data |= 0x80;

	return data;
}

READ8_MEMBER( crvision_state::pia_pb_r )
{
	/*
	    Signal  Description

	    PB0     Keyboard input
	    PB1     Keyboard input
	    PB2     Keyboard input
	    PB3     Keyboard input
	    PB4     Keyboard input
	    PB5     Keyboard input
	    PB6     Keyboard input
	    PB7     Keyboard input
	*/

	UINT8 data = 0xff;

	for (int i = 0; i < 4; i++)
		if (m_keylatch >> i & 1)
			data &= read_keyboard(i);

	return data;
}

READ8_MEMBER( laser2001_state::pia_pa_r )
{
	/*
	    Signal  Description

	    PA0     Keyboard column 0
	    PA1     Keyboard column 1
	    PA2     Keyboard column 2
	    PA3     Keyboard column 3
	    PA4     Keyboard column 4
	    PA5     Keyboard column 5
	    PA6     Keyboard column 6
	    PA7     Keyboard column 7
	*/

	UINT8 data = 0xff;

	for (int i = 0; i < 8; i++)
		if (~m_keylatch >> i & 1)
			data &= m_inp_y[i]->read();

	return data;
}

WRITE8_MEMBER( laser2001_state::pia_pa_w )
{
	/*
	    PA0     Joystick player 1 output 0
	    PA1     Joystick player 1 output 1
	    PA2     Joystick player 2 output 0
	    PA3     Joystick player 2 output 1
	    PA4     ?
	    PA5     ?
	    PA6     ?
	    PA7     ?
	*/

	m_joylatch = data;
}

READ8_MEMBER( laser2001_state::pia_pb_r )
{
	UINT8 data = 0xff;

	for (int i = 0; i < 4; i++)
		if (~m_joylatch >> i & 1)
			data &= m_inp_joy[i]->read();

	return data;
}

WRITE8_MEMBER( laser2001_state::pia_pb_w )
{
	/*
	    Signal  Description

	    PB0     Keyboard row 0, PSG data 7, centronics data 0
	    PB1     Keyboard row 1, PSG data 6, centronics data 1
	    PB2     Keyboard row 2, PSG data 5, centronics data 2
	    PB3     Keyboard row 3, PSG data 4, centronics data 3
	    PB4     Keyboard row 4, PSG data 3, centronics data 4
	    PB5     Keyboard row 5, PSG data 2, centronics data 5
	    PB6     Keyboard row 6, PSG data 1, centronics data 6
	    PB7     Keyboard row 7, PSG data 0, centronics data 7
	*/

	/* keyboard latch */
	m_keylatch = data;

	/* centronics data */
	m_cent_data_out->write(space, 0, data);
}

READ_LINE_MEMBER( laser2001_state::pia_ca1_r )
{
	return (m_cassette)->input() > -0.1469;
}

WRITE_LINE_MEMBER( laser2001_state::pia_ca2_w )
{
	m_cassette->output(state ? +1.0 : -1.0);
}


WRITE_LINE_MEMBER(laser2001_state::write_centronics_busy)
{
	m_centronics_busy = state;
	m_pia->cb1_w(pia_cb1_r());
}

WRITE_LINE_MEMBER(laser2001_state::write_psg_ready)
{
	m_psg_ready = state;
	m_pia->cb1_w(pia_cb1_r());
}

READ_LINE_MEMBER( laser2001_state::pia_cb1_r )
{
	/* actually this is a diode-AND (READY & _BUSY), but ctronics.c returns busy status if no device is mounted -> Manager won't boot */
	return m_psg_ready && (!m_centronics_busy || m_pia->ca2_output_z());
}

WRITE_LINE_MEMBER( laser2001_state::pia_cb2_w )
{
	if (m_pia->ca2_output_z())
	{
		if (!state) m_psg->write(m_keylatch);
	}
	else
	{
		m_centronics->write_strobe(state);
	}
}

/***************************************************************************
    MACHINE INITIALIZATION
***************************************************************************/

void crvision_state::machine_start()
{
	// zerofill/state saving
	m_keylatch = 0;
	save_item(NAME(m_keylatch));

	if (m_cart->exists())
	{
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x4000, 0x7fff, read8_delegate(FUNC(crvision_cart_slot_device::read_rom40),(crvision_cart_slot_device*)m_cart));
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x8000, 0xbfff, read8_delegate(FUNC(crvision_cart_slot_device::read_rom80),(crvision_cart_slot_device*)m_cart));
	}
}

void laser2001_state::machine_start()
{
	crvision_state::machine_start();

	// zerofill/state saving
	m_joylatch = 0;
	m_centronics_busy = 0;
	m_psg_ready = 0;

	save_item(NAME(m_joylatch));
	save_item(NAME(m_centronics_busy));
	save_item(NAME(m_psg_ready));
}


/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

static SLOT_INTERFACE_START(crvision_cart)
	SLOT_INTERFACE_INTERNAL("crv_rom4k",  CRVISION_ROM_4K)
	SLOT_INTERFACE_INTERNAL("crv_rom6k",  CRVISION_ROM_6K)
	SLOT_INTERFACE_INTERNAL("crv_rom8k",  CRVISION_ROM_8K)
	SLOT_INTERFACE_INTERNAL("crv_rom10k", CRVISION_ROM_10K)
	SLOT_INTERFACE_INTERNAL("crv_rom12k", CRVISION_ROM_12K)
	SLOT_INTERFACE_INTERNAL("crv_rom16k", CRVISION_ROM_16K)
	SLOT_INTERFACE_INTERNAL("crv_rom18k", CRVISION_ROM_18K)
SLOT_INTERFACE_END

/*-------------------------------------------------
    MACHINE_CONFIG_START( creativision, crvision_state )
-------------------------------------------------*/

static MACHINE_CONFIG_START( creativision, crvision_state )
	// basic machine hardware
	MCFG_CPU_ADD(M6502_TAG, M6502, XTAL_2MHz)
	MCFG_CPU_PROGRAM_MAP(crvision_map)

	// devices
	MCFG_DEVICE_ADD(PIA6821_TAG, PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(crvision_state, pia_pa_r))
	MCFG_PIA_READPB_HANDLER(READ8(crvision_state, pia_pb_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(crvision_state, pia_pa_w))
	MCFG_PIA_WRITEPB_HANDLER(DEVWRITE8(SN76489_TAG, sn76496_base_device, write))

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED)

	MCFG_CENTRONICS_ADD(CENTRONICS_TAG, centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(DEVWRITELINE("cent_status_in", input_buffer_device, write_bit7))

	MCFG_DEVICE_ADD("cent_status_in", INPUT_BUFFER, 0)

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", CENTRONICS_TAG)

	MCFG_DEVICE_ADD("cent_ctrl_out", OUTPUT_LATCH, 0)
	MCFG_OUTPUT_LATCH_BIT4_HANDLER(DEVWRITELINE(CENTRONICS_TAG, centronics_device, write_strobe))

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SN76489_TAG, SN76489A, XTAL_2MHz)
	MCFG_SN76496_READY_HANDLER(DEVWRITELINE(PIA6821_TAG, pia6821_device, cb1_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(1, "mono", 0.25)

	// cartridge
	MCFG_CRVISION_CARTRIDGE_ADD("cartslot", crvision_cart, NULL)

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("1K") // MAIN RAM
	MCFG_RAM_EXTRA_OPTIONS("15K") // 16K expansion (lower 14K available only, upper 2K shared with BIOS ROM)

	// software lists
	MCFG_SOFTWARE_LIST_ADD("cart_list", "crvision")
MACHINE_CONFIG_END

/*-------------------------------------------------
    MACHINE_CONFIG_DERIVED( ntsc, creativision )
-------------------------------------------------*/

static MACHINE_CONFIG_DERIVED( ntsc, creativision )
	// video hardware
	MCFG_DEVICE_ADD( TMS9929_TAG, TMS9918, XTAL_10_738635MHz / 2 )
	MCFG_TMS9928A_VRAM_SIZE(0x4000)
	MCFG_TMS9928A_OUT_INT_LINE_CB(INPUTLINE(M6502_TAG, INPUT_LINE_IRQ0))
	MCFG_TMS9928A_SCREEN_ADD_NTSC( SCREEN_TAG )
	MCFG_SCREEN_UPDATE_DEVICE( TMS9929_TAG, tms9918_device, screen_update )
MACHINE_CONFIG_END

/*-------------------------------------------------
    MACHINE_CONFIG_DERIVED( pal, creativision )
-------------------------------------------------*/

static MACHINE_CONFIG_DERIVED_CLASS( pal, creativision, crvision_pal_state )
	// video hardware
	MCFG_DEVICE_ADD( TMS9929_TAG, TMS9929, XTAL_10_738635MHz / 2 )
	MCFG_TMS9928A_VRAM_SIZE(0x4000)
	MCFG_TMS9928A_OUT_INT_LINE_CB(INPUTLINE(M6502_TAG, INPUT_LINE_IRQ0))
	MCFG_TMS9928A_SCREEN_ADD_PAL( SCREEN_TAG )
	MCFG_SCREEN_UPDATE_DEVICE( TMS9929_TAG, tms9929_device, screen_update )
MACHINE_CONFIG_END

/*-------------------------------------------------
    MACHINE_CONFIG_START( lasr2001, laser2001_state )
-------------------------------------------------*/

static MACHINE_CONFIG_START( lasr2001, laser2001_state )
	// basic machine hardware
	MCFG_CPU_ADD(M6502_TAG, M6502, XTAL_17_73447MHz/9)
	MCFG_CPU_PROGRAM_MAP(lasr2001_map)

	// devices
	MCFG_DEVICE_ADD(PIA6821_TAG, PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(laser2001_state, pia_pa_r))
	MCFG_PIA_READPB_HANDLER(READ8(laser2001_state, pia_pb_r))
	MCFG_PIA_READCA1_HANDLER(READLINE(laser2001_state, pia_ca1_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(laser2001_state, pia_pa_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(laser2001_state, pia_pb_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(laser2001_state, pia_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(laser2001_state, pia_cb2_w))

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED)

	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(laser2001_state, write_centronics_busy))

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", CENTRONICS_TAG)

	// video hardware
	MCFG_DEVICE_ADD( TMS9929_TAG, TMS9929A, XTAL_10_738635MHz / 2 )
	MCFG_TMS9928A_VRAM_SIZE(0x4000)
	MCFG_TMS9928A_OUT_INT_LINE_CB(INPUTLINE(M6502_TAG, INPUT_LINE_IRQ0))
	MCFG_TMS9928A_SCREEN_ADD_PAL( SCREEN_TAG )
	MCFG_SCREEN_UPDATE_DEVICE( TMS9929_TAG, tms9929a_device, screen_update )

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SN76489_TAG, SN76489A, XTAL_17_73447MHz/9)
	MCFG_SN76496_READY_HANDLER(WRITELINE(laser2001_state, write_psg_ready))

	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(1, "mono", 0.25)

	// cartridge
	MCFG_CRVISION_CARTRIDGE_ADD("cartslot", crvision_cart, NULL)

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("16K")
	MCFG_RAM_EXTRA_OPTIONS("32K")

	// software list
	MCFG_SOFTWARE_LIST_ADD("cart_list","crvision")
MACHINE_CONFIG_END

/***************************************************************************
    ROMS
***************************************************************************/

ROM_START( crvision )
	ROM_REGION( 0x10000, M6502_TAG, 0 )
	ROM_LOAD( "crvision.u20", 0x0000, 0x0800, CRC(c3c590c6) SHA1(5ac620c529e4965efb5560fe824854a44c983757) )
ROM_END

ROM_START( fnvision )
	ROM_REGION( 0x10000, M6502_TAG, 0 )
	ROM_LOAD( "funboot.rom",  0x0000, 0x0800, CRC(05602697) SHA1(c280b20c8074ba9abb4be4338b538361dfae517f) )
ROM_END

#define rom_wizzard rom_crvision
#define rom_crvisioj rom_crvision
#define rom_crvisio2 rom_crvision
#define rom_rameses rom_fnvision
#define rom_vz2000 rom_fnvision

ROM_START( manager )
	ROM_REGION( 0x10000, M6502_TAG, 0 )
	ROM_LOAD( "01", 0x0000, 0x2000, CRC(702f4cf5) SHA1(cd14ee74e787d24b76c166de484dae24206e219b) )
	ROM_LOAD( "23", 0x2000, 0x2000, CRC(46489d88) SHA1(467f5bcd62d0b4117c443e13373df8f3c45df7b2) )

	ROM_REGION( 0x1000, "disk", 0 )
	ROM_LOAD( "floppy interface cartridge", 0x0000, 0x1000, NO_DUMP )
ROM_END

/***************************************************************************
    SYSTEM DRIVERS
***************************************************************************/

/*    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT       INIT    COMPANY                   FULLNAME */
CONS( 1982, crvision,   0,          0,      pal,        crvision, driver_device,    0,      "Video Technology",         "CreatiVision", 0 )
CONS( 1982, fnvision,   crvision,   0,      pal,        crvision, driver_device,    0,      "Video Technology",         "FunVision", 0 )
CONS( 1982, crvisioj,   crvision,   0,      ntsc,       crvision, driver_device,    0,      "Cheryco",                  "CreatiVision (Japan)", 0 )
CONS( 1982, wizzard,    crvision,   0,      pal,        crvision, driver_device,    0,      "Dick Smith Electronics",   "Wizzard (Oceania)", 0 )
CONS( 1982, rameses,    crvision,   0,      pal,        crvision, driver_device,    0,      "Hanimex",                  "Rameses (Oceania)", 0 )
CONS( 1983, vz2000,     crvision,   0,      pal,        crvision, driver_device,    0,      "Dick Smith Electronics",   "VZ 2000 (Oceania)", 0 )
CONS( 1983, crvisio2,   crvision,   0,      pal,        crvision, driver_device,    0,      "Video Technology",         "CreatiVision MK-II (Europe)", 0 )
//COMP( 1983, lasr2001,   0,          0,      lasr2001,   lasr2001, driver_device,   0,      "Video Technology",         "Laser 2001", MACHINE_NOT_WORKING )
//COMP( 1983, vz2001,     lasr2001,   0,      lasr2001,   lasr2001, driver_device,   0,      "Dick Smith Electronics",   "VZ 2001 (Oceania)", MACHINE_NOT_WORKING )
COMP( 1983, manager,    0,          0,      lasr2001,   manager, driver_device, 0,      "Salora",                   "Manager (Finland)", 0 )
