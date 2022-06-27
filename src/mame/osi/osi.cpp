// license:BSD-3-Clause
// copyright-holders:Curt Coder, Robbbert, Wilbert Pol
/*

Ohio Scientific Superboard II Model 600

PCB Layout
----------

OHIO SCIENTIFIC MODEL 600 REV D

|-------------------------------------------------------------------------------------------|
|                   2114    2114                                                            |
|                                   LS138               CN1                                 |
|       IC1         2114    2114                                                            |
|                                   LS04            LS75    LS75                            |
|                   2114    2114                                                            |
|                                   LS138                                                   |
|                   2114    2114                    LS125   LS125                           |
|                                   LS138                                                   |
| -                 2114    2114                    8T28    8T28                            |
| |                                                                                         |
|C|                 2114    2114                                                            |
|N|                                                                                         |
|3|                 2114    2114                        6502                                |
| |                                                                                         |
| -                 2114    2114                                                            |
|                                                                                           |
|                           2114    8T28                ROM0                                |
|           LS163   LS157                   LS174                                           |
|                           2114    8T28                ROM1                                |
|   CA3130  LS163   LS157                   LS02                                            |
|                           2114    8T28                ROM2                                |
|   LS14    LS163   LS157                   LS04                                            |
|                                   SW1                 ROM3                                |
|   7417    7404    LS20    ROM5    7408    LS139                                           |
|                                                       ROM4                                |
| - 74123   7474    LS163   LS165   7474                                                    |
|C|                                                                                         |
|N|         7476    7400    LS157   LS93    LS04        6850                                |
|2|                                                                                         |
| - 7403    74123   3.7MHz  LS86    LS163   LS20                                    CN4     |
|-------------------------------------------------------------------------------------------|

Notes:
    All IC's shown.

    ROM0-5  -
    6502    -
    6850    - Asynchronous Communications Interface Adapter
    8T28    - 4-Bit Bidirectional Bus Transceiver
    CA3130  - Operational Amplifier
    CN1     - OSI-48 bus connector
    CN2     -
    CN3     -
    CN4     - sound connector
    SW1     -

*/

/*

Compukit UK101

PCB Layout
----------

|-------------------------------------------------------------------------------------------|
|                   2114    2114                                                            |
|                                   LS138               CN1                                 |
|       IC1         2114    2114                                                            |
|                                   7404            LS75    LS75                            |
|                   2114    2114                                                            |
|                                   LS138                                                   |
|                   2114    2114                    LS125   LS125                           |
|                                   LS138                                                   |
| -                 2114*   2114*                   8T28*   8T28*                           |
| |                                                                                         |
|C|                 2114*   2114*                                                           |
|N|     CN4                                                                                 |
|3|                 2114*   2114*                       6502                                |
| |                                                                                         |
| -                 2114*   2114*                                                           |
|                                                                                           |
|                           2114    8T28                ROM0                                |
|           74163   LS157                                                                   |
|                           2114    8T28                ROM1                                |
|   IC66'   74163   LS157                   7402                                            |
|                                                       ROM2                                |
|   IC67'   74163   LS157                   7404                                            |
|                                                       ROM3                                |
|   IC68'   7404    7420    ROM5            LS139                                           |
|                                                       ROM4                                |
| - LS123   7474    74163   74165   LS123                                                   |
|C|                                                                                         |
|N|         7476    7400            7493    7404        6850                                |
|2|                                                                                         |
| - 7403    LS123   8MHz            74163   LS20                                            |
|-------------------------------------------------------------------------------------------|

Notes:
    All IC's shown.

    ROM0-5  -
    6502    -
    6850    - Asynchronous Communications Interface Adapter
    8T28    - 4-Bit Bidirectional Bus Transceiver
    CN1     - OSI-48 Bus Connector
    CN2     -
    CN3     -
    CN4     - UHF Modulator Output
    *       - present when 8KB of RAM installed
    '       - present when cassette option installed

*/

