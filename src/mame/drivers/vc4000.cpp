// license:GPL-2.0+
// copyright-holders:Peter Trauner, Robbbert
// thanks-to:Manfred Schneider
/******************************************************************************
 Peter Trauner May 2001

 Paul Robson's Emulator at www.classicgaming.com/studio2 made it possible
******************************************************************************/

/*****************************************************************************
Additional Notes by Manfred Schneider
Memory Map
Memory mapping is done in two steps. The PVI(2636) provides the chip select signals
according to signals provided by the CPU address lines.
The PVI has 12 address line (A0-A11) which give him control over 4K. A11 of the PVI is not
connected to A11 of the CPU, but connected to the cartridge slot. On the cartridge it is
connected to A12 of the CPU which extends the addressable Range to 8K. This is also the
maximum usable space, because A13 and A14 of the CPU are not used.
With the above in mind address range will look like this:
$0000 - $15FF  ROM, RAM
$1600 - $167F  unused
$1680 - $16FF  used for I/O Control on main PCB
$1700 - $17FF PVI internal Registers and RAM
$1800 - $1DFF ROM, RAM
$1E00 - $1E7F unused
$1E80 - $1EFF mirror of $1680 - $167F
$1F00 - $1FFF mirror of $1700 - $17FF
$2000 - $3FFF mirror of $0000 - $1FFF
$4000 - $5FFF mirror of $0000 - $1FFF
$6000 - $7FFF mirror of $0000 - $1FFF
On all cartridges for the Interton A11 from PVI is connected to A12 of the CPU.
There are four different types of Cartridges with the following memory mapping.
Type 1: 2K Rom or EPROM mapped from $0000 - $07FF
Type 2: 4K Rom or EPROM mapped from $0000 - $0FFF
Type 3: 4K Rom + 1K Ram
    Rom is mapped from $0000 - $0FFF
    Ram is mapped from $1000 - $13FF and mirrored from $1800 - $1BFF
Type 4: 6K Rom + 1K Ram
    Rom is mapped from $0000 - $15FF (only 5,5K ROM visible to the CPU)
    Ram is mapped from $1800 - $1BFF

One other type is known for Radofin (rom compatible to VC4000, but not the Cartridge connector).
It consisted of a 2K ROM and 2K RAM which are most likely mapped as follows (needs to be confirmed):
2K Rom mapped from $0000 - $07FF
2K Ram mapped from $0800 - $0FFF
The Cartridge is called Hobby Module and the Rom is probably the same as used in
elektor TV Game Computer which is a kind of developer machine for the VC4000.

******************************************************************************

Elektor TV Games Computer
This is much the same as the vc4000, however it has its own ROM (with inbuilt
monitor program similar to the Signetics Instructor 50), and 2K of ram. No cart
slot, no joystick, but has a cassette interface.

ToDo:
- Most quickloads don't work too well
- Might need to rework keyboard, again

When booted you get the familiar 00 00 pattern. Pressing Q gives a display of
IIII. Now, you enter a command.

Key   Command    Purpose
------------------------
Q     Start      Boot up system
L     RCAS       Load a tape
S     WCAS       Save a tape
W     BP1/2      Set a breakpoint
R     REG        View/Set registers
X     PC         Go
+pad  +          Enter data and do next thing
-pad  -          Decrement
-     MEM        Specify an address
0-9   0-9        Hex digits
A-F   A-F        Hex digits

Keyboard layout when using the Monitor on real hardware (n/a = key not assigned)

n/a    RCAS  WCAS  C    D  E  F
Start  BP1/2 REG   8    9  A  B
n/a    PC    MEM   4    5  6  7
Reset  -     +     0    1  2  3

This wouldn't fit too well on our keyboard with any chance of remembering
it, so I've hooked it much the same as the Instructor.

The Select key (Z) and the joystick don't actually exist, but I've left them
in the keyboard matrix for now.


Quickloads
----------
You can load pgm and tvc files with the quickload facility. The quickloads
are meant for the ElektorTVGC, however with a bit a trickery they can be made
to work on the vc4000 as well. Procedure:

- Get a copy of the Elektor bios and rename it to ELEKTOR.BIN then save it
  with the rest of your vc4000 carts.

- Start vc4000, and load ELEKTOR.BIN into the cartslot. Now your vc4000
  thinks it is an Elektor.

- Load a quickload file. Some of them will work, and in some cases, better
  than on the Elektor system.


Pasting
-------
This system uses the standard trainer paste codes:
        0-F : as is
        +   : ^
        -   : V
        MEM : -
        MON : Q

Here's a sample from the manual, page 34/35 (down-arrow to escape)
Q-0900^762005CA06CA0D4A00CD7F00FA780C1E88441099791F0000040005CA06CACD4A00FA7B
04FFCC0AC8CC0AC90409CC0AC60402CC0AC01F0900
-0A00^F15155757FFFFFC3A52480FF4FFF-0AC0^C018P0900^

Another sample, from page 94 (Q to escape)
Q-0900^76203F0161063005080E492DCD4890597877103F020E75105A0A0C1E89F4101879
1F003877103F02CF75101B5A
17A2A2A2A2A2A217
17171000000D1717
0A171100BC17000F
17170D000E051717
14150A0CBC120C0E
0A171112BCBC110EP0900^


******************************************************************************/

