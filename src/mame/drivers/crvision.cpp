// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Curt Coder
/*
    This 1980s computer was manufactured by VTech of Hong Kong.
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

    - RAM mapping
    - cassette (figure out correct input level) - it can load its own recordings if you try a few times;
    -- The "sonicinvader.wav" found on the net works perfectly with CRUN.
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


Left Keyboard           Right Keyboard
1    2 3 4 5 6          7 8 9 0 :  -
CTRL Q W E R T          Y U I O P  RETN
<-   A S D F G          H J K L dn ->
SHFT Z X C V B          N M . , /  SHFT

- laser2001, manager: cassette format is incompatible with crvision.
- manager: appears the joystick is 4-way only.
- crvision: if you get "ERROR 00" while loading a tape, you can continue to load the remainder of it with CLOAD or CRUN.
            The bad line(s) will be missing. (behaviour depends on version of Basic)

TODO:
- manager: paste can lose a character or 2 at the start of a line
- crvision: paste is very poor
- crvision: in natural key mode & in paste, shifted characters do not appear.
- crvision: there is a buzzing noise while carts are running.

*/

#include "emu.h"
#include "includes/crvision.h"
#include "softlist_dev.h"
#include "speaker.h"


/***************************************************************************
    MEMORY MAPS
***************************************************************************/

/*-------------------------------------------------
    ADDRESS_MAP( crvision_map )
-------------------------------------------------*/

void crvision_state::crvision_map(address_map &map)
{
	map(0x0000, 0x03ff).mirror(0x0c00).ram();
	map(0x1000, 0x1003).mirror(0x0ffc).rw(m_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x2000, 0x2001).mirror(0x0ffe).r(TMS9929_TAG, FUNC(tms9928a_device::read));
	map(0x3000, 0x3001).mirror(0x0ffe).w(TMS9929_TAG, FUNC(tms9928a_device::write));
	map(0x4000, 0x7fff).rom().share(BANK_ROM2);
	map(0x8000, 0xbfff).rom().share(BANK_ROM1);
//  map(0xc000, 0xe7ff).bankrw(3);
	map(0xe800, 0xe800).w(m_cent_data_out, FUNC(output_latch_device::write));
	map(0xe801, 0xe801).r("cent_status_in", FUNC(input_buffer_device::read));
	map(0xe801, 0xe801).w("cent_ctrl_out", FUNC(output_latch_device::write));
//  map(0xe802, 0xf7ff).bankrw(4);
	map(0xf800, 0xffff).rom().region(M6502_TAG, 0);
}

/*-------------------------------------------------
    ADDRESS_MAP( lasr2001_map )
-------------------------------------------------*/

void laser2001_state::lasr2001_map(address_map &map)
{
	map(0x0000, 0x03ff).mirror(0x0c00).ram();
	map(0x1000, 0x1003).mirror(0x0ffc).rw(m_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x2000, 0x2001).mirror(0x0ffe).r(TMS9929_TAG, FUNC(tms9928a_device::read));
	map(0x3000, 0x3001).mirror(0x0ffe).w(TMS9929_TAG, FUNC(tms9928a_device::write));
	map(0x4000, 0x7fff).ram().share(BANK_ROM2);
	map(0x8000, 0xbfff).ram().share(BANK_ROM1);
	map(0xc000, 0xffff).rom().region(M6502_TAG, 0);
}

/***************************************************************************
    INPUT PORTS
***************************************************************************/

/*-------------------------------------------------
    INPUT_CHANGED_MEMBER( trigger_nmi )
-------------------------------------------------*/

INPUT_CHANGED_MEMBER( crvision_state::trigger_nmi )
{
	m_maincpu->set_input_line(m6502_device::NMI_LINE, newval ? ASSERT_LINE : CLEAR_LINE);
}

/*-------------------------------------------------
    INPUT_PORTS( crvision )
    Each joystick has 8 direction pads. Further,
    by activating 2 adjacent pads at once, 16
    directions can be obtained. BASIC only handles
    the 8 pads. Direction codes per the manual:
    0 - no direction
    1 - down
    2 - down/right
    3 - right
    4 - up/right
    5 - up
    6 - up/left
    7 - left
    8 - down/left
    when using PRINT JOY(1) [or JOY(2)].
    As you can see, there are multiple choices as
    which input to choose. I've taken a guess; if
    it turns out to be wrong use another option.
-------------------------------------------------*/

