/***************************************************************************

Apple II

This family of computers bank-switches everything up the wazoo.

Remarkable features
-------------------

Apple II (original model)
-------------------------

RAM: 4/8/12/16/20/24/32/36/48 KB (according to the manual)

ROM: 8 KB mapped to $E000-$FFFF
Empty ROM sockets mapped at $D000-$D7FF (usually occupied by Programmer's
Aid #1 chip) and $D800-$DFFF (usually empty, but a couple of 3rd party
chips were produced)

HI-RES Palette has only 4 colors: 0 - black, 1 - green, 2 - purple,
3 - white

Due to an hardware bug, green/purple artifacts are present in text mode
too!

No 80 columns
No Open/Solid Apple keys
No Up/Down arrows key

Users often connected the SHIFT key to the paddle #2 button (mapped to
$C063) in order to inform properly written software that characters were
to be intended upper/lower case

*** TODO: Should MESS emulate this via a dipswitch?

Integer BASIC in ROM, AppleSoft must be loaded from disk or tape

No AutoStart ROM: once the machine was switched on, the user had to manually
perform the reset cycle pressing, guess what, RESET ;)

Apple II+
---------

RAM: 16/32/48 KB + extra 16 KB at $C000 if using Apple Language Card
ROM: 12 KB mapped to $D000-$DFFF

HI-RES Palette has four more entries: 4 - black (again), 5 - orange,
6 - blue, 7 - white (again)

No more artifact bug in text mode

No 80 columns
No Open/Solid Apple keys
NO Up/Down arrows keys

Users still did the SHIFT key mod

AppleSoft BASIC in ROM

AutoStart ROM - no more need to press RESET after switching the machine on

Apple IIe
---------

RAM: 64 KB + optional bank of 64 KB (see 80 columns card)
ROM: 16 KB

80 columns card: this card was available in two versions - one equipped
with 1 KB of memory to provide the extra RAM for the display, the other
equipped with full 64 KB of RAM - the 80 columns card is not included
in the standard configuration and is available as add-on.

Open/Solid Apple keys mapped to buttons 0 and 1 of the paddle #1
Up/Down arrows keys
Connector for an optional numeric keypad

Apple begins manufacturing its machines with the SHIFT key mod

Revision A motherboards cannot handle double-hires graphics, Revision B can

*** TODO: Should MESS emulate this via a dipswitch?

Apple IIe (enhanced)
--------------------

The enhancement consists in bugfix of the ROM code, a 65c02 instead of the
6502 and a change in the character generator ROM which now includes the
so called "MouseText" characters (thus, no flashing characters in 80
columns mode)

Double hi-res mode is supported

Apple IIe (Platinum)
--------------------

Identical to IIe enhanced except for:

The numerical keypad is integrated into the main keyboard (although the
internal connector is still present)

The CLEAR key on the keypad generates the same character of the ESC key,
but some users did an hardware modification so that it generates CTRL-X

*** TODO: Should MESS emulate this via a dipswitch?

The 64 KB 80 columns card is built in

Due to the SHIFT key mod, if the user press both SHIFT and the paddle
button where the shift key was connected, a short circuit is caused
and the power supply is shut down!

Apple IIc
---------

Same as IIe enhanced (Rev B) except for:

There are no slots in hardware. The machine however sees (for compatibility
reasons):

Two Super Serial Cards in slots 1-2
80 columns card (64 KB version) in slot 3
Mouse in slot 4
Easter Egg in slot 5 (!)
Disk II in slot 6
External 5.25 drive in slot 7

MouseText characters

No numerical keypad

Switchables keyboard layouts (the user, via an external switch, can choose
between two layouts, i.e. US and German, and in the USA QWERTY and Dvorak)

*** TODO: Should MESS emulate this?

Apple IIc (UniDisk 3.5)
-----------------------

Identical to IIc except for:

ROM: 32 KB

The disk firmware can handle up to four 3.5 disk drives or three 3.5 drives
and a 5.25 drive

Preliminary support (but not working and never completed) for AppleTalk
network in slot 7

Apple IIc (Original Memory Expansion)
-------------------------------------

Identical to IIc except for:

Support for Memory Expansion Board (mapped to slot 4)
This card can provide up to 1 MB of RAM in increments of 256 MB
The firmware in ROM sees the extra RAM as a RAMdisk

Since the expansion is mapped to slot 4, mouse is now mapped to slot 7

Apple IIc (Revised Memory Expansion)
------------------------------------

ROMSET NOT DUMPED

Identical to IIc (OME) except for bugfixes

Apple IIc Plus
--------------

Identical to IIc (RME) except for:

The 65c02 works at 4MHz

The machine has an internal "Apple 3.5" drive (which is DIFFERENT from the
UniDisk 3.5 drive!)

The external drive port supports not only 5.25 drives but also UniDisk and
Apple 3.5 drives, allowing via daisy-chaining any combination of UniDisk,
Apple 3.5 and Apple 5.25 drives - up to three devices

***************************************************************************/


#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "cpu/z80/z80.h"
#include "imagedev/flopdrv.h"
#include "imagedev/cassette.h"
#include "formats/ap2_dsk.h"
#include "includes/apple2.h"
#include "machine/ay3600.h"
#include "sound/speaker.h"
#include "machine/ram.h"

#include "machine/a2bus.h"
#include "machine/a2lang.h"
#include "machine/a2diskii.h"
#include "machine/a2mockingboard.h"
#include "machine/a2cffa.h"
#include "machine/a2memexp.h"
#include "machine/a2scsi.h"
#include "machine/a2thunderclock.h"
#include "machine/a2softcard.h"
#include "machine/a2videoterm.h"
#include "machine/a2ssc.h"
#include "machine/a2swyft.h"
#include "machine/a2themill.h"
#include "machine/a2sam.h"
#include "machine/a2alfam2.h"
#include "machine/laser128.h"
#include "machine/a2echoii.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define JOYSTICK_DELTA			80
#define JOYSTICK_SENSITIVITY	50
#define JOYSTICK_AUTOCENTER     80
#define PADDLE_DELTA            10
#define PADDLE_SENSITIVITY      10
#define PADDLE_AUTOCENTER       0

static WRITE8_DEVICE_HANDLER(a2bus_irq_w)
{
    apple2_state *a2 = device->machine().driver_data<apple2_state>();

    a2->m_maincpu->set_input_line(M6502_IRQ_LINE, data);
}

static WRITE8_DEVICE_HANDLER(a2bus_nmi_w)
{
    apple2_state *a2 = device->machine().driver_data<apple2_state>();

    a2->m_maincpu->set_input_line(INPUT_LINE_NMI, data);
}

static WRITE8_DEVICE_HANDLER(a2bus_inh_w)
{
    apple2_state *a2 = device->machine().driver_data<apple2_state>();

    a2->m_inh_slot = data;
    apple2_update_memory(device->machine());
}

/***************************************************************************
    ADDRESS MAP
***************************************************************************/

static ADDRESS_MAP_START( apple2_map, AS_PROGRAM, 8, apple2_state )
	/* nothing in the address map - everything is added dynamically */
ADDRESS_MAP_END



/***************************************************************************
    INPUT PORTS
***************************************************************************/

static INPUT_PORTS_START( apple2_joystick )
	PORT_START("joystick_1_x")		/* Joystick 1 X Axis */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X) PORT_NAME("P1 Joystick X")
	PORT_SENSITIVITY(JOYSTICK_SENSITIVITY)
	PORT_KEYDELTA(JOYSTICK_DELTA)
	PORT_CENTERDELTA(JOYSTICK_AUTOCENTER)
	PORT_MINMAX(0,0xff) PORT_PLAYER(1)
	PORT_CODE_DEC(KEYCODE_4_PAD)	PORT_CODE_INC(KEYCODE_6_PAD)
	PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH)	PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH)

	PORT_START("joystick_1_y")		/* Joystick 1 Y Axis */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y) PORT_NAME("P1 Joystick Y")
	PORT_SENSITIVITY(JOYSTICK_SENSITIVITY)
	PORT_KEYDELTA(JOYSTICK_DELTA)
	PORT_CENTERDELTA(JOYSTICK_AUTOCENTER)
	PORT_MINMAX(0,0xff) PORT_PLAYER(1)
	PORT_CODE_DEC(KEYCODE_8_PAD)	PORT_CODE_INC(KEYCODE_2_PAD)
	PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH)		PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH)

	PORT_START("joystick_2_x")		/* Joystick 2 X Axis */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X) PORT_NAME("P2 Joystick X")
	PORT_SENSITIVITY(JOYSTICK_SENSITIVITY)
	PORT_KEYDELTA(JOYSTICK_DELTA)
	PORT_CENTERDELTA(JOYSTICK_AUTOCENTER)
	PORT_MINMAX(0,0xff) PORT_PLAYER(2)
	PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH)	PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH)

	PORT_START("joystick_2_y")		/* Joystick 2 Y Axis */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y) PORT_NAME("P2 Joystick Y")
	PORT_SENSITIVITY(JOYSTICK_SENSITIVITY)
	PORT_KEYDELTA(JOYSTICK_DELTA)
	PORT_CENTERDELTA(JOYSTICK_AUTOCENTER)
	PORT_MINMAX(0,0xff) PORT_PLAYER(2)
	PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH)		PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH)

	PORT_START("joystick_buttons")
    PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1)  PORT_PLAYER(1)			PORT_CODE(KEYCODE_0_PAD)	PORT_CODE(JOYCODE_BUTTON1)
    PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2)  PORT_PLAYER(1)			PORT_CODE(KEYCODE_ENTER_PAD)PORT_CODE(JOYCODE_BUTTON2)
    PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1)  PORT_PLAYER(2)			PORT_CODE(JOYCODE_BUTTON1)