#include "emu.h"
#include "includes/vc4000.h"
#include "softlist_dev.h"
#include "speaker.h"


uint8_t vc4000_state::vc4000_key_r(offs_t offset)
{
	uint8_t data=0;
	switch(offset & 0x0f)
	{
	case 0x08:
		data = m_keypad1_1->read();
		break;
	case 0x09:
		data = m_keypad1_2->read();
		break;
	case 0x0a:
		data = m_keypad1_3->read();
		break;
	case 0x0b:
		data = m_panel->read();
		break;
	case 0x0c:
		data = m_keypad2_1->read();
		break;
	case 0x0d:
		data = m_keypad2_2->read();
		break;
	case 0x0e:
		data = m_keypad2_3->read();
		break;
	}
	return data;
}

void vc4000_state::vc4000_sound_ctl(offs_t offset, uint8_t data)
{
	logerror("Write to sound control register offset= %d value= %d\n", offset, data);
}

// Write cassette - Address 0x1DFF
void vc4000_state::elektor_cass_w(uint8_t data)
{
	m_cassette->output(BIT(data, 7) ? -1.0 : +1.0);
}

// Read cassette - Address 0x1DBF
uint8_t vc4000_state::elektor_cass_r()
{
	return (m_cassette->input() > 0.03) ? 0xff : 0x7f;
}

void vc4000_state::vc4000_mem(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x1fff);
	map(0x0000, 0x07ff).rom();
	map(0x1680, 0x16ff).rw(FUNC(vc4000_state::vc4000_key_r), FUNC(vc4000_state::vc4000_sound_ctl)).mirror(0x0800);
	map(0x1700, 0x17ff).rw(FUNC(vc4000_state::vc4000_video_r), FUNC(vc4000_state::vc4000_video_w)).mirror(0x0800);
}

void vc4000_state::elektor_mem(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x1fff);
	map(0x0000, 0x07ff).rom();
	map(0x0800, 0x15ff).ram();
	map(0x1980, 0x19ff).mirror(0x400).rw(FUNC(vc4000_state::elektor_cass_r), FUNC(vc4000_state::elektor_cass_w));
	map(0x1680, 0x168f).mirror(0x800).rw(FUNC(vc4000_state::vc4000_key_r), FUNC(vc4000_state::vc4000_sound_ctl));
	map(0x1700, 0x17ff).mirror(0x800).rw(FUNC(vc4000_state::vc4000_video_r), FUNC(vc4000_state::vc4000_video_w));
}

