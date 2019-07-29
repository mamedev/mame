// license:GPL-2.0+
// copyright-holders:Kevin Thacker, Robbbert
/******************************************************************************

  Exidy Sorcerer system driver

    The UART controls rs232 and cassette i/o. The chip is a AY-3-1014A or AY-3-1015.


    port fc:
    ========
    input/output:
        uart data

    port fd:
    ========
    input: uart status
        bit 4: parity error (RPE)
        bit 3: framing error (RFE)
        bit 2: over-run (RDP)
        bit 1: data available (RDA)
        bit 0: transmit buffer empty (TPMT)

    output:
        bit 4: no parity (NPB)
        bit 3: parity type (POE)
        bit 2: number of stop bits (NSB)
        bit 1: number of bits per char bit 2 (NDB2)
        bit 0: number of bits per char bit 1 (NDB1)

    port fe:
    ========

    output:

        bit 7: rs232 enable (1=rs232, 0=cassette)
        bit 6: baud rate (1=1200, 0=300)
        bit 5: cassette motor 2
        bit 4: cassette motor 1
        bit 3..0: keyboard line select

    input:
        bit 7..6: parallel control (not emulated)
                7: must be 1 to read data from parallel port via PARIN
                6: must be 1 to send data out of parallel port via PAROUT
        bit 5: vsync
        bit 4..0: keyboard line data

    port ff:
    ========
      parallel port in/out

    -------------------------------------------------------------------------------------

    When cassette is selected, it is connected to the uart data input via the cassette
    interface hardware.

    The cassette interface hardware converts square-wave pulses into bits which the uart receives.

    1. the cassette format: "frequency shift" is converted
    into the uart data format "non-return to zero"

    2. on cassette a 1 data bit is stored as a high frequency
    and a 0 data bit as a low frequency
    - At 1200 baud, a logic 1 is 1 cycle of 1200 Hz and a logic 0 is 1/2 cycle of 600 Hz.
    - At 300 baud, a logic 1 is 8 cycles of 2400 Hz and a logic 0 is 4 cycles of 1200 Hz.

    Attenuation is applied to the signal and the square wave edges are rounded.

    A manchester encoder is used. A flip-flop synchronises input
    data on the positive-edge of the clock pulse.

    Interestingly the data on cassette is stored in xmodem-checksum.


    Due to bugs in the hardware and software of a real Sorcerer, the serial
    interface misbehaves.
    1. Sorcerer I had a hardware problem causing rs232 idle to be a space (+9v)
    instead of mark (-9v). Fixed in Sorcerer II.
    2. When you select a different baud for rs232, it was "remembered" but not
    sent to port fe. It only gets sent when motor on was requested. Motor on is
    only meaningful in a cassette operation.
    3. The monitor software always resets the device to cassette whenever the
    keyboard is scanned, motors altered, or an error occurred.
    4. The above problems make rs232 communication impractical unless you write
    your own routines or create a corrected monitor rom.

    Sound:

    External speaker connected to the parallel port.
    There was a dac you could make instead, this is supported.


    Kevin Thacker [MESS driver]
    Robbbert [Various corrections and additions over the years]

 ******************************************************************************

    The CPU clock speed is 2.106 MHz, which was increased to 4.0 MHz on the last production runs.

    The Sorcerer has a bus connection for S100 equipment. This allows the connection
    of disk drives, provided that suitable driver/boot software is loaded.

*******************************************************************************

Real machines had optional RAM sizes of 8k, 16k, 32k, 48k (officially).
Unofficially, the cart could hold another 8k of static RAM (4x 6116), giving 56k total.

On the back of the machine is a 50-pin expansion port, which could be hooked to the
expansion unit.

Also is a 25-pin parallel port, allowing inwards and outwards communication.
It was often hooked to a printer, a joystick, a music card, or a speaker.

We emulate the printer and the speaker.

Another 25-pin port provided two-way serial communications. Only two speeds are
available - 300 baud and 1200 baud. There is no handshaking.

Other pins on this connector provided for two cassette players. The connections
for cassette unit 1 are duplicated on a set of phono plugs.

We emulate the use of two cassette units. An option allows you to hear the sound
of the tape during playback.

********************************************************************************

Disk system on sorcererd:
- Uses a Micropolis controller, disks holding 143KB. Top of RAM is BBFF.
- To boot CP/M, insert the disk, then type GO BC00 at the monitor prompt.

Other rumoured disk possibilities (not emulated):
- Soft-sectored, top of RAM is BDFF
- Having the option of a S100 box, any random system could be used in theory.



Exidy Sorcerer Video/Disk Unit:
- Contains a screen and 2 floppy drives. No details available. Partial
  emulation done but it's all guesswork. The unit contains 2 dipswitch units
  (one has 4 switches and the other has 8), a 8 MHz crystal, 3 proms and a
  FD1793-B01 fdc. Going by the code, it would appear to place the Z-80 into
  WAIT while reading a sector. To try it out: GO BF00 at the monitor prompt.
  Currently the CP/M sign-on message appears, followed by lockup due to a
  fdc problem.

********************************************************************************/