/*

Ohio Scientific Single Sided Floppy Interface

PCB Layout
----------

OSI 470 REV B

|-------------------------------------------------------------------|
|                                                                   |
|                                               8T26    8T26        |
|                                                                   |
|       7417                                                        |
|                                                                   |
|                       6820                6850        7400  U2C   |
|       7417                                                        |
|                                                                   |
|                                                                   |
|       7417                                                7404    |
|                                                                   |
|                                                                   |
|       7417                                                        |
|                                                                   |
|                   7400    7404    74123   74123   7410            |
|   4MHz                                                            |
|                                                                   |
|                                                                   |
|   7400    7493    74390   74390   74390   7404    7430    7404    |
|                                                                   |
|                                                                   |
|                   U6A                                             |
|                                                                   |
|-------------------------------------------------------------------|

Notes:
    All IC's shown.

    6850    - Asynchronous Communications Interface Adapter
    6820    - Peripheral Interface Adapter
    8T26    - 4-Bit Bidirectional Bus Transceiver
    U2C     - PROTO?
    U6A     - PROTO?


TODO:

    - floppy PIA is actually a 6820
    - break key
    - power on reset
    - Superboard II revisions A/C/D
    - uk101 medium resolution graphics
    - uk101 ay-3-8910 sound
    - faster cassette
    - floppy
    - need floppies to test with
    - support for BAS files and other formats
    - wemon?
    - rs232

Notes added 2012-05-11 [Robbbert]
    General
    - Added F1 key to toggle the sound on 'sb2m600b', to silence the awful screech.
    - Added F3 key to simulate the RESET / BREAK key on the real keyboard.
    - These machines only uses uppercase input. Shift-Lock must be on at all times.
    - At boot you get a screen DCWM or similar:
      D = boot from floppy
      C = Cold boot & start BASIC
      W = Warm boot (don't use after turning on the machine) jump to 0000.
      M = Monitor (only options are to read/modify memory, Go, Load a tape).
    - The corrupt error messages in Basic are normal and are documented in the user
      manual. It's not a bug.

    Compukit UK101
    - The Monitor ROM that had always been there is for a 32x32 screen. I assume it
      was the prototype referred to in the first Practical Electronics construction
      article. Also, the keyboard doesn't work in Basic.
    - I found a working rom in another emulator, this runs fine on the 64x16 screen.
    - But, the proper rom that came with the kit needs to be found.
    - The proper rom (and the prototype) allow you to boot from floppy, but this is
      not normally fitted. It appears that it would work the same as in the other
      systems in this driver.

Notes added 2013-01-20
      Added a modified basic rom, which fixes the garbage collection problem.
      Try the following program with -bios 0 and -bios 1. It will work only
      with bios 1. You can copy/paste this code, but make sure you include the
      trailing blank line.

10 DIM A$(3)
RUN
PRINT FRE(0)


Keyboard notes and BTANBs:

- Keyboards on all models are identical, except on UK101 where ^ replaces LINE FEED.

- When run for the first time, the keyboard is in lowercase. Shift will correctly
  select uppercase. Special keys do weird things, but come good when Shift pressed.
  Since all input MUST be uppercase, the first thing to do is press Capslock (which
  is SHIFT LOCK on the real machine). This is saved in your cfg file on exit.

- After that, uppercase works correctly, as do the other keys. Pressing Shift will now
  do odd things to the letters, and they become random symbols.

- Natural keyboard is set up to take advantage of these oddities and get the maximum
  symbols available, however lowercase is not available. Natural keyboard will only
  work correctly when the SHIFT LOCK is engaged.

*/

#include "emu.h"
#include "osi.h"

#include "machine/clock.h"
#include "screen.h"
#include "speaker.h"


/* Sound */

static const discrete_dac_r1_ladder osi600_dac =
{
	4,          // size of ladder
	{ 180, 180, 180, 180, 0, 0, 0, 0 }, // R68, R69, R70, R71
	5,          // 5V
	RES_K(1),   // R67
	0,          // no rGnd
	0           // no cFilter
};

static DISCRETE_SOUND_START( osi600_discrete_interface )
	DISCRETE_INPUT_DATA(NODE_01)

	DISCRETE_DAC_R1(NODE_02, NODE_01, DEFAULT_TTL_V_LOGIC_1, &osi600_dac)
	DISCRETE_CRFILTER(NODE_03, NODE_02, (int)(1.0/(1.0/RES_K(1)+1.0/180+1.0/180+1.0/180+1.0/180)), CAP_U(0.1))
	DISCRETE_OUTPUT(NODE_03, 100)
	DISCRETE_GAIN(NODE_04, NODE_03, 32767.0/5)
	DISCRETE_OUTPUT(NODE_04, 100)