INPUT_PORTS_END

/*static INPUT_PORTS_START( apple2_paddle )
    PORT_START("paddle_0")
    PORT_BIT( 0xff, 0x80, IPT_PADDLE) PORT_NAME("P1 Paddle 0")
    PORT_SENSITIVITY(PADDLE_SENSITIVITY)
    PORT_KEYDELTA(PADDLE_DELTA)
    PORT_CENTERDELTA(PADDLE_AUTOCENTER)
    PORT_MINMAX(0,0xff) PORT_PLAYER(1)
    PORT_CODE_DEC(KEYCODE_4_PAD)    PORT_CODE_INC(KEYCODE_6_PAD)
    PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH)    PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH)

    PORT_START("paddle_1")
    PORT_BIT( 0xff, 0x80, IPT_PADDLE) PORT_NAME("P1 Paddle 1")
    PORT_SENSITIVITY(PADDLE_SENSITIVITY)
    PORT_KEYDELTA(PADDLE_DELTA)
    PORT_CENTERDELTA(PADDLE_AUTOCENTER)
    PORT_MINMAX(0,0xff) PORT_PLAYER(1)
    PORT_CODE_DEC(KEYCODE_8_PAD)    PORT_CODE_INC(KEYCODE_2_PAD)
    PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH)      PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH)

    PORT_START("paddle_2")
    PORT_BIT( 0xff, 0x80, IPT_PADDLE) PORT_NAME("P2 Paddle 0")
    PORT_SENSITIVITY(PADDLE_SENSITIVITY)
    PORT_KEYDELTA(PADDLE_DELTA)
    PORT_CENTERDELTA(PADDLE_AUTOCENTER)
    PORT_MINMAX(0,0xff) PORT_PLAYER(2)
    PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH)    PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH)

    PORT_START("paddle_3")
    PORT_BIT( 0xff, 0x80, IPT_PADDLE) PORT_NAME("P2 Paddle 1")
    PORT_SENSITIVITY(PADDLE_SENSITIVITY)
    PORT_KEYDELTA(PADDLE_DELTA)
    PORT_CENTERDELTA(PADDLE_AUTOCENTER)
    PORT_MINMAX(0,0xff) PORT_PLAYER(2)
    PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH)      PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH)
INPUT_PORTS_END
*/
static INPUT_PORTS_START( apple2_gameport )
	PORT_INCLUDE( apple2_joystick )
	//PORT_INCLUDE( apple2_paddle )
INPUT_PORTS_END

static INPUT_PORTS_START( apple2_common )
    PORT_START("keyb_0")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
    PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT)		PORT_CODE(KEYCODE_LEFT)
    PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
    PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
    PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
    PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Return")	PORT_CODE(KEYCODE_ENTER)	PORT_CHAR(13)
    PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT)		PORT_CODE(KEYCODE_RIGHT)
    PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc")		PORT_CODE(KEYCODE_ESC)		PORT_CHAR(27)

    PORT_START("keyb_1")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)	PORT_CHAR(' ')
    PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
    PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)	PORT_CHAR(',') PORT_CHAR('<')
    PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)	PORT_CHAR(':') PORT_CHAR('*')
    PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)	PORT_CHAR('.') PORT_CHAR('>')
    PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)	PORT_CHAR('/') PORT_CHAR('?')
    PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)		PORT_CHAR('0')
    PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)		PORT_CHAR('1') PORT_CHAR('!')

    PORT_START("keyb_2")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)	PORT_CHAR('2') PORT_CHAR('\"')
    PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)	PORT_CHAR('3') PORT_CHAR('#')
    PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)	PORT_CHAR('4') PORT_CHAR('$')
    PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)	PORT_CHAR('5') PORT_CHAR('%')
    PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)	PORT_CHAR('6') PORT_CHAR('&')
    PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)	PORT_CHAR('7') PORT_CHAR('\'')
    PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)	PORT_CHAR('8') PORT_CHAR('(')
    PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)	PORT_CHAR('9') PORT_CHAR(')')

    PORT_START("keyb_3")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)	PORT_CHAR(';') PORT_CHAR('+')
    PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)	PORT_CHAR('-') PORT_CHAR('=')
    PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
    PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
    PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
    PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
    PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)		PORT_CHAR('A')
    PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)		PORT_CHAR('B')

    PORT_START("keyb_4")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)	PORT_CHAR('C')
    PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)	PORT_CHAR('D')
    PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)	PORT_CHAR('E')
    PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)	PORT_CHAR('F')
    PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)	PORT_CHAR('G')
    PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)	PORT_CHAR('H')
    PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)	PORT_CHAR('I')
    PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)	PORT_CHAR('J')

    PORT_START("keyb_5")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)	PORT_CHAR('K')
    PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)	PORT_CHAR('L')
    PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)	PORT_CHAR('M') PORT_CHAR(']')
    PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)	PORT_CHAR('N') PORT_CHAR('^')
    PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)	PORT_CHAR('O')
    PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)	PORT_CHAR('P') PORT_CHAR('@')
    PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)	PORT_CHAR('Q')
    PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)	PORT_CHAR('R')

    PORT_START("keyb_6")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)	PORT_CHAR('S')
    PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)	PORT_CHAR('T')
    PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)	PORT_CHAR('U')
    PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)	PORT_CHAR('V')
    PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)	PORT_CHAR('W')
    PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)	PORT_CHAR('X')
    PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)	PORT_CHAR('Y')
    PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)	PORT_CHAR('Z')

    PORT_START("keyb_special")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
    PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Shift")	PORT_CODE(KEYCODE_LSHIFT)	PORT_CHAR(UCHAR_SHIFT_1)
    PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Shift")	PORT_CODE(KEYCODE_RSHIFT)	PORT_CHAR(UCHAR_SHIFT_1)
    PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Control")		PORT_CODE(KEYCODE_LCONTROL)	PORT_CHAR(UCHAR_SHIFT_2)
    PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
    PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
    PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RESET")		PORT_CODE(KEYCODE_F12)
INPUT_PORTS_END

static INPUT_PORTS_START( apple2 )
	PORT_INCLUDE(apple2_common)

	PORT_START("keyb_repeat")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("REPT")			PORT_CODE(KEYCODE_BACKSLASH)PORT_CHAR('\\')

	/* other devices */
	PORT_INCLUDE( apple2_gameport )
INPUT_PORTS_END

static INPUT_PORTS_START( apple2p )
	PORT_INCLUDE( apple2 )

	PORT_START("reset_dip")
	PORT_DIPNAME( 0x01, 0x01, "Reset" )
	PORT_DIPSETTING( 0x01, "CTRL-RESET" )
	PORT_DIPSETTING( 0x00, "RESET" )
INPUT_PORTS_END