#include "emu.h"
#include "includes/sorcerer.h"
#include "emupal.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"


void sorcerer_state::sorcerer_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x07ff).bankrw("boot");
	map(0x0800, 0xbfff).ram();
	//AM_RANGE(0xc000, 0xdfff)      // mapped by the cartslot
	map(0xe000, 0xefff).rom();                     /* rom pac and bios */
	map(0xf000, 0xf7ff).ram().region("maincpu", 0xf000);        /* screen ram */
	map(0xf800, 0xfbff).rom();                     /* char rom */
	map(0xfc00, 0xffff).ram().region("maincpu", 0xfc00);        /* programmable chars */
}

void sorcerer_state::sorcererd_mem(address_map &map)
{
	map.unmap_value_high();
	sorcerer_mem(map);
	map(0xbc00, 0xbcff).rom();
	map(0xbe00, 0xbe03).rw(m_fdc, FUNC(micropolis_device::read), FUNC(micropolis_device::write));
	map(0xbf00, 0xbfff).rom();
}

void sorcerer_state::sorcerer_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0xfc, 0xfc).rw(m_uart, FUNC(ay31015_device::receive), FUNC(ay31015_device::transmit));
	map(0xfd, 0xfd).rw(FUNC(sorcerer_state::port_fd_r), FUNC(sorcerer_state::port_fd_w));
	map(0xfe, 0xfe).rw(FUNC(sorcerer_state::port_fe_r), FUNC(sorcerer_state::port_fe_w));
	map(0xff, 0xff).r("cent_status_in", FUNC(input_buffer_device::bus_r));
	map(0xff, 0xff).w(FUNC(sorcerer_state::port_ff_w));
}

void sorcerer_state::sorcererd_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	sorcerer_io(map);
	map(0x28, 0x2b).rw(m_fdc2, FUNC(fd1793_device::read), FUNC(fd1793_device::write));
	map(0x2c, 0x2c).w(FUNC(sorcerer_state::port_2c_w));
}