static INPUT_PORTS_START( crvision )
	// Player 1 Joystick

	PORT_START("PA0.0")
	//PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_16WAY // 2
	//PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY // 3
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA0.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_16WAY PORT_CODE(KEYCODE_PGDN) // 2
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_16WAY // 1
	//PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_16WAY // 1
	//PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_16WAY // 1
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA0.2")
	//PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY // 3
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY // 3
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_16WAY PORT_CODE(KEYCODE_PGUP) // 4
	//PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY // 3
	PORT_BIT( 0xb3, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA0.3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_16WAY // 5
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_16WAY PORT_CODE(KEYCODE_HOME) // 6
	//PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_16WAY // 5
	//PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_16WAY // 5
	PORT_BIT( 0xe7, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA0.4")
	//PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_16WAY // 6
	//PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY // 7
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA0.5")
	//PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY // 7
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY // 7
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_16WAY PORT_CODE(KEYCODE_END) // 8
	//PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY // 7
	PORT_BIT( 0x9f, IP_ACTIVE_LOW, IPT_UNUSED )

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
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Button 1 / SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)

	// Player 2 Joystick

	PORT_START("PA2.0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA2.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_16WAY PORT_PLAYER(2) PORT_CODE(KEYCODE_3_PAD) // 2
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_16WAY PORT_PLAYER(2) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA2.2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_PLAYER(2) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_16WAY PORT_PLAYER(2) PORT_CODE(KEYCODE_9_PAD) // 4
	PORT_BIT( 0xb3, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA2.3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_16WAY PORT_PLAYER(2) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_16WAY PORT_PLAYER(2) PORT_CODE(KEYCODE_7_PAD) // 6
	PORT_BIT( 0xe7, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA2.4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA2.5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_PLAYER(2) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_16WAY PORT_PLAYER(2) PORT_CODE(KEYCODE_1_PAD) // 8
	PORT_BIT( 0x9f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA2.6")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PA2.7")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P2 Button 2 / \xE2\x86\x92") PORT_CODE(KEYCODE_TAB) PORT_CHAR(9) PORT_PLAYER(2)

	// Player 2 Keyboard

	PORT_START("PA3.0")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RET'N") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
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
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('-') PORT_CHAR('#')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')

	PORT_START("Y.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xC3\x84 \xC3\xA4") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(0x00C4) PORT_CHAR(0x00E4) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"') PORT_CHAR('@')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z')

	PORT_START("Y.2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED) // I
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xC3\x85 \xC3\xA5") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(0x00C5) PORT_CHAR(0x00E5) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("Y.3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED) // N
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED) // X
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xC3\x96 \xC3\xB6") PORT_CODE(KEYCODE_COLON) PORT_CHAR(0x00D6) PORT_CHAR(0x00F6) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED) // J
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l') PORT_CHAR(UCHAR_MAMEKEY(UP))
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
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED) // N
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
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_START("JOY.1")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("JOY.2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)

	PORT_START("JOY.3")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("NMI")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F10) PORT_CHANGED_MEMBER(DEVICE_SELF, crvision_state, trigger_nmi, 0)
INPUT_PORTS_END

/***************************************************************************
    DEVICE CONFIGURATION
***************************************************************************/

void crvision_state::pia_pa_w(uint8_t data)
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

uint8_t crvision_state::read_keyboard(u8 pa)
{
	uint8_t value = 0xff;

	// Get shift/ctrl keys
	u8 modifier = m_io_keypad[pa][7]->read() & 0x80;

	// Get normal keys/joystick
	for (u8 i = 0; i < 6; i++)
	{
		value = m_io_keypad[pa][i]->read() & 0x7f;

		if (value < 0x7f)
		{
			if (value == 0x7f - (1 << i))
				break;
			else
			{
				value -= (1 << i);
				break;
			}
		}
	}

	value |= modifier;

	//if (value < 0xff) printf("%01X%02X ",pa,value);
	return value;
}

uint8_t crvision_state::pia_pa_r()
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

	uint8_t data = 0x7f;

	if ((m_cassette)->input() > -0.1469) data |= 0x80;

	return data;
}

uint8_t crvision_state::pia_pb_r()
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

	uint8_t data = 0xff;

	for (u8 i = 0; i < 4; i++)
		if (BIT(m_keylatch, i))
			data &= read_keyboard(i);

	return data;
}

uint8_t laser2001_state::pia_pa_r()
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

	uint8_t data = 0xff;

	for (u8 i = 0; i < 8; i++)
		if (!BIT(m_keylatch, i))
			data &= m_inp_y[i]->read();

	return data;
}

void laser2001_state::pia_pa_w(uint8_t data)
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

uint8_t laser2001_state::pia_pb_r()
{
	uint8_t data = 0xff;

	for (u8 i = 0; i < 4; i++)
		if (~m_joylatch >> i & 1)
			data &= m_inp_joy[i]->read();

	return data;
}