static INPUT_PORTS_START( apple2e_common )
    PORT_START("keyb_0")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Delete")	PORT_CODE(KEYCODE_BACKSPACE)PORT_CHAR(8)
    PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT)		PORT_CODE(KEYCODE_LEFT)
    PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab")		PORT_CODE(KEYCODE_TAB)		PORT_CHAR(9)
    PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_DOWN)		PORT_CODE(KEYCODE_DOWN)		PORT_CHAR(10)
    PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_UP)		PORT_CODE(KEYCODE_UP)
    PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Return")	PORT_CODE(KEYCODE_ENTER)	PORT_CHAR(13)
    PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT)		PORT_CODE(KEYCODE_RIGHT)
    PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc")		PORT_CODE(KEYCODE_ESC)		PORT_CHAR(27)

    PORT_START("keyb_1")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)	PORT_CHAR(' ')
    PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)	PORT_CHAR('\'') PORT_CHAR('\"')
    PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)	PORT_CHAR(',') PORT_CHAR('<')
    PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)	PORT_CHAR('-') PORT_CHAR('_')
    PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)	PORT_CHAR('.') PORT_CHAR('>')
    PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)	PORT_CHAR('/') PORT_CHAR('?')
    PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)		PORT_CHAR('0') PORT_CHAR(')')
    PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)		PORT_CHAR('1') PORT_CHAR('!')

    PORT_START("keyb_2")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)	PORT_CHAR('2') PORT_CHAR('@')
    PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)	PORT_CHAR('3') PORT_CHAR('#')
    PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)	PORT_CHAR('4') PORT_CHAR('$')
    PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)	PORT_CHAR('5') PORT_CHAR('%')
    PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)	PORT_CHAR('6') PORT_CHAR('^')
    PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)	PORT_CHAR('7') PORT_CHAR('&')
    PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)	PORT_CHAR('8') PORT_CHAR('*')
    PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)	PORT_CHAR('9') PORT_CHAR('(')

    PORT_START("keyb_3")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)		PORT_CHAR(';') PORT_CHAR(':')
    PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)		PORT_CHAR('=') PORT_CHAR('+')
    PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)	PORT_CHAR('[') PORT_CHAR('{')
    PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)	PORT_CHAR('\\') PORT_CHAR('|')
    PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)	PORT_CHAR(']') PORT_CHAR('}')
    PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)		PORT_CHAR('`') PORT_CHAR('~')
    PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)			PORT_CHAR('A') PORT_CHAR('a')
    PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)			PORT_CHAR('B') PORT_CHAR('b')

    PORT_START("keyb_4")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)	PORT_CHAR('C') PORT_CHAR('c')
    PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)	PORT_CHAR('D') PORT_CHAR('d')
    PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)	PORT_CHAR('E') PORT_CHAR('e')
    PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)	PORT_CHAR('F') PORT_CHAR('f')
    PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)	PORT_CHAR('G') PORT_CHAR('g')
    PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)	PORT_CHAR('H') PORT_CHAR('h')
    PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)	PORT_CHAR('I') PORT_CHAR('i')
    PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)	PORT_CHAR('J') PORT_CHAR('j')

    PORT_START("keyb_5")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)	PORT_CHAR('K') PORT_CHAR('k')
    PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)	PORT_CHAR('L') PORT_CHAR('l')
    PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)	PORT_CHAR('M') PORT_CHAR('m')
    PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)	PORT_CHAR('N') PORT_CHAR('n')
    PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)	PORT_CHAR('O') PORT_CHAR('o')
    PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)	PORT_CHAR('P') PORT_CHAR('p')
    PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)	PORT_CHAR('Q') PORT_CHAR('q')
    PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)	PORT_CHAR('R') PORT_CHAR('r')

    PORT_START("keyb_6")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)	PORT_CHAR('S') PORT_CHAR('s')
    PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)	PORT_CHAR('T') PORT_CHAR('t')
    PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)	PORT_CHAR('U') PORT_CHAR('u')
    PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)	PORT_CHAR('V') PORT_CHAR('v')
    PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)	PORT_CHAR('W') PORT_CHAR('w')
    PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)	PORT_CHAR('X') PORT_CHAR('x')
    PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)	PORT_CHAR('Y') PORT_CHAR('y')
    PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)	PORT_CHAR('Z') PORT_CHAR('z')

	PORT_START("keyb_special")
    PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_KEYBOARD) PORT_NAME("Caps Lock")	PORT_CODE(KEYCODE_CAPSLOCK)	PORT_TOGGLE
    PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Shift")	PORT_CODE(KEYCODE_LSHIFT)	PORT_CHAR(UCHAR_SHIFT_1)
    PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Shift")	PORT_CODE(KEYCODE_RSHIFT)	PORT_CHAR(UCHAR_SHIFT_1)
    PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Control")		PORT_CODE(KEYCODE_LCONTROL)	PORT_CHAR(UCHAR_SHIFT_2)
    PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Open Apple")	PORT_CODE(KEYCODE_LALT)
    PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Solid Apple")	PORT_CODE(KEYCODE_RALT)
    PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RESET")		PORT_CODE(KEYCODE_F12)
INPUT_PORTS_END