DISCRETE_SOUND_END

#if 0
static const discrete_dac_r1_ladder osi600c_dac =
{
	8,          // size of ladder
	{ RES_K(68), RES_K(33), RES_K(16), RES_K(8.2), RES_K(3.9), RES_K(2), RES_K(1), 510 }, // R73, R71, R70, R67, R68, R69, R75, R74
	5,          // 5V
	510,        // R86
	0,          // no rGnd
	CAP_U(33)   // C63
};
#endif

static DISCRETE_SOUND_START( osi600c_discrete_interface )
	DISCRETE_INPUT_DATA(NODE_01)
	DISCRETE_INPUT_LOGIC(NODE_10)

	DISCRETE_DAC_R1(NODE_02, NODE_01, DEFAULT_TTL_V_LOGIC_1, &osi600_dac)
	DISCRETE_ONOFF(NODE_03, NODE_10, NODE_02)
	DISCRETE_OUTPUT(NODE_03, 100)
DISCRETE_SOUND_END

/* Keyboard */

uint8_t sb2m600_state::keyboard_r()
{
	if (m_io_reset->read())
		m_maincpu->reset();

	u8 i, data = 0xff;

	for (i = 0; i < 8; i++)
		if (!BIT(m_keylatch, i))
			data &= m_io_keyboard[i]->read();

	return data;
}

void sb2m600_state::keyboard_w(uint8_t data)
{
	m_keylatch = data;

	if (m_io_sound->read())
		m_discrete->write(NODE_01, (data >> 2) & 0x0f);
}

void uk101_state::keyboard_w(uint8_t data)
{
	m_keylatch = data;
}

void sb2m600_state::ctrl_w(uint8_t data)
{
	/*

	    bit     signal          description

	    0       _32             screen size (0=32x32, 1=64x16)
	    1       COLOR EN        color enable
	    2       BK0
	    3       BK1
	    4       DAC DISABLE     DAC sound enable
	    5
	    6
	    7

	*/

	m_32 = BIT(data, 0);
	m_coloren = BIT(data, 1);

	m_discrete->write(NODE_10, BIT(data, 4));
}

void c1p_state::osi630_ctrl_w(uint8_t data)
{
	/*

	    bit     description

	    0       AC control enable
	    1       tone generator enable
	    2       modem select (0 = printer, 1 = modem)
	    3
	    4
	    5
	    6
	    7

	*/

	m_beeper->set_state(BIT(data, 1));
}

void c1p_state::osi630_sound_w(uint8_t data)
{
	if (data != 0)
		m_beeper->set_clock(49152 / data);
}

/* Disk Drive */

/*
    C000 FLOPIN         FLOPPY DISK STATUS PORT

    BIT FUNCTION
    0   DRIVE 0 READY (0 IF READY)
    1   TRACK 0 (0 IF AT TRACK 0)
    2   FAULT (0 IF FAULT)
    3
    4   DRIVE 1 READY (0 IF READY)
    5   WRITE PROTECT (0 IF WRITE PROTECT)
    6   DRIVE SELECT (1 = A OR C, 0 = B OR D)
    7   INDEX (0 IF AT INDEX HOLE)

    C002 FLOPOT         FLOPPY DISK CONTROL PORT

    BIT FUNCTION
    0   WRITE ENABLE (0 ALLOWS WRITING)
    1   ERASE ENABLE (0 ALLOWS ERASING)
        ERASE ENABLE IS ON 200us AFTER WRITE IS ON
        ERASE ENABLE IS OFF 530us AFTER WRITE IS OFF
    2   STEP BIT : INDICATES DIRECTION OF STEP (WAIT 10us FIRST)
        0 INDICATES STEP TOWARD 76
        1 INDICATES STEP TOWARD 0
    3   STEP (TRANSITION FROM 1 TO 0)
        MUST HOLD AT LEAST 10us, MIN 8us BETWEEN
    4   FAULT RESET (0 RESETS)
    5   SIDE SELECT (1 = A OR B, 0 = C OR D)
    6   LOW CURRENT (0 FOR TRKS 43-76, 1 FOR TRKS 0-42)
    7   HEAD LOAD (0 TO LOAD : MUST WAIT 40ms AFTER)

    C010 ACIA           DISK CONTROLLER ACIA STATUS PORT
    C011 ACIAIO         DISK CONTROLLER ACIA I/O PORT
*/