static INPUT_PORTS_START(sorcerer)
	PORT_START("VS")
	/* vblank */
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_VBLANK("screen")

	/* line 0 */
	PORT_START("X.0")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Control") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Graphic") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Stop") PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	/* line 1 */
	PORT_START("X.1")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("(Sel)") PORT_CODE(KEYCODE_F2) PORT_CHAR(27)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Skip") PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Repeat") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Clear") PORT_CODE(KEYCODE_F5) PORT_CHAR(12)
	/* line 2 */
	PORT_START("X.2")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q') PORT_CHAR(0x11)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a') PORT_CHAR(0x01)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z') PORT_CHAR(0x1a)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x') PORT_CHAR(0x18)
	/* line 3 */
	PORT_START("X.3")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w') PORT_CHAR(0x17)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s') PORT_CHAR(0x13)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d') PORT_CHAR(0x04)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c') PORT_CHAR(0x03)
	/* line 4 */
	PORT_START("X.4")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e') PORT_CHAR(0x05)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r') PORT_CHAR(0x12)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f') PORT_CHAR(0x06)
	/* line 5 */
	PORT_START("X.5")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t') PORT_CHAR(0x14)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g') PORT_CHAR(0x07)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v') PORT_CHAR(0x16)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b') PORT_CHAR(0x02)
	/* line 6 */
	PORT_START("X.6")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y') PORT_CHAR(0x19)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h') PORT_CHAR(0x08)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n') PORT_CHAR(0x0e)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m') PORT_CHAR(0x0d)
	/* line 7 */
	PORT_START("X.7")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u') PORT_CHAR(0x15)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j') PORT_CHAR(0x0a)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i') PORT_CHAR(0x09)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k') PORT_CHAR(0x0b)
	/* line 8 */
	PORT_START("X.8")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o') PORT_CHAR(0x0f)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l') PORT_CHAR(0x0c)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	/* line 9 */
	PORT_START("X.9")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p') PORT_CHAR(0x10)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("; +") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	/* line 10 */
	PORT_START("X.10")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(": *") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[ {") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[')  PORT_CHAR('{')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("] }") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')  PORT_CHAR('}')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@ `") PORT_CODE(KEYCODE_F7) PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\ |") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\')  PORT_CHAR('|')
	/* line 11 */
	PORT_START("X.11")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("- =") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^ ~") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LINE FEED") PORT_CODE(KEYCODE_F6) PORT_CHAR(10)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(13)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("_ Rub") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR('_') PORT_CHAR(8)
	/* line 12 */
	PORT_START("X.12")
	PORT_BIT(0x10, 0x10, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("- (PAD)") PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/ (PAD)") PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("* (PAD)") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("+ (PAD)") PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	/* line 13 */
	PORT_START("X.13")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7 (PAD)") PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8 (PAD)") PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4 (PAD)") PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1 (PAD)") PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0 (PAD)") PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	/* line 14 */
	PORT_START("X.14")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9 (PAD)") PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6 (PAD)") PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5 (PAD)") PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2 (PAD)") PORT_CODE(KEYCODE_2_PAD) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(". (PAD)") PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	/* line 15 */
	PORT_START("X.15")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3 (PAD)") PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("= (PAD)") PORT_CODE(KEYCODE_NUMLOCK)
	PORT_BIT(0x04, 0x04, IPT_UNUSED)
	PORT_BIT(0x02, 0x02, IPT_UNUSED)
	PORT_BIT(0x01, 0x01, IPT_UNUSED)

	/* Enhanced options not available on real hardware */
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x01, "Autorun on Quickload")
	PORT_CONFSETTING(    0x00, DEF_STR(No))
	PORT_CONFSETTING(    0x01, DEF_STR(Yes))
	/* hardware connected to printer port */
	PORT_CONFNAME( 0x02, 0x02, "Parallel port" )
	PORT_CONFSETTING(    0x00, "7-bit" )
	PORT_CONFSETTING(    0x02, "8-bit" )
	PORT_CONFNAME( 0x08, 0x08, "Cassette Speaker")
	PORT_CONFSETTING(    0x08, DEF_STR(On))
	PORT_CONFSETTING(    0x00, DEF_STR(Off))
INPUT_PORTS_END

/**************************** F4 CHARACTER DISPLAYER ******************************************************/

static const gfx_layout sorcerer_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{  0*8,  1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8 },
	8*8                 /* every char takes 8 bytes */
};

/* This will show the 128 characters in the ROM + whatever happens to be in the PCG */
static GFXDECODE_START( gfx_sorcerer )
	GFXDECODE_ENTRY( "maincpu", 0xf800, sorcerer_charlayout, 0, 1 )
GFXDECODE_END

void sorcerer_state::video_start()
{
	m_p_videoram = memregion("maincpu")->base()+0xf000;
}

uint32_t sorcerer_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t y,ra,chr,gfx;
	uint16_t sy=0,ma=0x80,x;
	uint16_t *p;

	for (y = 0; y < 30; y++)
	{
		for (ra = 0; ra < 8; ra++)
		{
			p = &bitmap.pix16(sy++);

			for (x = ma; x < ma+64; x++)
			{
				chr = m_p_videoram[x];

				/* get pattern of pixels for that character scanline */
				gfx = m_p_videoram[0x800 | (chr<<3) | ra];

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 7);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
			}
		}
		ma+=64;
	}
	return 0;
}


/**********************************************************************************************************/

static const floppy_interface sorcerer_floppy_interface =
{
	FLOPPY_STANDARD_8_SSSD,
	LEGACY_FLOPPY_OPTIONS_NAME(sorcerer),
	"floppy_8"
};

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_1200 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_1200 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_2 )
DEVICE_INPUT_DEFAULTS_END