static INPUT_PORTS_START( vc4000 )
	PORT_START("PANEL")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START ) PORT_NAME("Start")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_NAME("Game Select")

	PORT_START("KEYPAD1_1")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad 7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad Enter") PORT_CODE(KEYCODE_PLUS_PAD)

	PORT_START("KEYPAD1_2")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad 2/Button") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad 8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad 0") PORT_CODE(KEYCODE_0_PAD)

	PORT_START("KEYPAD1_3")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad 9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P1 Keypad Clear") PORT_CODE(KEYCODE_MINUS_PAD)

	PORT_START("KEYPAD2_1")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad 1") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad 4") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad 7") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad Enter") PORT_CODE(KEYCODE_V)

	PORT_START("KEYPAD2_2")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad 2/Button") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad 5") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad 8") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad 0") PORT_CODE(KEYCODE_F)

	PORT_START("KEYPAD2_3")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad 3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad 6") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad 9") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("P2 Keypad Clear") PORT_CODE(KEYCODE_R)
#ifndef ANALOG_HACK
	// auto centering too slow, so only using 5 bits, and scaling at videoside
	PORT_START("JOY1_X")
PORT_BIT(0xff,0x70,IPT_AD_STICK_X) PORT_SENSITIVITY(70) PORT_KEYDELTA(5) PORT_CENTERDELTA(0) PORT_MINMAX(20,225) PORT_CODE_DEC(KEYCODE_LEFT) PORT_CODE_INC(KEYCODE_RIGHT) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH) PORT_PLAYER(1)
	PORT_START("JOY1_Y")
PORT_BIT(0xff,0x70,IPT_AD_STICK_Y) PORT_SENSITIVITY(70) PORT_KEYDELTA(5) PORT_CENTERDELTA(0) PORT_MINMAX(20,225) PORT_CODE_DEC(KEYCODE_UP) PORT_CODE_INC(KEYCODE_DOWN) PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH) PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH) PORT_PLAYER(1)
	PORT_START("JOY2_X")
PORT_BIT(0xff,0x70,IPT_AD_STICK_X) PORT_SENSITIVITY(70) PORT_KEYDELTA(5) PORT_CENTERDELTA(0) PORT_MINMAX(20,225) PORT_CODE_DEC(KEYCODE_DEL) PORT_CODE_INC(KEYCODE_PGDN) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH) PORT_PLAYER(2)
	PORT_START("JOY2_Y")
PORT_BIT(0xff,0x70,IPT_AD_STICK_Y) PORT_SENSITIVITY(70) PORT_KEYDELTA(5) PORT_CENTERDELTA(0) PORT_MINMAX(20,225) PORT_CODE_DEC(KEYCODE_HOME) PORT_CODE_INC(KEYCODE_END) PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH) PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH) PORT_PLAYER(2)
#else
	PORT_START("JOYS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CODE(KEYCODE_DEL) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CODE(KEYCODE_PGDN) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CODE(KEYCODE_END) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CODE(KEYCODE_HOME) PORT_PLAYER(2)

	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x00, "Treat Joystick as...")
	PORT_CONFSETTING(    0x00, "Buttons")
	PORT_CONFSETTING(    0x01, "Paddle")
#endif
INPUT_PORTS_END

INPUT_PORTS_START( elektor )
	PORT_START("PANEL")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("MON") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Game Select") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')

	PORT_START("KEYPAD1_1")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RCAS") PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("BP1/2") PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PC") PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("-") PORT_CODE(KEYCODE_DOWN) PORT_CHAR('V')

	PORT_START("KEYPAD1_2")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("WCAS") PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("REG") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("MEM") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("+") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')

	PORT_START("KEYPAD1_3")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')

	PORT_START("KEYPAD2_1")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')

	PORT_START("KEYPAD2_2")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')

	PORT_START("KEYPAD2_3")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
#ifndef ANALOG_HACK
	// auto centering too slow, so only using 5 bits, and scaling at videoside
	PORT_START("JOY1_X")
PORT_BIT(0xff,0x70,IPT_AD_STICK_X) PORT_SENSITIVITY(70) PORT_KEYDELTA(5) PORT_CENTERDELTA(0) PORT_MINMAX(20,225) PORT_CODE_DEC(KEYCODE_LEFT) PORT_CODE_INC(KEYCODE_RIGHT) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH) PORT_PLAYER(1)
	PORT_START("JOY1_Y")