static INPUT_PORTS_START( apple2_keypad )
	PORT_START("keypad_1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)	PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)	PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)	PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)	PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)	PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)	PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)	PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)	PORT_CHAR(UCHAR_MAMEKEY(7_PAD))

	PORT_START("keypad_2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)		PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)		PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD)	PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)	PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)	PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)	PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)		PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD)	PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
INPUT_PORTS_END

static INPUT_PORTS_START( apple2e )
	PORT_INCLUDE( apple2e_common )
	PORT_INCLUDE( apple2_gameport )
INPUT_PORTS_END

INPUT_PORTS_START( apple2ep )
	PORT_INCLUDE( apple2e_common )
	PORT_INCLUDE( apple2_gameport )
	PORT_INCLUDE( apple2_keypad )
INPUT_PORTS_END

/* according to Steve Nickolas (author of Dapple), our original palette would
 * have been more appropriate for an Apple IIgs.  So we've substituted in the
 * Robert Munafo palette instead, which is more accurate on 8-bit Apples
 */
static const rgb_t apple2_palette[] =
{
	RGB_BLACK,
	MAKE_RGB(0xE3, 0x1E, 0x60),	/* Dark Red */
	MAKE_RGB(0x60, 0x4E, 0xBD),	/* Dark Blue */
	MAKE_RGB(0xFF, 0x44, 0xFD),	/* Purple */
	MAKE_RGB(0x00, 0xA3, 0x60),	/* Dark Green */
	MAKE_RGB(0x9C, 0x9C, 0x9C),	/* Dark Gray */
	MAKE_RGB(0x14, 0xCF, 0xFD),	/* Medium Blue */
	MAKE_RGB(0xD0, 0xC3, 0xFF),	/* Light Blue */
	MAKE_RGB(0x60, 0x72, 0x03),	/* Brown */
	MAKE_RGB(0xFF, 0x6A, 0x3C),	/* Orange */
	MAKE_RGB(0x9C, 0x9C, 0x9C),	/* Light Grey */
	MAKE_RGB(0xFF, 0xA0, 0xD0),	/* Pink */
	MAKE_RGB(0x14, 0xF5, 0x3C),	/* Light Green */
	MAKE_RGB(0xD0, 0xDD, 0x8D),	/* Yellow */
	MAKE_RGB(0x72, 0xFF, 0xD0),	/* Aquamarine */
	RGB_WHITE
};

/* Initialize the palette */
PALETTE_INIT_MEMBER(apple2_state,apple2)
{
	palette_set_colors(machine(), 0, apple2_palette, ARRAY_LENGTH(apple2_palette));
}

static const cassette_interface apple2_cassette_interface =
{
	cassette_default_formats,
	NULL,
	(cassette_state)(CASSETTE_STOPPED),
	NULL,
	NULL
};

static const struct a2bus_interface a2bus_intf =
{
	// interrupt lines
	DEVCB_HANDLER(a2bus_irq_w),
	DEVCB_HANDLER(a2bus_nmi_w),
    DEVCB_HANDLER(a2bus_inh_w)
};

static const struct a2eauxslot_interface a2eauxbus_intf =
{
	// interrupt lines
	DEVCB_HANDLER(a2bus_irq_w),
	DEVCB_HANDLER(a2bus_nmi_w)
};

static SLOT_INTERFACE_START(apple2_slot0_cards)
    SLOT_INTERFACE("lang", A2BUS_LANG)      /* Apple II Language Card */
SLOT_INTERFACE_END

static SLOT_INTERFACE_START(apple2_cards)
    SLOT_INTERFACE("diskii", A2BUS_DISKII)  /* Disk II Controller Card */
    SLOT_INTERFACE("mockingboard", A2BUS_MOCKINGBOARD)  /* Sweet Micro Systems Mockingboard */
    SLOT_INTERFACE("phasor", A2BUS_PHASOR)  /* Applied Engineering Phasor */
    SLOT_INTERFACE("cffa2", A2BUS_CFFA2)  /* CFFA2000 Compact Flash for Apple II (www.dreher.net), 65C02/65816 firmware */
    SLOT_INTERFACE("cffa202", A2BUS_CFFA2_6502)  /* CFFA2000 Compact Flash for Apple II (www.dreher.net), 6502 firmware */
    SLOT_INTERFACE("memexp", A2BUS_MEMEXP)  /* Apple II Memory Expansion Card */
    SLOT_INTERFACE("ramfactor", A2BUS_RAMFACTOR)    /* Applied Engineering RamFactor */
    SLOT_INTERFACE("thclock", A2BUS_THUNDERCLOCK)    /* ThunderWare ThunderClock Plus */
    SLOT_INTERFACE("softcard", A2BUS_SOFTCARD)  /* Microsoft SoftCard */
    SLOT_INTERFACE("videoterm", A2BUS_VIDEOTERM)    /* Videx VideoTerm */
    SLOT_INTERFACE("ssc", A2BUS_SSC)    /* Apple Super Serial Card */
    SLOT_INTERFACE("swyft", A2BUS_SWYFT)    /* IAI SwyftCard */
    SLOT_INTERFACE("themill", A2BUS_THEMILL)    /* Stellation Two The Mill (6809 card) */
    SLOT_INTERFACE("sam", A2BUS_SAM)    /* SAM Software Automated Mouth (8-bit DAC + speaker) */
    SLOT_INTERFACE("alfam2", A2BUS_ALFAM2)    /* ALF Apple Music II */
    SLOT_INTERFACE("echoii", A2BUS_ECHOII)    /* Street Electronics Echo II */
    SLOT_INTERFACE("ap16", A2BUS_IBSAP16)    /* IBS AP16 (German VideoTerm clone) */
    SLOT_INTERFACE("ap16alt", A2BUS_IBSAP16ALT)    /* IBS AP16 (German VideoTerm clone), alternate revision */
    SLOT_INTERFACE("vtc1", A2BUS_VTC1)    /* Unknown VideoTerm clone #1 */
    SLOT_INTERFACE("vtc2", A2BUS_VTC2)    /* Unknown VideoTerm clone #2 */
//    SLOT_INTERFACE("scsi", A2BUS_SCSI)  /* Apple II SCSI Card */
SLOT_INTERFACE_END

static SLOT_INTERFACE_START(apple2eaux_cards)
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( apple2_common, apple2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, 1021800)		/* close to actual CPU frequency of 1.020484 MHz */
	MCFG_CPU_PROGRAM_MAP(apple2_map)
	MCFG_TIMER_ADD_SCANLINE("scantimer", apple2_interrupt, "screen", 0, 1)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_MACHINE_START_OVERRIDE(apple2_state, apple2 )

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(280*2, 192)
	MCFG_SCREEN_VISIBLE_AREA(0, (280*2)-1,0,192-1)
	MCFG_SCREEN_UPDATE_DRIVER(apple2_state, screen_update_apple2)

	MCFG_PALETTE_LENGTH(ARRAY_LENGTH(apple2_palette))
	MCFG_PALETTE_INIT_OVERRIDE(apple2_state,apple2)

	MCFG_VIDEO_START_OVERRIDE(apple2_state,apple2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("a2speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* slot devices */
    MCFG_A2BUS_BUS_ADD("a2bus", "maincpu", a2bus_intf)
    MCFG_A2BUS_SLOT_ADD("a2bus", "sl0", apple2_slot0_cards, "lang", NULL)
    MCFG_A2BUS_SLOT_ADD("a2bus", "sl1", apple2_cards, NULL, NULL)
    MCFG_A2BUS_SLOT_ADD("a2bus", "sl2", apple2_cards, NULL, NULL)
    MCFG_A2BUS_SLOT_ADD("a2bus", "sl3", apple2_cards, NULL, NULL)
    MCFG_A2BUS_SLOT_ADD("a2bus", "sl4", apple2_cards, "mockingboard", NULL)
    MCFG_A2BUS_SLOT_ADD("a2bus", "sl5", apple2_cards, NULL, NULL)
    MCFG_A2BUS_SLOT_ADD("a2bus", "sl6", apple2_cards, "diskii", NULL)
    MCFG_A2BUS_SLOT_ADD("a2bus", "sl7", apple2_cards, NULL, NULL)

	MCFG_SOFTWARE_LIST_ADD("flop525_list","apple2")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( apple2, apple2_common )
	MCFG_MACHINE_START_OVERRIDE(apple2_state,apple2orig)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
	MCFG_RAM_EXTRA_OPTIONS("4K,8K,12K,16K,20K,24K,32K,36K,48K")
	MCFG_RAM_DEFAULT_VALUE(0x00)
	/* At the moment the RAM bank $C000-$FFFF is available only if you choose   */
	/* default configuration: on real machine is present also in configurations */
	/* with less memory, provided that the language card is installed           */
	MCFG_CASSETTE_ADD( CASSETTE_TAG, apple2_cassette_interface )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( apple2p, apple2_common )
	MCFG_MACHINE_START_OVERRIDE(apple2_state,apple2orig)
	MCFG_VIDEO_START_OVERRIDE(apple2_state,apple2p)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
	MCFG_RAM_EXTRA_OPTIONS("16K,32K,48K")
	MCFG_RAM_DEFAULT_VALUE(0x00)
	/* At the moment the RAM bank $C000-$FFFF is available only if you choose   */
	/* default configuration: on real machine is present also in configurations */
	/* with less memory, provided that the language card is installed           */
	MCFG_CASSETTE_ADD( CASSETTE_TAG, apple2_cassette_interface )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( apple2e, apple2_common )
    MCFG_VIDEO_START_OVERRIDE(apple2_state,apple2e)
	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")
	MCFG_RAM_EXTRA_OPTIONS("64K")
	MCFG_RAM_DEFAULT_VALUE(0x00)
	MCFG_CASSETTE_ADD( CASSETTE_TAG, apple2_cassette_interface )

    // IIe and later have no physical slot 0, the "language card" is built into the motherboard
    MCFG_A2BUS_SLOT_REMOVE("sl0")
    MCFG_A2BUS_ONBOARD_ADD("a2bus", "sl0", A2BUS_LANG, NULL)

    MCFG_A2EAUXSLOT_BUS_ADD(AUXSLOT_TAG, "maincpu", a2eauxbus_intf)
    MCFG_A2EAUXSLOT_SLOT_ADD(AUXSLOT_TAG, "slaux", apple2eaux_cards, NULL, NULL)

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( tk2000, apple2_common )
	MCFG_MACHINE_START_OVERRIDE(apple2_state,tk2000)
    MCFG_VIDEO_START_OVERRIDE(apple2_state,apple2e)
	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
	MCFG_RAM_DEFAULT_VALUE(0x00)
	MCFG_CASSETTE_ADD( CASSETTE_TAG, apple2_cassette_interface )

    // TK2000 doesn't have slots, and it doesn't emulate a language card
    // C05A maps RAM from C100-FFFF, C05B maps ROM
    MCFG_A2BUS_SLOT_REMOVE("sl0")
    MCFG_A2BUS_SLOT_REMOVE("sl1")
    MCFG_A2BUS_SLOT_REMOVE("sl2")
    MCFG_A2BUS_SLOT_REMOVE("sl3")
    MCFG_A2BUS_SLOT_REMOVE("sl4")
    MCFG_A2BUS_SLOT_REMOVE("sl5")
    MCFG_A2BUS_SLOT_REMOVE("sl6")
    MCFG_A2BUS_SLOT_REMOVE("sl7")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( mprof3, apple2e )

	/* internal ram */
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( apple2ee, apple2e )
	MCFG_CPU_REPLACE("maincpu", M65C02, 1021800)		/* close to actual CPU frequency of 1.020484 MHz */
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( apple2ep, apple2e )
	MCFG_CPU_REPLACE("maincpu", M65C02, 1021800)		/* close to actual CPU frequency of 1.020484 MHz */
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( apple2c, apple2ee )
    MCFG_A2BUS_SLOT_REMOVE("sl1")   // IIc has no slots, of course :)
    MCFG_A2BUS_SLOT_REMOVE("sl2")
    MCFG_A2BUS_SLOT_REMOVE("sl3")
    MCFG_A2BUS_SLOT_REMOVE("sl4")
    MCFG_A2BUS_SLOT_REMOVE("sl5")
    MCFG_A2BUS_SLOT_REMOVE("sl6")
    MCFG_A2BUS_SLOT_REMOVE("sl7")

    // TODO: populate the IIc's other virtual slots with ONBOARD_ADD
    MCFG_A2BUS_ONBOARD_ADD("a2bus", "sl6", A2BUS_DISKII, NULL)

    MCFG_A2EAUXSLOT_SLOT_REMOVE("slaux")
    MCFG_A2EAUXSLOT_BUS_REMOVE(AUXSLOT_TAG)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( apple2c_iwm, apple2c )

    MCFG_A2BUS_SLOT_REMOVE("sl6")
    MCFG_A2BUS_ONBOARD_ADD("a2bus", "sl6", A2BUS_IWM_FDC, NULL)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( laser128, apple2c )
	MCFG_MACHINE_START_OVERRIDE(apple2_state,laser128)

    MCFG_A2BUS_SLOT_REMOVE("sl6")

    MCFG_A2BUS_ONBOARD_ADD("a2bus", "sl1", A2BUS_LASER128, NULL)
    MCFG_A2BUS_ONBOARD_ADD("a2bus", "sl2", A2BUS_LASER128, NULL)
    MCFG_A2BUS_ONBOARD_ADD("a2bus", "sl3", A2BUS_LASER128, NULL)
    MCFG_A2BUS_ONBOARD_ADD("a2bus", "sl4", A2BUS_LASER128, NULL)
    MCFG_A2BUS_ONBOARD_ADD("a2bus", "sl5", A2BUS_LASER128, NULL)
    MCFG_A2BUS_ONBOARD_ADD("a2bus", "sl6", A2BUS_IWM_FDC, NULL)     // slots 6 and 7 are hacks for now
//    MCFG_A2BUS_ONBOARD_ADD("a2bus", "sl7", A2BUS_LASER128, NULL)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( space84, apple2p )
	MCFG_MACHINE_START_OVERRIDE(apple2_state,space84)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START(apple2) /* the classic, non-autoboot apple2 with integer basic in rom. optional card with autoboot and applesoft basic was possible but isn't yet supported */
	ROM_REGION(0x0800,"gfx1",0)
	ROM_LOAD ( "a2.chr", 0x0000, 0x0800, BAD_DUMP CRC(64f415c6) SHA1(f9d312f128c9557d9d6ac03bfad6c3ddf83e5659)) /* current dump is 341-0036 which is the appleII+ character generator, not the original appleII one, whose rom number is not yet known! */

	ROM_REGION(0x4700,"maincpu",0)
	ROM_LOAD_OPTIONAL ( "341-0016-00.d0", 0x1000, 0x0800, CRC(4234e88a) SHA1(c9a81d704dc2f0c3416c20f9c4ab71fedda937ed)) /* 341-0016: Programmer's Aid #1 D0 */

/* The area $D800-$DFFF in Apple II is reserved for 3rd party add-ons:
   Maybe MESS should map this space to a CARTSLOT device?              */

	ROM_LOAD ( "341-0001-00.e0", 0x2000, 0x0800, CRC(c0a4ad3b) SHA1(bf32195efcb34b694c893c2d342321ec3a24b98f)) /* Needs verification. From eBay: Label: S7925E // C48077 // 3410001-00 // (C)APPLE78 E0 */
	ROM_LOAD ( "341-0002-00.e8", 0x2800, 0x0800, CRC(a99c2cf6) SHA1(9767d92d04fc65c626223f25564cca31f5248980)) /* Needs verification. From eBay: Label: S7916E // C48078 // 3410002-00 // (C)APPLE78 E8 */
	ROM_LOAD ( "341-0003-00.f0", 0x3000, 0x0800, CRC(62230d38) SHA1(f268022da555e4c809ca1ae9e5d2f00b388ff61c)) /* Needs verification. From eBay: Label: S7908E // C48709 // 3410003 // CAPPLE78 F0 */
	ROM_LOAD ( "341-0004-00.f8", 0x3800, 0x0800, CRC(020a86d0) SHA1(52a18bd578a4694420009cad7a7a5779a8c00226))
	/* For the following bits, I'm not sure how to do this properly, since the P5 and P6 roms came in pairs and are in different address spaces...
    Also the 3.5" 400k? disk control rom, 341-0438-a should probably be here as well, assuming it could be used with an apple2/2+/2e */
	//ROMX_LOAD ( "341-0009.p5", 0x4500, 0x0100, CRC(d34eb2ff) SHA1(afd060e6f35faf3bb0146fa889fc787adf56330a), ROM_BIOS(1)) /* 341-0009: 13-sector disk drive, PROM P5 */
	//ROMX_LOAD ( "341-0027-a.p5", 0x4500, 0x0100, CRC(ce7144f6) SHA1(d4181c9f046aafc3fb326b381baac809d9e38d16), ROM_BIOS(2)) /* 341-0027-a: 16-sector disk drive (older version), PROM P5 */
	//ROMX_LOAD ( "341-0127-a.p5a", 0x4500, 0x0100, NO_DUMP, ROM_BIOS(3)) /* 341-0127-A: 16-sector disk drive (later version) PROM P5; Label: 341-0127-A // (C) APPLE 81 P5A (see 'Apple Disk II Interface Card.jpg') (I have a suspicion this rom is the same as the 341-0027-a one)*/

	//ROM_REGION(0x100,"wsmprom",0) /* prom 'p6' for the woz state machine on 650-X104- Disk II interface card */
	//ROMX_LOAD ( "341-0010.rom", 0x0000, 0x0100, CRC(62e22620) SHA1(e3d6d1c30653572b49ecc2dc54ce073978411a04), ROM_BIOS(1)) /* 341-0010: 13-sector disk drive, PROM P6 */
	//ROMX_LOAD ( "341-0028-a.rom", 0x0000, 0x0100, CRC(b72a2c70) SHA1(bc39fbd5b9a8d2287ac5d0a42e639fc4d3c2f9d4), ROM_BIOS(2)) /* 341-0028: 16-sector disk drive (older version), PROM P6 */
	//ROMX_LOAD ( "341-0128-a.rom", 0x0000, 0x0100, NO_DUMP, ROM_BIOS(3)) /* 341-0128-A: 16-sector disk drive (later version), PROM P6 Label: 341-0128-A // (C) APPLE 81 P6A (see 'Apple Disk II Interface Card.jpg') (This rom MIGHT be the same as the 341-0028 one) */
	ROM_END

ROM_START(apple2p) /* the autoboot apple2+ with applesoft (microsoft-written) basic in rom; optional card with monitor and integer basic was possible but isn't yet supported */
	ROM_REGION(0x0800,"gfx1",0)
	ROM_LOAD ( "341-0036.chr", 0x0000, 0x0800, CRC(64f415c6) SHA1(f9d312f128c9557d9d6ac03bfad6c3ddf83e5659))

	ROM_REGION(0x4700,"maincpu",0)
	ROM_LOAD ( "341-0011.d0", 0x1000, 0x0800, CRC(6f05f949) SHA1(0287ebcef2c1ce11dc71be15a99d2d7e0e128b1e))
	ROM_LOAD ( "341-0012.d8", 0x1800, 0x0800, CRC(1f08087c) SHA1(a75ce5aab6401355bf1ab01b04e4946a424879b5))
	ROM_LOAD ( "341-0013.e0", 0x2000, 0x0800, CRC(2b8d9a89) SHA1(8d82a1da63224859bd619005fab62c4714b25dd7))
	ROM_LOAD ( "341-0014.e8", 0x2800, 0x0800, CRC(5719871a) SHA1(37501be96d36d041667c15d63e0c1eff2f7dd4e9))
	ROM_LOAD ( "341-0015.f0", 0x3000, 0x0800, CRC(9a04eecf) SHA1(e6bf91ed28464f42b807f798fc6422e5948bf581))
	ROM_LOAD ( "341-0020-00.f8", 0x3800, 0x0800, CRC(079589c4) SHA1(a28852ff997b4790e53d8d0352112c4b1a395098)) /* 341-0020-00: Autostart Monitor/Applesoft Basic $f800; Was sometimes mounted on Language card; Label(from Apple Language Card - Front.jpg): S 8115 // C68018 // 341-0020-00 */
ROM_END

ROM_START(prav82)
	ROM_REGION(0x0800,"gfx1",0)
	ROM_LOAD ( "pravetz82.chr", 0x0000, 0x0800, BAD_DUMP CRC(8C55C984) SHA1(5a5a202000576b88b4ae2e180dd2d1b9b337b594)) // Taken from Agat computer

	ROM_REGION(0x4700,"maincpu",0)
	ROM_LOAD ( "pravetz82.d0", 0x1000, 0x0800, CRC(6f05f949) SHA1(0287ebcef2c1ce11dc71be15a99d2d7e0e128b1e))
	ROM_LOAD ( "pravetz82.d8", 0x1800, 0x0800, CRC(1f08087c) SHA1(a75ce5aab6401355bf1ab01b04e4946a424879b5))
	ROM_LOAD ( "pravetz82.e0", 0x2000, 0x0800, CRC(2b8d9a89) SHA1(8d82a1da63224859bd619005fab62c4714b25dd7))
	ROM_LOAD ( "pravetz82.e8", 0x2800, 0x0800, CRC(5719871a) SHA1(37501be96d36d041667c15d63e0c1eff2f7dd4e9))
	ROM_LOAD ( "pravetz82.f0", 0x3000, 0x0800, CRC(e26d9d35) SHA1(ce6e42e6c9a6c98e92522af7a6090cd04c56c778))
	ROM_LOAD ( "pravetz82.f8", 0x3800, 0x0800, CRC(57547818) SHA1(db30bedec98305e31a14acb9e2a92be1c4853807))
ROM_END

ROM_START(prav8m)
	ROM_REGION(0x0800,"gfx1",0)
	ROM_LOAD ( "pravetz8m.chr", 0x0000, 0x0800, BAD_DUMP CRC(8C55C984) SHA1(5a5a202000576b88b4ae2e180dd2d1b9b337b594)) // Taken from Agat computer
	ROM_REGION(0x4700,"maincpu",0)
	ROM_LOAD ( "pravetz8m.d0", 0x1000, 0x0800, CRC(6f05f949) SHA1(0287ebcef2c1ce11dc71be15a99d2d7e0e128b1e))
	ROM_LOAD ( "pravetz8m.d8", 0x1800, 0x0800, CRC(654b6f7b) SHA1(f7b1457b48fe6974c4de7e976df3a8fca6b7b661))
	ROM_LOAD ( "pravetz8m.e0", 0x2000, 0x0800, CRC(2b8d9a89) SHA1(8d82a1da63224859bd619005fab62c4714b25dd7))
	ROM_LOAD ( "pravetz8m.e8", 0x2800, 0x0800, CRC(5719871a) SHA1(37501be96d36d041667c15d63e0c1eff2f7dd4e9))
	ROM_LOAD ( "pravetz8m.f0", 0x3000, 0x0800, CRC(e26d9d35) SHA1(ce6e42e6c9a6c98e92522af7a6090cd04c56c778))
	ROM_LOAD ( "pravetz8m.f8", 0x3800, 0x0800, CRC(5bab0a46) SHA1(f6c0817ce37d2e2c43f482c339acaede0a73359b))
ROM_END

ROM_START( agat7 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "v1", "Version 1" )
	ROMX_LOAD( "monitor7.rom", 0x3800, 0x0800, CRC(071fda0b) SHA1(6089d46b7addc4e2ae096b2cf81124681bd2b27a), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "v2", "Version 2" )
	ROMX_LOAD( "agat_pzu.bin", 0x3800, 0x0800, CRC(c605163d) SHA1(b30fd1b264a347a9de69bb9e3105483254994d06), ROM_BIOS(2))
	// Floppy controllers
	ROM_LOAD( "shugart7.rom", 0x4500, 0x0100, CRC(c6e4850c) SHA1(71626d3d2d4bbeeac2b77585b45a5566d20b8d34))
	ROM_LOAD( "teac.rom",	  0x4500, 0x0100, CRC(94266928) SHA1(5d369bad6cdd6a70b0bb16480eba69640de87a2e))
	ROM_REGION(0x0800,"gfx1",0)
	ROM_LOAD( "agathe7.fnt", 0x0000, 0x0800, CRC(fcffb490) SHA1(0bda26ae7ad75f74da835c0cf6d9928f9508844c))
ROM_END

ROM_START( agat9 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "v1", "Version 1" )
	ROMX_LOAD( "monitor9.rom", 0x3800, 0x0800, CRC(b90bb66a) SHA1(02217f0785913b41fc25eabcff70fa814799c69a), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "v2", "Version 2" )
	ROMX_LOAD( "monitor91.rom", 0x3800, 0x0800, CRC(89b10fc1) SHA1(7fe1ede32b5525255f82597ca9c3c2034c5996fa), ROM_BIOS(2))
	// Floppy controllers
	ROM_LOAD( "shugart9.rom", 0x4500, 0x0100, CRC(964a0ce2) SHA1(bf955189ebffe874c20ef649a3db8177dc16af61))
	ROM_LOAD( "teac.rom",	  0x4500, 0x0100, CRC(94266928) SHA1(5d369bad6cdd6a70b0bb16480eba69640de87a2e))
	// Printer card
	ROM_LOAD( "cm6337.rom", 0x8000, 0x0100, CRC(73be16ec) SHA1(ead1abbef5b86f1def0b956147d5b267f0d544b5))
	ROM_LOAD( "cm6337p.rom", 0x8100, 0x0800, CRC(9120f11f) SHA1(78107653491e88d5ea12e07367c4c028771a4aca))
	ROM_REGION(0x0800,"gfx1",0)
	ROM_LOAD( "agathe9.fnt", 0x0000, 0x0800, CRC(8c55c984) SHA1(5a5a202000576b88b4ae2e180dd2d1b9b337b594))
ROM_END
ROM_START(apple2jp)
	ROM_REGION(0x0800,"gfx1",0)
	ROM_LOAD ( "a2jp.chr", 0x0000, 0x0800, BAD_DUMP CRC(487104b5) SHA1(0a382be58db5215c4a3de53b19a72fab660d5da2)) // not confirmed as the actual rom on motherboard

	ROM_REGION(0x4700,"maincpu",0)
	ROM_LOAD ( "a2p.d0", 0x1000, 0x0800, BAD_DUMP CRC(6f05f949) SHA1(0287ebcef2c1ce11dc71be15a99d2d7e0e128b1e)) // not confirmed as the actual rom on motherboard
	ROM_LOAD ( "a2p.d8", 0x1800, 0x0800, BAD_DUMP CRC(1f08087c) SHA1(a75ce5aab6401355bf1ab01b04e4946a424879b5)) // not confirmed as the actual rom on motherboard
	ROM_LOAD ( "a2p.e0", 0x2000, 0x0800, BAD_DUMP CRC(2b8d9a89) SHA1(8d82a1da63224859bd619005fab62c4714b25dd7)) // not confirmed as the actual rom on motherboard
	ROM_LOAD ( "a2p.e8", 0x2800, 0x0800, BAD_DUMP CRC(5719871a) SHA1(37501be96d36d041667c15d63e0c1eff2f7dd4e9)) // not confirmed as the actual rom on motherboard
	ROM_LOAD ( "a2p.f0", 0x3000, 0x0800, BAD_DUMP CRC(9a04eecf) SHA1(e6bf91ed28464f42b807f798fc6422e5948bf581)) // not confirmed as the actual rom on motherboard
	ROM_LOAD ( "a2jp.f8", 0x3800, 0x0800, CRC(6ea8379b) SHA1(00a75ae3b58e1917ad640249366f654608589cf4))
ROM_END

ROM_START(ace100)
	ROM_REGION(0x0800,"gfx1",0)
	ROM_LOAD ( "ace100.chr", 0x0000, 0x0800, BAD_DUMP CRC(64f415c6) SHA1(f9d312f128c9557d9d6ac03bfad6c3ddf83e5659)) // copy of a2.chr - real Ace chr is undumped

	ROM_REGION(0x4700,"maincpu",0)
	ROM_LOAD ( "ace100.rom", 0x1000, 0x3000, CRC(9d5ec94f) SHA1(8f2b3f2561788bebc7a805f620ec9e7ade973460))
ROM_END

ROM_START(apple2e)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD ( "342-0133-a.chr", 0x0000, 0x1000,CRC(b081df66) SHA1(7060de104046736529c1e8a687a0dd7b84f8c51b))
	ROM_LOAD ( "342-0133-a.chr", 0x1000, 0x1000,CRC(b081df66) SHA1(7060de104046736529c1e8a687a0dd7b84f8c51b))

	ROM_REGION(0x4700,"maincpu",0)
	ROM_LOAD ( "342-0135-b.64", 0x0000, 0x2000, CRC(e248835e) SHA1(523838c19c79f481fa02df56856da1ec3816d16e))
	ROM_LOAD ( "342-0134-a.64", 0x2000, 0x2000, CRC(fc3d59d8) SHA1(8895a4b703f2184b673078f411f4089889b61c54))

	ROM_REGION( 0x800, "keyboard", ROMREGION_ERASE00 )
//  ROM_LOAD_OPTIONAL( "341-0132-a.e12", 0x000, 0x800, CRC(7ded1ac6) SHA1(69f45cd487e8210e37c839df78a0b8930a3a6ac1) ) // 1982 US-DE
//  ROM_LOAD_OPTIONAL( "341-0150-a.e12", 0x000, 0x800, CRC(66ffacd7) SHA1(47bb9608be38ff75429a989b930a93b47099648e) ) // 1982 US-UK
//  ROM_LOAD_OPTIONAL( "341-0151-a.e12", 0x000, 0x800, CRC(64574bb4) SHA1(c44809bbb017bfe3c07dc99e87a3a9fa7b9741c3) ) // 1982 US-DE
//  ROM_LOAD_OPTIONAL( "342-0132-b.e12", 0x000, 0x800, CRC(ecfceb45) SHA1(c2f249589ee02121c27336af022bbca0cc9b2245) ) // 1982 US-Dvorak
//  ROM_LOAD( "342-0132-c.e12", 0x000, 0x800, CRC(e47045f4) SHA1(12a2e718f5f4acd69b6c33a45a4a940b1440a481) ) // 1983 US-Dvorak
ROM_END

ROM_START(mprof3)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD ( "mpf3.chr", 0x0000, 0x1000,CRC(2597bc19) SHA1(e114dcbb512ec24fb457248c1b53cbd78039ed20))
	ROM_LOAD ( "mpf3.chr", 0x1000, 0x1000,CRC(2597bc19) SHA1(e114dcbb512ec24fb457248c1b53cbd78039ed20))

	ROM_REGION(0x4700,"maincpu",0)
	ROM_LOAD ( "mpf3-cd.rom", 0x0000, 0x2000, CRC(5b662e06) SHA1(aa0db775ca78986480829fcc10f00e57629e1a7c))
	ROM_LOAD ( "mpf3-ef.rom", 0x2000, 0x2000, CRC(2c5e8b92) SHA1(befeb03e04b7c3ef36ef5829948a53880df85e92))

ROM_END

ROM_START(apple2ee)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD ( "342-0265-a.chr", 0x0000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))
	ROM_LOAD ( "342-0265-a.chr", 0x1000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))

	ROM_REGION(0x4700,"maincpu",0)
	ROM_LOAD ( "342-0304-a.e10", 0x0000, 0x2000, CRC(443aa7c4) SHA1(3aecc56a26134df51e65e17f33ae80c1f1ac93e6)) /* PCB: "CD ROM // 342-0304", 2364 mask rom */
	ROM_LOAD ( "342-0303-a.e8", 0x2000, 0x2000, CRC(95e10034) SHA1(afb09bb96038232dc757d40c0605623cae38088e)) /* PCB: "EF ROM // 342-0303", 2364 mask rom */

	ROM_REGION( 0x800, "keyboard", 0 )
	ROM_LOAD( "341-0132-d.e12", 0x000, 0x800, CRC(c506efb9) SHA1(8e14e85c645187504ec9d162b3ea614a0c421d32) )
ROM_END

ROM_START(apple2ep)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD ( "342-0265-a.chr", 0x0000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))
	ROM_LOAD ( "342-0265-a.chr", 0x1000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))

	ROM_REGION(0x4700,"maincpu",0)
	ROM_LOAD ("32-0349-b.128", 0x0000, 0x4000, CRC(1d70b193) SHA1(b8ea90abe135a0031065e01697c4a3a20d51198b)) /* should rom name be 342-0349-b? */