void sorcerer_state::sorcerer(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, ES_CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &sorcerer_state::sorcerer_mem);
	m_maincpu->set_addrmap(AS_IO, &sorcerer_state::sorcerer_io);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(200));
	screen.set_size(64*8, 30*8);
	screen.set_visarea(0, 64*8-1, 0, 30*8-1);
	screen.set_screen_update(FUNC(sorcerer_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_sorcerer);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	AY31015(config, m_uart);
	m_uart->set_auto_rdav(true);

	CLOCK(config, m_uart_clock, ES_UART_CLOCK);
	m_uart_clock->signal_handler().set(m_uart, FUNC(ay31015_device::write_tcp));
	m_uart_clock->signal_handler().append(m_uart, FUNC(ay31015_device::write_rcp));

	RS232_PORT(config, "rs232", default_rs232_devices, "null_modem").set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	/* printer */
	/* The use of the parallel port as a general purpose port is not emulated.
	Currently the only use is to read the printer status in the Centronics CENDRV bios routine. */
	CENTRONICS(config, m_centronics, centronics_devices, "covox");
	m_centronics->busy_handler().set("cent_status_in", FUNC(input_buffer_device::write_bit7));

	INPUT_BUFFER(config, "cent_status_in");

	/* quickload */
	SNAPSHOT(config, "snapshot", "snp", attotime::from_seconds(2)).set_load_callback(FUNC(sorcerer_state::snapshot_cb), this);
	QUICKLOAD(config, "quickload", "bin", attotime::from_seconds(3)).set_load_callback(FUNC(sorcerer_state::quickload_cb), this);

	CASSETTE(config, m_cassette1);
	m_cassette1->set_formats(sorcerer_cassette_formats);
	m_cassette1->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette1->add_route(ALL_OUTPUTS, "mono", 0.05); // cass1 speaker
	m_cassette1->set_interface("sorcerer_cass");

	CASSETTE(config, m_cassette2);
	m_cassette2->set_formats(sorcerer_cassette_formats);
	m_cassette2->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette2->add_route(ALL_OUTPUTS, "mono", 0.05); // cass2 speaker
	m_cassette2->set_interface("sorcerer_cass");

	/* cartridge */
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "sorcerer_cart", "bin,rom");

	/* software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("sorcerer_cart");
	SOFTWARE_LIST(config, "cass_list").set_original("sorcerer_cass");

	// internal ram
	RAM(config, RAM_TAG).set_default_size("48K").set_extra_options("8K,16K,32K");
}

static void floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

void sorcerer_state::sorcererd(machine_config &config)
{
	sorcerer(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &sorcerer_state::sorcererd_mem);
	m_maincpu->set_addrmap(AS_IO, &sorcerer_state::sorcererd_io);

	MCFG_MACHINE_START_OVERRIDE(sorcerer_state, sorcererd )

	MICROPOLIS(config, m_fdc, 0);
	m_fdc->set_default_drive_tags();
	legacy_floppy_image_device::add_4drives(config, &sorcerer_floppy_interface);

	FD1793(config, m_fdc2, 8_MHz_XTAL / 8);  // confirmed clock
	m_fdc2->set_force_ready(true); // should be able to get rid of this when fdc issue is fixed
	m_fdc2->intrq_wr_callback().set(FUNC(sorcerer_state::intrq_w));
	m_fdc2->drq_wr_callback().set(FUNC(sorcerer_state::drq_w));
	FLOPPY_CONNECTOR(config, "fdc2:0", floppies, "525qd", floppy_image_device::default_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc2:1", floppies, "525qd", floppy_image_device::default_floppy_formats).enable_sound(true);

	SOFTWARE_LIST(config, "flop_list").set_original("sorcerer_flop");
}


void sorcerer_state::init_sorcerer()
{
	uint8_t *RAM = memregion("maincpu")->base();
	membank("boot")->configure_entries(0, 2, &RAM[0x0000], 0xe000);
}

/***************************************************************************

  Game driver(s)

***************************************************************************/
ROM_START(sorcerer)
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("exmo1-1.1e",   0xe000, 0x0800, CRC(ac924f67) SHA1(72fcad6dd1ed5ec0527f967604401284d0e4b6a1) ) /* monitor roms */
	ROM_LOAD("exmo1-2.2e",   0xe800, 0x0800, CRC(ead1d0f6) SHA1(c68bed7344091bca135e427b4793cc7d49ca01be) )
	ROM_LOAD("exchr-1.20d",  0xf800, 0x0400, CRC(4a7e1cdd) SHA1(2bf07a59c506b6e0c01ec721fb7b747b20f5dced) ) /* char rom */
ROM_END