PORT_BIT(0xff,0x70,IPT_AD_STICK_Y) PORT_SENSITIVITY(70) PORT_KEYDELTA(5) PORT_CENTERDELTA(0) PORT_MINMAX(20,225) PORT_CODE_DEC(KEYCODE_UP) PORT_CODE_INC(KEYCODE_DOWN) PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH) PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH) PORT_PLAYER(1)
	PORT_START("JOY2_X")
PORT_BIT(0xff,0x70,IPT_AD_STICK_X) PORT_SENSITIVITY(70) PORT_KEYDELTA(5) PORT_CENTERDELTA(0) PORT_MINMAX(20,225) PORT_CODE_DEC(KEYCODE_DEL) PORT_CODE_INC(KEYCODE_PGDN) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH) PORT_PLAYER(2)
	PORT_START("JOY2_Y")
PORT_BIT(0xff,0x70,IPT_AD_STICK_Y) PORT_SENSITIVITY(70) PORT_KEYDELTA(5) PORT_CENTERDELTA(0) PORT_MINMAX(20,225) PORT_CODE_DEC(KEYCODE_HOME) PORT_CODE_INC(KEYCODE_END) PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH) PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH) PORT_PLAYER(2)
#else
	PORT_START("JOYS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_CODE(KEYCODE_DEL) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_CODE(KEYCODE_PGDN) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_CODE(KEYCODE_END) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_CODE(KEYCODE_HOME) PORT_PLAYER(2)

	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x00, "Treat Joystick as...")
	PORT_CONFSETTING(    0x00, "Buttons")
	PORT_CONFSETTING(    0x01, "Paddle")
#endif
INPUT_PORTS_END

static constexpr rgb_t vc4000_pens[] =
{
	// background colors
	rgb_t(0, 0, 0), // black
	rgb_t(0, 0, 175), // blue
	rgb_t(0, 175, 0), // green
	rgb_t(0, 255, 255), // cyan
	rgb_t(255, 0, 0), // red
	rgb_t(255, 0, 255), // magenta
	rgb_t(200, 200, 0), // yellow
	rgb_t(200, 200, 200), // white
	/* sprite colors
	The control line simply inverts the RGB lines all at once.
	We can do that in the code with ^7 */
};

void vc4000_state::vc4000_palette(palette_device &palette) const
{
	palette.set_pen_colors(0, vc4000_pens);
}


void vc4000_state::machine_start()
{
	if (m_cart->exists())
	{
		// extra handler
		switch (m_cart->get_type())
		{
		case VC4000_STD:
			m_maincpu->space(AS_PROGRAM).install_read_handler(0x0000, 0x07ff, read8sm_delegate(*m_cart, FUNC(vc4000_cart_slot_device::read_rom)));
			break;
		case VC4000_ROM4K:
			m_maincpu->space(AS_PROGRAM).install_read_handler(0x0000, 0x0fff, read8sm_delegate(*m_cart, FUNC(vc4000_cart_slot_device::read_rom)));
			break;
		case VC4000_RAM1K:
			m_maincpu->space(AS_PROGRAM).install_read_handler(0x0000, 0x0fff, read8sm_delegate(*m_cart, FUNC(vc4000_cart_slot_device::read_rom)));
			m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x1000, 0x15ff, read8sm_delegate(*m_cart, FUNC(vc4000_cart_slot_device::read_ram)), write8sm_delegate(*m_cart, FUNC(vc4000_cart_slot_device::write_ram)));
			break;
		case VC4000_CHESS2:
			m_maincpu->space(AS_PROGRAM).install_read_handler(0x0000, 0x15ff, read8sm_delegate(*m_cart, FUNC(vc4000_cart_slot_device::read_rom)));
			m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x1800, 0x1bff, read8sm_delegate(*m_cart, FUNC(vc4000_cart_slot_device::read_ram)), write8sm_delegate(*m_cart, FUNC(vc4000_cart_slot_device::write_ram)));
			break;
		// undumped Radofin Hobby Module
//      case VC4000_HOBBY:
//          m_maincpu->space(AS_PROGRAM).install_read_handler(0x0000, 0x07ff, read8sm_delegate(*m_cart, FUNC(vc4000_cart_slot_device::read_rom)));
//          m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x0800, 0x0fff, read8sm_delegate(*m_cart, FUNC(vc4000_cart_slot_device::read_ram)), write8sm_delegate(*m_cart, FUNC(vc4000_cart_slot_device::write_ram)));
//          break;
		}

		m_cart->save_ram();
	}
}