ROM_END

ROM_START(apple2c)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD ( "341-0265-a.chr", 0x0000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))
	ROM_LOAD ( "341-0265-a.chr", 0x1000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))

	ROM_REGION(0x4000,"maincpu",0)
	ROM_LOAD ( "a2c.128", 0x0000, 0x4000, CRC(f0edaa1b) SHA1(1a9b8aca5e32bb702ddb7791daddd60a89655729))
ROM_END

ROM_START(tk2000)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD ( "341-0265-a.chr", 0x0000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))
	ROM_LOAD ( "341-0265-a.chr", 0x1000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))

	ROM_REGION(0x4000,"maincpu",0)
    ROM_LOAD( "tk2000.rom",   0x000000, 0x004000, CRC(dfdbacc3) SHA1(bb37844c31616046630868a4399ee3d55d6df277) )
ROM_END

ROM_START(prav8c)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD ( "charrom.d20", 0x0000, 0x2000,CRC(935212cc) SHA1(934603a441c631bd841ea0d2ff39525474461e47))
	ROM_REGION(0x4000,"maincpu",0)
	ROM_LOAD ( "prom_cd.d46", 0x0000, 0x2000, CRC(195d0b48) SHA1(f8c4f3722159081f6950207f03bc85da30980c08))
	ROM_LOAD ( "prom_ef.d41", 0x2000, 0x2000, CRC(ec6aa2f6) SHA1(64bce893ebf0e22cd8f22436b97ef1bfeddf692f))
	ROM_REGION(0x2000,"unknown",0)
	ROM_LOAD ( "eprom.d38", 0x0000, 0x2000, CRC(c8d00b19) SHA1(13d156957ea68d0e7bc4be57cb1580c8b1399981))