void laser2001_state::pia_pb_w(uint8_t data)
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
	m_cent_data_out->write(data);
}

READ_LINE_MEMBER( laser2001_state::pia_ca1_r )
{
	return (m_cassette->input() > 0.16) ? 0 : 1;
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
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x4000, 0x7fff, read8sm_delegate(*m_cart, FUNC(crvision_cart_slot_device::read_rom40)));
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x8000, 0xbfff, read8sm_delegate(*m_cart, FUNC(crvision_cart_slot_device::read_rom80)));
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

static void crvision_cart(device_slot_interface &device)
{
	device.option_add_internal("crv_rom4k",  CRVISION_ROM_4K);
	device.option_add_internal("crv_rom6k",  CRVISION_ROM_6K);
	device.option_add_internal("crv_rom8k",  CRVISION_ROM_8K);
	device.option_add_internal("crv_rom10k", CRVISION_ROM_10K);
	device.option_add_internal("crv_rom12k", CRVISION_ROM_12K);
	device.option_add_internal("crv_rom16k", CRVISION_ROM_16K);
	device.option_add_internal("crv_rom18k", CRVISION_ROM_18K);
}

/*-------------------------------------------------
    creativision machine configuration
-------------------------------------------------*/

void crvision_state::creativision(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, XTAL(2'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &crvision_state::crvision_map);

	// devices
	PIA6821(config, m_pia, 0);
	m_pia->readpa_handler().set(FUNC(crvision_state::pia_pa_r));
	m_pia->readpb_handler().set(FUNC(crvision_state::pia_pb_r));
	m_pia->writepa_handler().set(FUNC(crvision_state::pia_pa_w));
	m_pia->writepb_handler().set(SN76489_TAG, FUNC(sn76496_base_device::write));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SN76489A(config, m_psg, XTAL(2'000'000));
	m_psg->ready_cb().set(m_pia, FUNC(pia6821_device::cb1_w));
	m_psg->add_route(ALL_OUTPUTS, "mono", 1.00);

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set("cent_status_in", FUNC(input_buffer_device::write_bit7));

	INPUT_BUFFER(config, "cent_status_in", 0);

	OUTPUT_LATCH(config, m_cent_data_out);
	m_centronics->set_output_latch(*m_cent_data_out);

	OUTPUT_LATCH(config, "cent_ctrl_out").bit_handler<4>().set(m_centronics, FUNC(centronics_device::write_strobe));

	// cartridge
	CRVISION_CART_SLOT(config, m_cart, crvision_cart, nullptr);

	// internal ram
	RAM(config, m_ram);
	m_ram->set_default_size("1K"); // main RAM
	m_ram->set_extra_options("15K"); // 16K expansion (lower 14K available only, upper 2K shared with BIOS ROM)

	// software lists
	SOFTWARE_LIST(config, "cart_list").set_original("crvision");
}

/*-------------------------------------------------
    machine_config( ntsc )
-------------------------------------------------*/

void crvision_state::ntsc(machine_config &config)
{
	creativision(config);
	// video hardware
	tms9918_device &vdp(TMS9918(config, TMS9929_TAG, XTAL(10'738'635)));
	vdp.set_screen("screen");
	vdp.set_vram_size(0x4000);
	vdp.int_callback().set_inputline(M6502_TAG, m6502_device::IRQ_LINE);
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);
}

/*-------------------------------------------------
    machine_config( pal )
-------------------------------------------------*/

void crvision_pal_state::pal(machine_config &config)
{
	creativision(config);
	// video hardware
	tms9929_device &vdp(TMS9929(config, TMS9929_TAG, XTAL(10'738'635)));
	vdp.set_screen("screen");
	vdp.set_vram_size(0x4000);
	vdp.int_callback().set_inputline(M6502_TAG, m6502_device::IRQ_LINE);
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);
}

/*-------------------------------------------------
    machine_config( lasr2001 )
-------------------------------------------------*/