QUICKLOAD_LOAD_MEMBER(vc4000_state::quickload_cb)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	int i;
	int exec_addr;
	int quick_length;
	std::vector<uint8_t> quick_data;
	int read_;
	image_init_result result = image_init_result::FAIL;

	quick_length = image.length();
	quick_data.resize(quick_length);
	read_ = image.fread( &quick_data[0], quick_length);
	if (read_ != quick_length)
	{
		image.seterror(image_error::INVALIDIMAGE, "Cannot read the file");
		image.message(" Cannot read the file");
	}
	else
	{
		if (image.is_filetype("tvc"))
		{
			if (quick_data[0] != 2)
			{
				image.seterror(image_error::INVALIDIMAGE, "Invalid header");
				image.message(" Invalid header");
			}
			else
			{
				int quick_addr = quick_data[1] * 256 + quick_data[2];
				exec_addr = quick_data[3] * 256 + quick_data[4];

				if (quick_length < 0x5)
				{
					image.seterror(image_error::INVALIDIMAGE, "File too short");
					image.message(" File too short");
				}
				else
					if ((quick_length + quick_addr - 5) > 0x1600)
					{
						image.seterror(image_error::INVALIDIMAGE, "File too long");
						image.message(" File too long");
					}
					else
					{
						space.write_byte(0x08be, quick_data[3]);
						space.write_byte(0x08bf, quick_data[4]);

						for (i = 5; i < quick_length; i++)
							space.write_byte(i - 5 + quick_addr, quick_data[i]);

						/* display a message about the loaded quickload */
						image.message(" Quickload: size=%04X : start=%04X : end=%04X : exec=%04X",quick_length-5,quick_addr,quick_addr+quick_length-5,exec_addr);

						// Start the quickload
						m_maincpu->set_state_int(S2650_PC, exec_addr);
						result = image_init_result::PASS;
					}
			}
		}
		else
			if (image.is_filetype("pgm"))
			{
				if (quick_data[0] != 0)
				{
					image.seterror(image_error::INVALIDIMAGE, "Invalid header");
					image.message(" Invalid header");
				}
				else
				{
					exec_addr = quick_data[1] * 256 + quick_data[2];

					if (exec_addr >= quick_length)
					{
						image.seterror(image_error::INVALIDIMAGE, "Exec address beyond end of file");
						image.message(" Exec address beyond end of file");
					}
					else
						if (quick_length < 0x904)
						{
							image.seterror(image_error::INVALIDIMAGE, "File too short");
							image.message(" File too short");
						}
						else
							if (quick_length > 0x2000)
							{
								image.seterror(image_error::INVALIDIMAGE, "File too long");
								image.message(" File too long");
							}
							else
							{
								space.write_byte(0x08be, quick_data[1]);
								space.write_byte(0x08bf, quick_data[2]);

								// load to 08C0-15FF (standard ram + extra)
								int read_ = 0x1600;
								if (quick_length < 0x1600)
									read_ = quick_length;
								for (i = 0x8c0; i < read_; i++)
									space.write_byte(i, quick_data[i]);

								// load to 1F50-1FAF (PVI regs)
								read_ = 0x1FB0;
								if (quick_length < 0x1FB0)
									read_ = quick_length;
								if (quick_length > 0x1FC0)
									for (i = 0x1F50; i < read_; i++)
										vc4000_video_w(i-0x1f00, quick_data[i]);

								/* display a message about the loaded quickload */
								image.message(" Quickload: size=%04X : exec=%04X",quick_length,exec_addr);

								// Start the quickload
								m_maincpu->set_state_int(S2650_PC, exec_addr);
								result = image_init_result::PASS;
							}
				}
			}
	}
	return result;
}