ROM_END

ROM_START(apple2c0)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD ( "341-0265-a.chr", 0x0000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))
	ROM_LOAD ( "341-0265-a.chr", 0x1000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))

	ROM_REGION(0x8700,"maincpu",0)
	ROM_LOAD("3420033a.256", 0x0000, 0x8000, CRC(c8b979b3) SHA1(10767e96cc17bad0970afda3a4146564e6272ba1))
ROM_END

ROM_START(apple2c3)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD ( "341-0265-a.chr", 0x0000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))
	ROM_LOAD ( "341-0265-a.chr", 0x1000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))

	ROM_REGION(0x8700,"maincpu",0)
	ROM_LOAD("342-0445-a.256", 0x0000, 0x8000, CRC(bc5a79ff) SHA1(5338d9baa7ae202457b6500fde5883dbdc86e5d3))
ROM_END

ROM_START(apple2c4)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD ( "341-0265-a.chr", 0x0000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))
	ROM_LOAD ( "341-0265-a.chr", 0x1000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))

	ROM_REGION(0x8700,"maincpu",0)
	ROM_LOAD("3410445b.256", 0x0000, 0x8000, CRC(06f53328) SHA1(015061597c4cda7755aeb88b735994ffd2f235ca))
ROM_END

ROM_START(las3000)
	ROM_REGION(0x0800,"gfx1",0)
	ROM_LOAD ( "341-0036.chr", 0x0000, 0x0800, CRC(64f415c6) SHA1(f9d312f128c9557d9d6ac03bfad6c3ddf83e5659))

	ROM_REGION(0x8700,"maincpu",0)
	ROM_LOAD ( "las3000.rom", 0x4000, 0x4000, CRC(9C7AEB09) SHA1(3302ADF41E258CF50210C19736948C8FA65E91DE))
    ROM_CONTINUE(0x0000, 0x4000)
	ROM_LOAD ( "l3kdisk.rom", 0x8500, 0x0100, CRC(2D4B1584) SHA1(989780B77E100598124DF7B72663E5A31A3339C0))