void sb2m600_state::floppy_index_callback(floppy_image_device *floppy, int state)
{
	m_fdc_index = state;
}

uint8_t c1pmf_state::osi470_pia_pa_r()
{
	/*

	    bit     description

	    0       _READY DRIVE 1
	    1       _TRACK 00
	    2       _FAULT
	    3       _SECTOR
	    4       _READY DRIVE 2
	    5       _WRITE PROTECT
	    6
	    7       _INDEX

	*/

	return (m_fdc_index << 7);
}

void c1pmf_state::osi470_pia_pa_w(uint8_t data)
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5
	    6       drive select
	    7

	*/
}

void c1pmf_state::osi470_pia_pb_w(uint8_t data)
{
	/*

	    bit     description

	    0       _WRITE ENABLE
	    1       _ERASE ENABLE
	    2       _STEP IN
	    3       _STEP
	    4       _FAULT RESET
	    5       side select
	    6       _LOW CURRENT
	    7       _HEAD LOAD

	*/
}

WRITE_LINE_MEMBER( c1pmf_state::osi470_pia_cb2_w )
{
}

/* Memory Maps */

void sb2m600_state::osi600_mem(address_map &map)
{
	map(0x0000, 0x1fff).bankrw("bank1");
	map(0xa000, 0xbfff).rom();
	map(0xd000, 0xd3ff).ram().share("video_ram");
	map(0xdf00, 0xdf00).rw(FUNC(sb2m600_state::keyboard_r), FUNC(sb2m600_state::keyboard_w));
	map(0xf000, 0xf001).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0xf800, 0xffff).rom();
}

void uk101_state::uk101_mem(address_map &map)
{
	map(0x0000, 0x1fff).bankrw("bank1");
	map(0xa000, 0xbfff).rom();
	map(0xd000, 0xd3ff).ram().share("video_ram");
	map(0xd400, 0xd7ff).noprw();  // bios sets this to spaces at boot
	map(0xdc00, 0xdfff).r(FUNC(uk101_state::keyboard_r)).w(FUNC(uk101_state::keyboard_w));
	map(0xf000, 0xf001).mirror(0x00fe).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0xf800, 0xffff).rom();
}

void c1p_state::c1p_mem(address_map &map)
{
	map(0x0000, 0x4fff).bankrw("bank1");
	map(0xa000, 0xbfff).rom();
	map(0xc704, 0xc707).rw("pia_1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xc708, 0xc70b).rw("pia_2", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xc70c, 0xc70f).rw("pia_3", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xd000, 0xd3ff).ram().share("video_ram");
	map(0xd400, 0xd7ff).ram().share("color_ram");
	map(0xd800, 0xd800).w(FUNC(c1p_state::ctrl_w));
	map(0xdf00, 0xdf00).rw(FUNC(c1p_state::keyboard_r), FUNC(c1p_state::keyboard_w));
	map(0xf000, 0xf001).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0xf7c0, 0xf7c0).w(FUNC(c1p_state::osi630_sound_w));
	map(0xf7e0, 0xf7e0).w(FUNC(c1p_state::osi630_ctrl_w));
	map(0xf800, 0xffff).rom();
}