ROM_START(sorcererd)
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("diskboot.dat", 0xbc00, 0x0100, CRC(d82a40d6) SHA1(cd1ef5fb0312cd1640e0853d2442d7d858bc3e3b) ) // micropolis floppy boot
	ROM_LOAD("boot.bin",     0xbf00, 0x0100, CRC(352e36bc) SHA1(99678e3cc4f315a0cf7d52aae511e405dc314190) ) // video/disk unit floppy boot
	ROM_LOAD("exmo1-1.1e",   0xe000, 0x0800, CRC(ac924f67) SHA1(72fcad6dd1ed5ec0527f967604401284d0e4b6a1) ) /* monitor roms */
	ROM_LOAD("exmo1-2.2e",   0xe800, 0x0800, CRC(ead1d0f6) SHA1(c68bed7344091bca135e427b4793cc7d49ca01be) )
	ROM_LOAD("exchr-1.20d",  0xf800, 0x0400, CRC(4a7e1cdd) SHA1(2bf07a59c506b6e0c01ec721fb7b747b20f5dced) ) /* char rom */

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD_OPTIONAL("bruce.15b",  0x0000, 0x0020, CRC(fae922cb) SHA1(470a86844cfeab0d9282242e03ff1d8a1b2238d1) ) /* video prom type 6331 */
	// from video/disk unit
	ROM_LOAD_OPTIONAL("sad4e.4e",  0x0100, 0x0100, CRC(b468a3f9) SHA1(8546f834901349baf59fc436c1a7cc57d541cddd) )
	ROM_LOAD_OPTIONAL("l.2e",      0x0200, 0x0100, CRC(9cb2500e) SHA1(d473c8dc042a4ace75174a93069fc0e9451763bd) )
	ROM_LOAD_OPTIONAL("h.3e",      0x0300, 0x0100, CRC(3c6163fb) SHA1(60ecefe461357eacfca64427931db6472283d0e3) )
ROM_END

ROM_START(sorcerer2)
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("exchr-1.20d",  0xf800, 0x0400, CRC(4a7e1cdd) SHA1(2bf07a59c506b6e0c01ec721fb7b747b20f5dced) ) /* char rom */
	ROM_SYSTEM_BIOS(0, "standard", "Standard")
	ROMX_LOAD("exm011-1.1e", 0xe000, 0x0800, CRC(af9394dc) SHA1(d7e0ada64d72d33e0790690be86a36020b41fd0d), ROM_BIOS(0) )
	ROMX_LOAD("exm011-2.2e", 0xe800, 0x0800, CRC(49978d6c) SHA1(b94127bfe99e5dc1cf5dbbb7d1b099b0ca036cd0), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "tvc", "TVI-MON-C-V1.5")
	ROMX_LOAD("tvc-1.1e",    0xe000, 0x0800, CRC(efc15a18) SHA1(3dee821270a0d83453b18baed88a024dfd0d7a6c), ROM_BIOS(1) )
	ROMX_LOAD("tvc-2.2e",    0xe800, 0x0800, CRC(bc194487) SHA1(dcfd916558e3e3be22091c5558ea633c332cf6c7), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(2, "dwmon", "DWMON 2.2C")
	ROMX_LOAD("dwmon.1e",    0xe000, 0x0800, CRC(a22db498) SHA1(ebedbce7454007f5a02fafe449fd09169173d7b3), ROM_BIOS(2) )
	ROMX_LOAD("dwmon.2e",    0xe800, 0x0800, CRC(7b22b65a) SHA1(7f23dd308f34b6d795d6df06f2387dfd17f69edd), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS(3, "ddmon", "DDMON 1.3")
	ROMX_LOAD("ddmon.1e",    0xe000, 0x0800, CRC(6ce481da) SHA1(c927762b29a281b7c13d59bb17ea56494c64569b), ROM_BIOS(3) )
	ROMX_LOAD("ddmon.2e",    0xe800, 0x0800, CRC(50069b13) SHA1(0808018830fac15cceaed8ff2b19900f77447470), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS(4, "adsmon", "ADSMON") // This requires an unemulated 80-column card. You can type 64 to get 64-columns, but it's mostly off the side.
	ROMX_LOAD("adsmon.1e",   0xe000, 0x0800, CRC(460f981a) SHA1(bdae1d87b9e8ae2cae11663acd349b9ed2387094), ROM_BIOS(4) )
	ROMX_LOAD("adsmon.2e",   0xe800, 0x0800, CRC(cb3f1dda) SHA1(3fc14306e83d73b9b9afd9b543566e52ba3e008f), ROM_BIOS(4) )
ROM_END

/*    YEAR  NAME       PARENT    COMPAT  MACHINE    INPUT     STATE           INIT           COMPANY      FULLNAME */
COMP( 1979, sorcerer,  0,        0,      sorcerer,  sorcerer, sorcerer_state, init_sorcerer, "Exidy Inc", "Sorcerer",                     0 )
COMP( 1979, sorcerer2, sorcerer, 0,      sorcerer,  sorcerer, sorcerer_state, init_sorcerer, "Exidy Inc", "Sorcerer 2",                   0 )
COMP( 1979, sorcererd, sorcerer, 0,      sorcererd, sorcerer, sorcerer_state, init_sorcerer, "Exidy Inc", "Sorcerer (with floppy disks)", 0 )