void laser2001_state::lasr2001(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, XTAL(17'734'470)/9);
	m_maincpu->set_addrmap(AS_PROGRAM, &laser2001_state::lasr2001_map);

	// devices
	PIA6821(config, m_pia, 0);
	m_pia->readpa_handler().set(FUNC(laser2001_state::pia_pa_r));
	m_pia->readpb_handler().set(FUNC(laser2001_state::pia_pb_r));
	m_pia->readca1_handler().set(FUNC(laser2001_state::pia_ca1_r));
	m_pia->writepa_handler().set(FUNC(laser2001_state::pia_pa_w));
	m_pia->writepb_handler().set(FUNC(laser2001_state::pia_pb_w));
	m_pia->ca2_handler().set(FUNC(laser2001_state::pia_ca2_w));
	m_pia->cb2_handler().set(FUNC(laser2001_state::pia_cb2_w));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SN76489A(config, m_psg, XTAL(17'734'470)/9);
	m_psg->ready_cb().set(FUNC(laser2001_state::write_psg_ready));
	m_psg->add_route(ALL_OUTPUTS, "mono", 1.00);

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(laser2001_state::write_centronics_busy));

	OUTPUT_LATCH(config, m_cent_data_out);
	m_centronics->set_output_latch(*m_cent_data_out);

	// video hardware
	tms9929a_device &vdp(TMS9929A(config, TMS9929_TAG, XTAL(10'738'635)));
	vdp.set_screen("screen");
	vdp.set_vram_size(0x4000);
	vdp.int_callback().set_inputline(M6502_TAG, m6502_device::IRQ_LINE);
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	// cartridge
	CRVISION_CART_SLOT(config, m_cart, crvision_cart, nullptr);

	// internal ram
	RAM(config, m_ram);
	m_ram->set_default_size("16K");
	m_ram->set_extra_options("32K");

	// software list
	SOFTWARE_LIST(config, "cart_list").set_original("crvision");
	SOFTWARE_LIST(config, "cart_list2").set_original("laser2001_cart");
	SOFTWARE_LIST(config, "flop_list").set_original("laser2001_flop");
}

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

ROM_START( lasr2001 )
	ROM_REGION( 0x10000, M6502_TAG, 0 )
	ROM_LOAD( "laser2001.rom", 0x0000, 0x4000, CRC(4dc35c39) SHA1(c12e098c14ac0724869053df2b63277a3e413802) )
ROM_END

ROM_START( manager )
	ROM_REGION( 0x10000, M6502_TAG, 0 )
	ROM_LOAD( "01", 0x0000, 0x2000, CRC(702f4cf5) SHA1(cd14ee74e787d24b76c166de484dae24206e219b) )
	ROM_LOAD( "23", 0x2000, 0x2000, CRC(46489d88) SHA1(467f5bcd62d0b4117c443e13373df8f3c45df7b2) )
ROM_END

/***************************************************************************
    SYSTEM DRIVERS
***************************************************************************/

//    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT     CLASS               INIT        COMPANY                   FULLNAME                       FLAGS
CONS( 1982, crvision, 0,        0,      pal,      crvision, crvision_pal_state, empty_init, "Video Technology",       "CreatiVision",                MACHINE_SUPPORTS_SAVE )
CONS( 1982, fnvision, crvision, 0,      pal,      crvision, crvision_pal_state, empty_init, "Video Technology",       "FunVision",                   MACHINE_SUPPORTS_SAVE )
CONS( 1982, crvisioj, crvision, 0,      ntsc,     crvision, crvision_state,     empty_init, "Cheryco",                "CreatiVision (Japan)",        MACHINE_SUPPORTS_SAVE )
CONS( 1982, wizzard,  crvision, 0,      pal,      crvision, crvision_pal_state, empty_init, "Dick Smith Electronics", "Wizzard (Oceania)",           MACHINE_SUPPORTS_SAVE )
CONS( 1982, rameses,  crvision, 0,      pal,      crvision, crvision_pal_state, empty_init, "Hanimex",                "Rameses HVC6502 (Oceania)",   MACHINE_SUPPORTS_SAVE )
CONS( 1983, vz2000,   crvision, 0,      pal,      crvision, crvision_pal_state, empty_init, "Dick Smith Electronics", "VZ 2000 (Oceania)",           MACHINE_SUPPORTS_SAVE )
CONS( 1983, crvisio2, crvision, 0,      pal,      crvision, crvision_pal_state, empty_init, "Video Technology",       "CreatiVision MK-II (Europe)", MACHINE_SUPPORTS_SAVE )
COMP( 1983, lasr2001, 0,        0,      lasr2001, manager,  laser2001_state,    empty_init, "Video Technology",       "Laser 2001",                  MACHINE_SUPPORTS_SAVE )
//COMP( 1983, vz2001,   lasr2001, 0,      lasr2001, lasr2001, laser2001_state,    empty_init, "Dick Smith Electronics", "VZ 2001 (Oceania)",           MACHINE_SUPPORTS_SAVE )
COMP( 1983, manager,  0,        0,      lasr2001, manager, laser2001_state,     empty_init, "Salora",                 "Manager (Finland)",           MACHINE_SUPPORTS_SAVE )