void c1pmf_state::c1pmf_mem(address_map &map)
{
	map(0x0000, 0x4fff).bankrw("bank1");
	map(0xa000, 0xbfff).rom();
	map(0xc000, 0xc003).rw("pia_0", FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // FDC
	map(0xc010, 0xc011).rw("acia_1", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0xc704, 0xc707).rw("pia_1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xc708, 0xc70b).rw("pia_2", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xc70c, 0xc70f).rw("pia_3", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xd000, 0xd3ff).ram().share("video_ram");
	map(0xd400, 0xd7ff).ram().share("color_ram");
	map(0xd800, 0xd800).w(FUNC(c1pmf_state::ctrl_w));
	map(0xdf00, 0xdf00).rw(FUNC(c1pmf_state::keyboard_r), FUNC(c1pmf_state::keyboard_w));
	map(0xf000, 0xf001).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0xf7c0, 0xf7c0).w(FUNC(c1pmf_state::osi630_sound_w));
	map(0xf7e0, 0xf7e0).w(FUNC(c1pmf_state::osi630_ctrl_w));
	map(0xf800, 0xffff).rom();
}

/* Input Ports */

static INPUT_PORTS_START( osi600 )
	PORT_START("ROW0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SHIFT LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RIGHT SHIFT") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LEFT SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_TAB) PORT_CHAR(27)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("REPEAT") PORT_CODE(KEYCODE_BACKSLASH)

	PORT_START("ROW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('@')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')

	PORT_START("ROW2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR(']')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('^')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('X')

	PORT_START("ROW3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('[')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S')

	PORT_START("ROW4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('W')

	PORT_START("ROW5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LINE FEED") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(10)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('_')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('\\')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')

	PORT_START("ROW6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RUB OUT") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')

	PORT_START("ROW7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')

	PORT_START("Sound")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) PORT_TOGGLE

	PORT_START("Reset")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RESET") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
INPUT_PORTS_END

static INPUT_PORTS_START( uk101 )
	PORT_INCLUDE(osi600)
	PORT_MODIFY("ROW5")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^") PORT_CODE(KEYCODE_BACKSLASH)
INPUT_PORTS_END

/* Machine Start */

TIMER_DEVICE_CALLBACK_MEMBER( sb2m600_state::kansas_w )
{
	m_cass_data[3]++;

	if (m_cassbit != m_cassold)
	{
		m_cass_data[3] = 0;
		m_cassold = m_cassbit;
	}

	if (m_cassbit)
		m_cass->output(BIT(m_cass_data[3], 0) ? -1.0 : +1.0); // 2400Hz
	else
		m_cass->output(BIT(m_cass_data[3], 1) ? -1.0 : +1.0); // 1200Hz
}

TIMER_DEVICE_CALLBACK_MEMBER( sb2m600_state::kansas_r)
{
	/* cassette - turn 1200/2400Hz to a bit */
	m_cass_data[1]++;
	uint8_t cass_ws = (m_cass->input() > +0.03) ? 1 : 0;

	if (cass_ws != m_cass_data[0])
	{
		m_cass_data[0] = cass_ws;
		m_acia->write_rxd((m_cass_data[1] < 12) ? 1 : 0);
		m_cass_data[1] = 0;
	}
}

void sb2m600_state::machine_start()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	/* configure RAM banking */
	membank("bank1")->configure_entry(0, memregion(M6502_TAG)->base());
	membank("bank1")->set_entry(0);

	switch (m_ram->size())
	{
	case 4*1024:
		program.install_readwrite_bank(0x0000, 0x0fff, membank("bank1"));
		program.unmap_readwrite(0x1000, 0x1fff);
		break;

	case 8*1024:
		program.install_readwrite_bank(0x0000, 0x1fff, membank("bank1"));
		break;
	}

	/* register for state saving */
	save_item(NAME(m_keylatch));
	save_pointer(NAME(m_video_ram.target()), OSI600_VIDEORAM_SIZE);
	save_item(NAME(m_cass_data));
	save_item(NAME(m_cassbit));
	save_item(NAME(m_cassold));
	save_item(NAME(m_fdc_index));
	save_item(NAME(m_32));
	save_item(NAME(m_coloren));
}

void c1p_state::machine_start()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	/* configure RAM banking */
	membank("bank1")->configure_entry(0, memregion(M6502_TAG)->base());
	membank("bank1")->set_entry(0);

	switch (m_ram->size())
	{
	case 8*1024:
		program.install_readwrite_bank(0x0000, 0x1fff, membank("bank1"));
		program.unmap_readwrite(0x2000, 0x4fff);
		break;

	case 20*1024:
		program.install_readwrite_bank(0x0000, 0x4fff, membank("bank1"));
		break;
	}

	/* register for state saving */
	save_item(NAME(m_cass_data));
	save_item(NAME(m_cassbit));
	save_item(NAME(m_cassold));
	save_item(NAME(m_fdc_index));
	save_item(NAME(m_keylatch));
	save_item(NAME(m_32));
	save_item(NAME(m_coloren));
	save_pointer(NAME(m_video_ram.target()), OSI600_VIDEORAM_SIZE);
	save_pointer(NAME(m_color_ram.target()), OSI630_COLORRAM_SIZE);
}

void c1pmf_state::machine_start()
{
	c1p_state::machine_start();

	// drive select logic missing
	if (m_floppy0->get_device())
		m_floppy0->get_device()->setup_index_pulse_cb(floppy_image_device::index_pulse_cb(&c1pmf_state::floppy_index_callback, this));
}

// disk format: 1 head, 36 tracks (? - manual displays a directory listing with 40 tracks),
// 10 sectors, 256 byte sector length, first sector id 0
static void osi_floppies(device_slot_interface &device)
{
	device.option_add("ssdd", FLOPPY_525_SSDD);
}

/* F4 Character Displayer */
static const gfx_layout osi_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( gfx_osi )
	GFXDECODE_ENTRY( "chargen", 0x0000, osi_charlayout, 0, 1 )
GFXDECODE_END

/* Machine Drivers */

void sb2m600_state::osi600(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, X1/4); // .98304 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &sb2m600_state::osi600_mem);

	/* video hardware */
	osi600_video(config);
	GFXDECODE(config, "gfxdecode", "palette", gfx_osi);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	DISCRETE(config, m_discrete);
	m_discrete->set_intf(osi600_discrete_interface);
	m_discrete->add_route(ALL_OUTPUTS, "mono", 0.50);

	/* cassette ACIA */
	ACIA6850(config, m_acia, 0);
	m_acia->txd_handler().set([this] (bool state) { m_cassbit = state; });

	clock_device &acia_clock(CLOCK(config, "acia_clock", 4'800)); // 300 baud x 16(divider) = 4800
	acia_clock.signal_handler().set(m_acia, FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append(m_acia, FUNC(acia6850_device::write_rxc));

	/* cassette */
	CASSETTE(config, m_cass);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
	TIMER(config, "kansas_w").configure_periodic(FUNC(sb2m600_state::kansas_w), attotime::from_hz(4800)); // cass write
	TIMER(config, "kansas_r").configure_periodic(FUNC(sb2m600_state::kansas_r), attotime::from_hz(40000)); // cass read

	/* internal ram */
	RAM(config, m_ram);
	m_ram->set_default_size("4K");
	m_ram->set_extra_options("8K");
}

void uk101_state::uk101(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, UK101_X1/8); // 1 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &uk101_state::uk101_mem);

	/* video hardware */
	uk101_video(config);
	GFXDECODE(config, "gfxdecode", "palette", gfx_osi);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	/* cassette ACIA */
	ACIA6850(config, m_acia, 0);
	m_acia->txd_handler().set([this] (bool state) { m_cassbit = state; });

	clock_device &acia_clock(CLOCK(config, "acia_clock", 4'800)); // 300 baud x 16(divider) = 4800
	acia_clock.signal_handler().set(m_acia, FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append(m_acia, FUNC(acia6850_device::write_rxc));

	/* cassette */
	CASSETTE(config, m_cass);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
	TIMER(config, "kansas_w").configure_periodic(FUNC(uk101_state::kansas_w), attotime::from_hz(4800)); // cass write
	TIMER(config, "kansas_r").configure_periodic(FUNC(uk101_state::kansas_r), attotime::from_hz(40000)); // cass read

	/* internal ram */
	RAM(config, m_ram);
	m_ram->set_default_size("4K");
	m_ram->set_extra_options("8K");
}

void c1p_state::c1p(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, X1/4); // .98304 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &c1p_state::c1p_mem);

	/* video hardware */
	osi630_video(config);
	GFXDECODE(config, "gfxdecode", "palette", gfx_osi);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	DISCRETE(config, m_discrete);
	m_discrete->set_intf(osi600c_discrete_interface);
	m_discrete->add_route(ALL_OUTPUTS, "mono", 0.50);
	BEEP(config, "beeper", 300).add_route(ALL_OUTPUTS, "mono", 0.50);
	TIMER(config, m_beep_timer).configure_generic(FUNC(c1p_state::beep_timer));

	PIA6821(config, "pia_1", 0);
	PIA6821(config, "pia_2", 0);
	PIA6821(config, "pia_3", 0);

	/* cassette ACIA */
	ACIA6850(config, m_acia, 0);
	m_acia->txd_handler().set([this] (bool state) { m_cassbit = state; });

	clock_device &acia_clock(CLOCK(config, "acia_clock", 4'800)); // 300 baud x 16(divider) = 4800
	acia_clock.signal_handler().set(m_acia, FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append(m_acia, FUNC(acia6850_device::write_rxc));

	/* cassette */
	CASSETTE(config, m_cass);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
	TIMER(config, "kansas_w").configure_periodic(FUNC(c1p_state::kansas_w), attotime::from_hz(4800)); // cass write
	TIMER(config, "kansas_r").configure_periodic(FUNC(c1p_state::kansas_r), attotime::from_hz(40000)); // cass read

	/* internal ram */
	RAM(config, m_ram);
	m_ram->set_default_size("8K");
	m_ram->set_extra_options("20K");
}

void c1pmf_state::c1pmf(machine_config &config)
{
	c1p(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &c1pmf_state::c1pmf_mem);

	pia6821_device &pia0(PIA6821(config, "pia_0", 0));
	pia0.readpa_handler().set(FUNC(c1pmf_state::osi470_pia_pa_r));
	pia0.writepa_handler().set(FUNC(c1pmf_state::osi470_pia_pa_w));
	pia0.writepb_handler().set(FUNC(c1pmf_state::osi470_pia_pb_w));
	pia0.cb2_handler().set(FUNC(c1pmf_state::osi470_pia_cb2_w));

	/* floppy ACIA */
	ACIA6850(config, "acia_1", 0);

	CLOCK(config, "floppy_clock", XTAL(4'000'000)/8).signal_handler().set("acia_1", FUNC(acia6850_device::write_txc)); // 250 kHz

	FLOPPY_CONNECTOR(config, "floppy0", osi_floppies, "ssdd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, "floppy1", osi_floppies, nullptr,   floppy_image_device::default_mfm_floppy_formats);

	/* internal ram */
	m_ram->set_default_size("20K");
}

/* ROMs */



ROM_START( sb2m600b )
	ROM_REGION( 0x10000, M6502_TAG, 0 )
	ROM_LOAD( "basus01.u9",  0xa000, 0x0800, CRC(f4f5dec0) SHA1(b41bf24b4470b6e969d32fe48d604637276f846e) )
	ROM_LOAD( "basus02.u10", 0xa800, 0x0800, CRC(0039ef6a) SHA1(1397f0dc170c16c8e0c7d02e63099e986e86385b) )
	ROM_LOAD( "basus04.u12", 0xb800, 0x0800, CRC(8ee6030e) SHA1(71f210163e4268cba2dd78a97c4d8f5dcebf980e) )
	ROM_LOAD( "monde01.u13", 0xf800, 0x0800, CRC(95a44d2e) SHA1(4a0241c4015b94c436d0f0f58b3dd9d5207cd847) ) // also known as syn600.rom
	ROM_SYSTEM_BIOS(0, "original", "Original")
	ROMX_LOAD("basus03.u11", 0xb000, 0x0800, CRC(ca25f8c1) SHA1(f5e8ee93a5e0656657d0cc60ef44e8a24b8b0a80), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "fixed", "Fixed")
	ROMX_LOAD( "basic3.rom", 0xb000, 0x0800, CRC(ac37d575) SHA1(11407eb24d1ba7afb889b7677c987e8be1a61aab), ROM_BIOS(1) )

	ROM_REGION( 0x0800, "chargen",0)
	ROM_LOAD( "chgsup2.u41", 0x0000, 0x0800, CRC(735f5e0a) SHA1(87c6271497c5b00a974d905766e91bb965180594) ) // see below, is this the same rom, but on a different pcb/form factor?
	// Label: "<Synertek Logo> 7837E // C28339M // CARGENV1.0"; This mask ROM is on the OSI 540 PCB, as seen at http://www.classic-computers.org.nz/blog/images/2014-06-14-540-board.jpg at location ?E3?

	ROM_REGION( 0x0800, "user1",0)
	// Another bios rom
	ROM_LOAD( "c2 c4 synmon.rom", 0x0000, 0x0800, CRC(03cdbcc5) SHA1(5426ae14522ef485b6089472011db0ae1d192630) )
ROM_END

// same roms are used in Challenger 2P and 4P
#define rom_c1p rom_sb2m600b
#define rom_c1pmf rom_sb2m600b

ROM_START( uk101 )
	ROM_REGION( 0x10000, M6502_TAG, 0 )
	ROM_LOAD( "basuk01.ic9",   0xa000, 0x0800, CRC(9d3caa92) SHA1(b2c3d1af0c4f3cead1dbd44aaf5a11680880f772) )
	ROM_LOAD( "basus02.ic10",  0xa800, 0x0800, CRC(0039ef6a) SHA1(1397f0dc170c16c8e0c7d02e63099e986e86385b) )
	ROM_LOAD( "basuk03.ic11",  0xb000, 0x0800, CRC(0d011242) SHA1(54bd33522a5d1991086eeeff3a4f73c026be45b6) )
	ROM_LOAD( "basuk04.ic12",  0xb800, 0x0800, CRC(667223e8) SHA1(dca78be4b98317413376d69119942d692e39575a) )
	// This monitor came from another emulator and works well on the 64x16 screen
	ROM_SYSTEM_BIOS(0, "final", "64x16 screen final? rom")
	ROMX_LOAD( "monuk02.ic13",  0xf800, 0x0800, CRC(e5b7028d) SHA1(74f0934014fdf83d33c8d3579e562b53c0683270), ROM_BIOS(0) )
	// This monitor is for a 32x32 screen, and could be the prototype referred to in Practical Electronics
	ROM_SYSTEM_BIOS(1, "proto", "32x32 screen proto? rom")
	ROMX_LOAD( "monuk02_alt.ic13",  0xf800, 0x0800, CRC(04ac5822) SHA1(2bbbcd0ca18103fd68afcf64a7483653b925d83e), ROM_BIOS(1) )

	ROM_REGION( 0x800, "chargen", 0 )
	ROM_LOAD( "chguk101.ic41", 0x0000, 0x0800, CRC(fce2c84a) SHA1(baa66a7a48e4d62282671ef53abfaf450b888b70) )
ROM_END

/* Driver Initialization */

TIMER_DEVICE_CALLBACK_MEMBER(c1p_state::beep_timer)
{
	m_beeper->set_state(0);
	m_beeper->set_clock(300);
}

void c1p_state::init_c1p()
{
	m_beep_timer->adjust(attotime::zero);
}


/* System Drivers */

//    YEAR  NAME      PARENT    COMPAT  MACHINE  INPUT   CLASS          INIT        COMPANY            FULLNAME                            FLAGS
COMP( 1978, sb2m600b, 0,        0,      osi600,  osi600, sb2m600_state, empty_init, "Ohio Scientific", "Superboard II Model 600 (Rev. B)", MACHINE_SUPPORTS_SAVE )
//COMP( 1980, sb2m600c, 0,        0,      osi600c, osi600, sb2m600_state, empty_init, "Ohio Scientific", "Superboard II Model 600 (Rev. C)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1980, c1p,      sb2m600b, 0,      c1p,     osi600, c1p_state,     init_c1p,   "Ohio Scientific", "Challenger 1P Series 2",           MACHINE_SUPPORTS_SAVE )
COMP( 1980, c1pmf,    sb2m600b, 0,      c1pmf,   osi600, c1pmf_state,   init_c1p,   "Ohio Scientific", "Challenger 1P MF Series 2",        MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1979, uk101,    sb2m600b, 0,      uk101,   uk101,  uk101_state,   empty_init, "Compukit",        "UK101",                            MACHINE_SUPPORTS_SAVE )