ROM_END

ROM_START(laser128)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD ( "341-0265-a.chr", 0x0000, 0x1000, BAD_DUMP CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10)) // need to dump real laser rom
	ROM_LOAD ( "341-0265-a.chr", 0x1000, 0x1000, BAD_DUMP CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10)) // need to dump real laser rom

	ROM_REGION(0x8700,"maincpu",0)
	ROM_LOAD("laser128.256", 0x0000, 0x8000, CRC(39E59ED3) SHA1(CBD2F45C923725BFD57F8548E65CC80B13BC18DA))
ROM_END

ROM_START(las128ex)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD ( "341-0265-a.chr", 0x0000, 0x1000, BAD_DUMP CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10)) // need to dump real laser rom
	ROM_LOAD ( "341-0265-a.chr", 0x1000, 0x1000, BAD_DUMP CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10)) // need to dump real laser rom

	ROM_REGION(0x8700,"maincpu",0)
	ROM_LOAD("las128ex.256", 0x0000, 0x8000, CRC(B67C8BA1) SHA1(8BD5F82A501B1CF9D988C7207DA81E514CA254B0))
ROM_END

ROM_START(las128e2)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD ( "341-0265-a.chr", 0x0000, 0x1000, BAD_DUMP CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10)) // need to dump real laser rom
	ROM_LOAD ( "341-0265-a.chr", 0x1000, 0x1000, BAD_DUMP CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10)) // need to dump real laser rom

	ROM_REGION(0x8700,"maincpu",0)
    ROM_LOAD( "laser 128ex2 v6.1 rom.bin", 0x000000, 0x008000, CRC(7f911c90) SHA1(125754c1bd777d4c510f5239b96178c6f2e3236b) )