static void vc4000_cart(device_slot_interface &device)
{
	device.option_add_internal("std",      VC4000_ROM_STD);
	device.option_add_internal("rom4k",    VC4000_ROM_ROM4K);
	device.option_add_internal("ram1k",    VC4000_ROM_RAM1K);
	device.option_add_internal("chess2",   VC4000_ROM_CHESS2);
}


void vc4000_state::vc4000(machine_config &config)
{
	/* basic machine hardware */
//  S2650(config, m_maincpu, 865000);        /* 3550000/4, 3580000/3, 4430000/3 */
	S2650(config, m_maincpu, 3546875/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &vc4000_state::vc4000_mem);
	m_maincpu->sense_handler().set(FUNC(vc4000_state::vc4000_vsync_r));
	m_maincpu->set_periodic_int(FUNC(vc4000_state::vc4000_video_line), attotime::from_hz(312*53));  // GOLF needs this exact value
	m_maincpu->intack_handler().set([]() { return 0x03; });

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(50);
	m_screen->set_size(226, 312);
	m_screen->set_visarea(8, 184, 0, 269);
	m_screen->set_screen_update(FUNC(vc4000_state::screen_update_vc4000));
	m_screen->set_palette("palette");

	PALETTE(config, "palette", FUNC(vc4000_state::vc4000_palette), 8);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	VC4000_SND(config, m_custom, 0).add_route(ALL_OUTPUTS, "mono", 0.50);

	/* quickload */
	QUICKLOAD(config, "quickload", "pgm,tvc").set_load_callback(FUNC(vc4000_state::quickload_cb));

	/* cartridge */
	VC4000_CART_SLOT(config, "cartslot", vc4000_cart, nullptr);

	/* software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("vc4000");
}

void vc4000_state::cx3000tc(machine_config &config)
{
	vc4000(config);
	SOFTWARE_LIST(config.replace(), "cart_list").set_original("cx3000tc");
}

void vc4000_state::mpu1000(machine_config &config)
{
	vc4000(config);
	SOFTWARE_LIST(config.replace(), "cart_list").set_original("mpu1000");
}

void vc4000_state::database(machine_config &config)
{
	vc4000(config);
	SOFTWARE_LIST(config.replace(), "cart_list").set_original("database");
}

void vc4000_state::rwtrntcs(machine_config &config)
{
	vc4000(config);
	SOFTWARE_LIST(config.replace(), "cart_list").set_original("rwtrntcs");
}

void vc4000_state::h21(machine_config &config)
{
	vc4000(config);
	H21_CART_SLOT(config.replace(), "cartslot", vc4000_cart, nullptr);
	SOFTWARE_LIST(config.replace(), "cart_list").set_original("h21");
}

void vc4000_state::elektor(machine_config &config)
{
	vc4000(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &vc4000_state::elektor_mem);
	CASSETTE(config, m_cassette);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
}


ROM_START( vc4000 )
	ROM_REGION( 0x2000,"maincpu", ROMREGION_ERASEFF )
ROM_END

ROM_START( spc4000 )
	ROM_REGION( 0x2000,"maincpu", ROMREGION_ERASEFF )
ROM_END

ROM_START( cx3000tc )
	ROM_REGION( 0x2000,"maincpu", ROMREGION_ERASEFF )
ROM_END

ROM_START( tvc4000 )
	ROM_REGION( 0x2000,"maincpu", ROMREGION_ERASEFF )
ROM_END

ROM_START( 1292apvs )
	ROM_REGION( 0x2000,"maincpu", ROMREGION_ERASEFF )
ROM_END

ROM_START( 1392apvs )
	ROM_REGION( 0x2000,"maincpu", ROMREGION_ERASEFF )
ROM_END

ROM_START( mpu1000 )
	ROM_REGION( 0x2000,"maincpu", ROMREGION_ERASEFF )
ROM_END

ROM_START( mpu2000 )
	ROM_REGION( 0x2000,"maincpu", ROMREGION_ERASEFF )
ROM_END

ROM_START( pp1292 )
	ROM_REGION( 0x2000,"maincpu", ROMREGION_ERASEFF )
ROM_END

ROM_START( pp1392 )
	ROM_REGION( 0x2000,"maincpu", ROMREGION_ERASEFF )
ROM_END

ROM_START( f1392 )
	ROM_REGION( 0x2000,"maincpu", ROMREGION_ERASEFF )
ROM_END

ROM_START( fforce2 )
	ROM_REGION( 0x2000,"maincpu", ROMREGION_ERASEFF )
ROM_END

ROM_START( hmg1292 )
	ROM_REGION( 0x2000,"maincpu", ROMREGION_ERASEFF )
ROM_END

ROM_START( hmg1392 )
	ROM_REGION( 0x2000,"maincpu", ROMREGION_ERASEFF )
ROM_END

ROM_START( lnsy1392 )
	ROM_REGION( 0x2000,"maincpu", ROMREGION_ERASEFF )
ROM_END

ROM_START( vc6000 )
	ROM_REGION( 0x2000,"maincpu", ROMREGION_ERASEFF )
ROM_END

ROM_START( database )
	ROM_REGION( 0x2000,"maincpu", ROMREGION_ERASEFF )
ROM_END

ROM_START( vmdtbase )
	ROM_REGION( 0x2000,"maincpu", ROMREGION_ERASEFF )
ROM_END

ROM_START( rwtrntcs )
	ROM_REGION( 0x2000,"maincpu", ROMREGION_ERASEFF )
ROM_END

ROM_START( telngtcs )
	ROM_REGION( 0x2000,"maincpu", ROMREGION_ERASEFF )
ROM_END

ROM_START( krvnjvtv )
	ROM_REGION( 0x2000,"maincpu", ROMREGION_ERASEFF )
ROM_END

ROM_START( oc2000 )
	ROM_REGION( 0x2000,"maincpu", ROMREGION_ERASEFF )
ROM_END

ROM_START( mpt05 )
	ROM_REGION( 0x2000,"maincpu", ROMREGION_ERASEFF )
ROM_END

ROM_START( h21 )
	ROM_REGION( 0x2000,"maincpu", ROMREGION_ERASEFF )
ROM_END

ROM_START( elektor )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "elektor.rom", 0x0000, 0x0800, CRC(e6ef1ee1) SHA1(6823b5a22582344016415f2a37f9f3a2dc75d2a7))
ROM_END



//    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT    CLASS         INIT  COMPANY        FULLNAME                                      FLAGS
CONS( 1978, vc4000,   0,        0,      vc4000,   vc4000,  vc4000_state, empty_init, "Interton",    "Interton Electronic VC 4000",                MACHINE_IMPERFECT_GRAPHICS) // Germany, Austria, UK, Australia
CONS( 1979, spc4000,  vc4000,   0,      vc4000,   vc4000,  vc4000_state, empty_init, "Grundig",     "Super Play Computer 4000",                   MACHINE_IMPERFECT_GRAPHICS) // Germany, Austria
CONS( 1979, cx3000tc, vc4000,   0,      cx3000tc, vc4000,  vc4000_state, empty_init, "Palson",      "CX 3000 Tele Computer",                      MACHINE_IMPERFECT_GRAPHICS) // Spain
CONS( 1979, tvc4000,  vc4000,   0,      vc4000,   vc4000,  vc4000_state, empty_init, "Koerting",    "TVC-4000",                                   MACHINE_IMPERFECT_GRAPHICS) // Argentina
CONS( 1976, 1292apvs, 0,        vc4000, vc4000,   vc4000,  vc4000_state, empty_init, "Radofin",     "1292 Advanced Programmable Video System",    MACHINE_IMPERFECT_GRAPHICS) // Europe
CONS( 1976, 1392apvs, 1292apvs, 0,      vc4000,   vc4000,  vc4000_state, empty_init, "Radofin",     "1392 Advanced Programmable Video System",    MACHINE_IMPERFECT_GRAPHICS) // Europe
CONS( 1979, mpu1000,  1292apvs, 0,      mpu1000,  vc4000,  vc4000_state, empty_init, "Acetronic",   "MPU-1000",                                   MACHINE_IMPERFECT_GRAPHICS) // Europe
CONS( 1979, mpu2000,  1292apvs, 0,      mpu1000,  vc4000,  vc4000_state, empty_init, "Acetronic",   "MPU-2000",                                   MACHINE_IMPERFECT_GRAPHICS) // Europe
CONS( 1978, pp1292,   1292apvs, 0,      vc4000,   vc4000,  vc4000_state, empty_init, "Audio Sonic", "PP-1292 Advanced Programmable Video System", MACHINE_IMPERFECT_GRAPHICS) // Europe
CONS( 1978, pp1392,   1292apvs, 0,      vc4000,   vc4000,  vc4000_state, empty_init, "Audio Sonic", "PP-1392 Advanced Programmable Video System", MACHINE_IMPERFECT_GRAPHICS) // Europe
CONS( 1979, f1392,    1292apvs, 0,      vc4000,   vc4000,  vc4000_state, empty_init, "Fountain",    "Fountain 1392",                              MACHINE_IMPERFECT_GRAPHICS) // New Zealand
CONS( 1979, fforce2,  1292apvs, 0,      vc4000,   vc4000,  vc4000_state, empty_init, "Fountain",    "Fountain Force 2",                           MACHINE_IMPERFECT_GRAPHICS) // New Zealand, Australia
CONS( 1979, hmg1292,  1292apvs, 0,      vc4000,   vc4000,  vc4000_state, empty_init, "Hanimex",     "HMG 1292",                                   MACHINE_IMPERFECT_GRAPHICS) // Europe
CONS( 1979, hmg1392,  1292apvs, 0,      vc4000,   vc4000,  vc4000_state, empty_init, "Hanimex",     "HMG 1392",                                   MACHINE_IMPERFECT_GRAPHICS) // Europe
CONS( 1979, lnsy1392, 1292apvs, 0,      vc4000,   vc4000,  vc4000_state, empty_init, "Lansay",      "Lansay 1392",                                MACHINE_IMPERFECT_GRAPHICS) // Europe
CONS( 1979, vc6000,   1292apvs, 0,      vc4000,   vc4000,  vc4000_state, empty_init, "Prinztronic", "VC 6000",                                    MACHINE_IMPERFECT_GRAPHICS) // UK
CONS( 1979, database, 0,        vc4000, database, vc4000,  vc4000_state, empty_init, "Voltmace",    "Voltmace Database",                          MACHINE_IMPERFECT_GRAPHICS) // UK
CONS( 1979, vmdtbase, database, 0,      database, vc4000,  vc4000_state, empty_init, "Videomaster", "Videomaster Database Games-Computer",        MACHINE_IMPERFECT_GRAPHICS) // UK
CONS( 1979, rwtrntcs, 0,        vc4000, rwtrntcs, vc4000,  vc4000_state, empty_init, "Rowtron",     "Rowtron Television Computer System",         MACHINE_IMPERFECT_GRAPHICS) // UK
CONS( 1979, telngtcs, rwtrntcs, 0,      rwtrntcs, vc4000,  vc4000_state, empty_init, "Teleng",      "Teleng Television Computer System",          MACHINE_IMPERFECT_GRAPHICS) // UK
CONS( 1979, krvnjvtv, 0,        vc4000, vc4000,   vc4000,  vc4000_state, empty_init, "SOE",         "OC Jeu Video TV Karvan",                     MACHINE_IMPERFECT_GRAPHICS) // France
CONS( 1979, oc2000,   krvnjvtv, 0,      vc4000,   vc4000,  vc4000_state, empty_init, "SOE",         "OC-2000",                                    MACHINE_IMPERFECT_GRAPHICS) // France
CONS( 1980, mpt05,    0,        vc4000, vc4000,   vc4000,  vc4000_state, empty_init, "ITMC",        "MPT-05",                                     MACHINE_IMPERFECT_GRAPHICS) // France
CONS( 1982, h21,      0,        vc4000, h21,      vc4000,  vc4000_state, empty_init, "TRQ",         "Video Computer H-21",                        MACHINE_IMPERFECT_GRAPHICS) // Spain
CONS( 1979, elektor,  0,        0,      elektor,  elektor, vc4000_state, empty_init, "Elektor",     "Elektor TV Games Computer",                  MACHINE_IMPERFECT_GRAPHICS)