ROM_END

ROM_START(apple2cp)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD ( "341-0265-a.chr", 0x0000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))
	ROM_LOAD ( "341-0265-a.chr", 0x1000, 0x1000,CRC(2651014d) SHA1(b2b5d87f52693817fc747df087a4aa1ddcdb1f10))

	ROM_REGION(0x8700,"maincpu",0)
	ROM_LOAD("341-0625-a.256", 0x0000, 0x8000, CRC(0b996420) SHA1(1a27ae26966bbafd825d08ad1a24742d3e33557c))
ROM_END

ROM_START(ivelultr)
	ROM_REGION(0x2000,"gfx1",0)
	ROM_LOAD ( "ultra.chr", 0x0000, 0x1000,CRC(fed62c85) SHA1(479fb3f38a3f7332cef2e8c4856871afe8dc6017))
	ROM_LOAD ( "ultra.chr", 0x1000, 0x1000,CRC(fed62c85) SHA1(479fb3f38a3f7332cef2e8c4856871afe8dc6017))
	ROM_REGION(0x4700,"maincpu",0)
	ROM_LOAD ( "ultra1.002", 0x1000, 0x1000, CRC(392170ff) SHA1(f065104ab5a476321d56d802f1a174afabff9dbd))
	ROM_LOAD ( "ultra1.001", 0x2000, 0x1000, CRC(671b8c2e) SHA1(c1e4b1477620c9292f2c2b25df55965eef445dee))
	ROM_LOAD ( "ultra2.bin", 0x3000, 0x1000, CRC(1ac1e17e) SHA1(a5b8adec37da91970c303905b5e2c4d1b715ee4e))
ROM_END

ROM_START(space84)
	ROM_REGION(0x2000,"gfx1",0)
    ROM_LOAD( "space 84 mobo chargen.bin", 0x0000, 0x2000, CRC(ceb98990) SHA1(8b2758da611bcfdd3d144edabc63ef1df2ca787b) )

	ROM_REGION(0x4700,"maincpu",0)
	ROM_LOAD ( "341-0011.d0",  0x1000, 0x0800, CRC(6f05f949) SHA1(0287ebcef2c1ce11dc71be15a99d2d7e0e128b1e))
	ROM_LOAD ( "341-0012.d8",  0x1800, 0x0800, CRC(1f08087c) SHA1(a75ce5aab6401355bf1ab01b04e4946a424879b5))
	ROM_LOAD ( "341-0013.e0",  0x2000, 0x0800, CRC(2b8d9a89) SHA1(8d82a1da63224859bd619005fab62c4714b25dd7))
	ROM_LOAD ( "341-0014.e8",  0x2800, 0x0800, CRC(5719871a) SHA1(37501be96d36d041667c15d63e0c1eff2f7dd4e9))
    ROM_LOAD( "space84_f.bin", 0x3000, 0x1000, CRC(4e741069) SHA1(ca1f16da9fb40e966ee4a899964cd6a7e140ab50))
ROM_END

/*    YEAR  NAME      PARENT    COMPAT    MACHINE      INPUT     INIT      COMPANY            FULLNAME */
COMP( 1977, apple2,   0,        0,        apple2,      apple2, driver_device,   0,        "Apple Computer",    "Apple ][", GAME_SUPPORTS_SAVE )
COMP( 1979, apple2p,  apple2,   0,        apple2p,	   apple2p, driver_device,  0,        "Apple Computer",    "Apple ][+", GAME_SUPPORTS_SAVE )
COMP( 1982, prav82,   apple2,   0,        apple2p,	   apple2p, driver_device,  0,        "Pravetz",           "Pravetz 82", GAME_SUPPORTS_SAVE )
COMP( 1985, prav8m,   apple2,   0,        apple2p,	   apple2p, driver_device,  0,        "Pravetz",           "Pravetz 8M", GAME_SUPPORTS_SAVE )
COMP( 1980, apple2jp, apple2,   0,        apple2p,	   apple2p, driver_device,  0,        "Apple Computer",    "Apple ][j+", GAME_SUPPORTS_SAVE )
COMP( 1982, ace100,   apple2,   0,        apple2,	   apple2e, driver_device,  0,        "Franklin Computer", "Franklin Ace 100", GAME_SUPPORTS_SAVE )
COMP( 1983, apple2e,  0,        apple2,	  apple2e,	   apple2e, driver_device,  0,        "Apple Computer",    "Apple //e", GAME_SUPPORTS_SAVE )
COMP( 1983, mprof3,   apple2e,  0,        mprof3,	   apple2e, driver_device,  0,        "Multitech",         "Microprofessor III", GAME_SUPPORTS_SAVE )
COMP( 1985, apple2ee, apple2e,  0,        apple2ee,	   apple2e, driver_device,  0,        "Apple Computer",    "Apple //e (enhanced)", GAME_SUPPORTS_SAVE )
COMP( 1987, apple2ep, apple2e,  0,        apple2ep,	   apple2ep, driver_device, 0,        "Apple Computer",    "Apple //e (Platinum)", GAME_SUPPORTS_SAVE )
COMP( 1984, apple2c,  0,        apple2,	  apple2c,	   apple2e, driver_device,  0,        "Apple Computer",    "Apple //c" , GAME_SUPPORTS_SAVE )
COMP( 1984, tk2000,   apple2c,  0,  	  tk2000,	   apple2e, driver_device,  0,        "Microdigital",      "TK2000" , GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
COMP( 1989, prav8c,   apple2c,  0,        apple2c,	   apple2e, driver_device,  0,        "Pravetz",           "Pravetz 8C", GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
COMP( 1983, las3000,  apple2,   0,        apple2p,	   apple2p, driver_device,  0,        "Video Technology",  "Laser 3000",	GAME_NOT_WORKING )
COMP( 1987, laser128, apple2c,  0,        laser128,	   apple2e, driver_device,  0,        "Video Technology",  "Laser 128 (version 4.2)", GAME_NOT_WORKING )
COMP( 1988, las128ex, apple2c,  0,        laser128,	   apple2e, driver_device,  0,        "Video Technology",  "Laser 128ex (version 4.5)", GAME_NOT_WORKING )
COMP( 1985, apple2c0, apple2c,  0,        apple2c_iwm, apple2e, driver_device,  0,        "Apple Computer",    "Apple //c (UniDisk 3.5)", GAME_SUPPORTS_SAVE )
COMP( 1986, apple2c3, apple2c,  0,        apple2c_iwm, apple2e, driver_device,  0,        "Apple Computer",    "Apple //c (Original Memory Expansion)", GAME_SUPPORTS_SAVE )
COMP( 1986, apple2c4, apple2c,  0,        apple2c_iwm, apple2e, driver_device,  0,        "Apple Computer",    "Apple //c (rev 4)", GAME_NOT_WORKING )
COMP( 1988, apple2cp, apple2c,  0,        apple2c_iwm, apple2e, driver_device,  0,        "Apple Computer",    "Apple //c Plus", GAME_SUPPORTS_SAVE )
COMP( 1984, ivelultr, apple2,   0,        apple2p,     apple2p, driver_device,  0,        "Ivasim",            "Ivel Ultra", GAME_SUPPORTS_SAVE )
COMP( 1983, agat7,    apple2,   0,        apple2p,     apple2p, driver_device,  0,        "Agat",              "Agat-7", GAME_NOT_WORKING) // disk controller ROM JSRs to $FCA8 which is a delay on apple II, illegal instruction crash here :(
COMP( 1984, agat9,    apple2,   0,        apple2p,     apple2p, driver_device,  0,        "Agat",              "Agat-9", GAME_NOT_WORKING)
COMP( 1985, space84,  apple2,   0,        space84,	   apple2p, driver_device,  0,        "ComputerTechnik/IBS",  "Space 84",	GAME_NOT_WORKING )

