// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

Samsung SPC-1000 driver by Miodrag Milanovic

    2009-05-10 Preliminary driver.
    2014-02-16 Added cassette, many games are playable

ToDo:
- Find out if any of the unconnected parts of 6000,4000,4001 are used


NOTE: 2014-09-13: added code from someone's modified MESS driver for floppy
                  disk. Since it is not to our coding standards, it is
                  commented out with #if 0/#endif and 3 slashes (///).
                  It is planned to be converted when time permits. The
                  author is Meeso Kim.

                  Hardware details of the fdc: Intelligent device, Z80 CPU,
                  XTAL_8MHz, PPI 8255, FDC uPD765C, 2 RAM chips, 28 other
                  small ics. And of course, no schematic.


2014-10-11: Replaced above code with MESS-compliant code [Meeso Kim]

2015-06-19: Added code for the centronics printer port

IMPORTANT NOTE for tape usage: you *FIRST* press PLAY on the tape drive
  (e.g. by pressing F2 in partial emulated keyboard mode) and *THEN* you
  type LOAD on the BASIC prompt!
  Otherwise, the system turns the tape motor ON but it does not receive any
  data from tape, and it turns it OFF before the user can press PLAY.


****************************************************************************/
/*
 * SAMSUNG SPC-1000 Series (info from zannylim)
 *
 * YEAR MODEL           MainVideo       PRT     FDD
 * ---- --------------  ------  -----   ------  -------
 * 1982 SPC-1000        S68047  RGB     buffer  -
 * 1983 SPC-1000        S68047  RF-TV   buffer  -
 * 1983 SPC-1100        S68047  RF-TV   -       -
 * 1983 SPC-1000/1100   MC6847  RF-TV   direct  support
 * 1985 SPC-1000A       MC6847  RF-TV   direct  support
 *
 * 2nd Video Display Processor type 1 : VDP UNIT (TMS9918 + 4KB VRAM) by staticsoft
 * 2nd Video Display Processor type 2 : SOFT BOX (TMS9918 + 4KB VRAM with BIOS) by sammi computer
 *
 * Intelligence FDD : SD-725(2FDD, RS232C), SD-720(1FDD), SD-725A(2FDD), SD-725B(Desktop 2FDD, RS232C) - EPSON TF20 F100
 * External FDD with Expansion slot : KWE-1000 by kyungwoo
 *
 * Network device : ISAM-1000 by samsung
 *
 *              +---------PRT------RGB----TVRF--+   SPC-1000
 *              +                               +
 *              +   ROM0                        +   CPU : Z80A (4MHz)
 *              +   ROM1                        +   RAM : 64KB
 *      +-------+   ROM2  AY-3-8910             +   VRAM : 6KB
 *      +                             SPC-1000  +   VDG : AMI S68047 with TTL RGB output
 *     IPL                                      +   PSG : AY-3-8910
 *     RESET                          S68047    +
 *      +           Z80A                        +   Include Internal Data-recorder
 *      +    ROM3                               +
 *      +                                       +   ROM : 32KB (8KB x 4)
 *      +---------------------------------------+
 *
 *              +---------PRT----VIDEO----TVRF--+   SPC-1000
 *              +                               +
 *              +   ROM0                 LM1889 +   Support RF TV Support, but Removed RGB output
 *              +   ROM1                        +
 *      +-------+   ROM2  AY-3-8910   SPC-1000  +
 *      +                                 1100  +
 *     IPL                                      +
 *     RESET                          S68047    +
 *      +           Z80A                        +
 *      +    ROM3                               +
 *      +                                       +
 *      +---------------------------------------+
 *
 *              +----------------VIDEO----TVRF--+   SPC-1100
 *              +                               +
 *              +   ROM0                 LM1889 +   Removed Printer port
 *              +   ROM1       LM386(5V)        +
 *      +-------+   ROM2  AY-3-8910   SPC-1000  +
 *      +                                 1100  +
 *     IPL                                      +
 *     RESET                          S68047    +
 *      +           Z80A                        +
 *      +    ROM3                               +
 *      +                                       +
 *      +---------------------------------------+
 *
 *              +---------PRT----VIDEO----TVRF--+   SPC-1000, SPC-1100
 *              +                               +
 *              +   ROM0                 MC1372 +   New Video Display Generator : MC6847
 *              +   ROM1       LM386(5V)        +
 *      +-------+   ROM2  AY-3-8910             +
 *      +           ROM3         SPC-1000/1100  +
 *     IPL                                      +
 *     RESET                          MC6847    +
 *      +           Z80A                        +
 *      +                                       +
 *      +                                       +
 *      +---------------------------------------+
 *
 *              +---------PRT----VIDEO----TVRF--+   SPC-1000, SPC-1100
 *              +                               +
 *              +   ROM0              S4 MC1372 +   REV PCB No.839291
 *              +   ROM1       LM386(5V)        +
 *      +-------+   ROM2  AY-3-8910             +   Add composite color on/off switch
 *      +           ROM3         SPC-1000/1100  +
 *     IPL                                      +
 *     RESET                          MC6847    +
 *      +           Z80A                        +
 *      +                                       +
 *      +                                       +
 *      +---------------------------------------+
 *
 *              +---------PRT----VIDEO----TVRF--+   SPC-1000A
 *              +                               +
 *              +   ROM0              S4 MC1372 +   Internal Data-recorder with Cassette Audio Player
 *              +   ROM1       LM386(12V)       +   Add FDD auto detect
 *      +-------+   ROM2  AY-3-8910             +   Remove IPL button
 *      + SPC-1000A ROM3                        +   Change DRAM refresh circuit
 *     IPL                                      +   Use 64K DRAM made by Samsung
 *      +                             MC6847    +   Use TTL IC made by Goldstar
 *      +           Z80A                        +
 *      +                                       +
 *      +                                       +
 *      +---------------------------------------+
 *
 */

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/ram.h"
#include "sound/ay8910.h"
#include "sound/wave.h"
#include "video/mc6847.h"
#include "imagedev/cassette.h"
#include "formats/spc1000_cas.h"
#include "bus/centronics/ctronics.h"

#include "bus/spc1000/exp.h"
#include "bus/spc1000/fdd.h"
#include "bus/spc1000/vdp.h"

#include "softlist.h"

class spc1000_state : public driver_device
{
public:
	spc1000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_vdg(*this, "mc6847")
		, m_cass(*this, "cassette")
		, m_ram(*this, RAM_TAG)
		, m_p_videoram(*this, "videoram")
		, m_io_kb(*this, "LINE")
		, m_io_joy(*this, "JOY")
		, m_centronics(*this, "centronics")
	{}

	DECLARE_WRITE8_MEMBER(iplk_w);
	DECLARE_READ8_MEMBER(iplk_r);
	DECLARE_WRITE_LINE_MEMBER(irq_w);
	DECLARE_WRITE8_MEMBER(gmode_w);
	DECLARE_READ8_MEMBER(gmode_r);
	DECLARE_READ8_MEMBER(porta_r);
	DECLARE_WRITE_LINE_MEMBER( centronics_busy_w ) { m_centronics_busy = state; }
	DECLARE_READ8_MEMBER(mc6847_videoram_r);
	DECLARE_WRITE8_MEMBER(cass_w);
	DECLARE_READ8_MEMBER(keyboard_r);
	MC6847_GET_CHARROM_MEMBER(get_char_rom)
	{
		return m_p_videoram[0x1000 + (ch & 0x7f) * 16 + line];
	}

private:
	UINT8 m_IPLK;
	UINT8 m_GMODE;
	UINT16 m_page;
	std::unique_ptr<UINT8[]> m_work_ram;
	attotime m_time;
	bool m_centronics_busy;
	virtual void machine_start() override;
	virtual void machine_reset() override;
	required_device<z80_device> m_maincpu;
	required_device<mc6847_base_device> m_vdg;
	required_device<cassette_image_device> m_cass;
	required_device<ram_device> m_ram;
	required_shared_ptr<UINT8> m_p_videoram;
	required_ioport_array<10> m_io_kb;
	required_ioport m_io_joy;
	required_device<centronics_device> m_centronics;
};

static ADDRESS_MAP_START(spc1000_mem, AS_PROGRAM, 8, spc1000_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_READ_BANK("bank1") AM_WRITE_BANK("bank2")
	AM_RANGE(0x8000, 0xffff) AM_READ_BANK("bank3") AM_WRITE_BANK("bank4")
ADDRESS_MAP_END

WRITE8_MEMBER(spc1000_state::iplk_w)
{
	m_IPLK = m_IPLK ? 0 : 1;
	membank("bank1")->set_entry(m_IPLK);
	membank("bank3")->set_entry(m_IPLK);
}

READ8_MEMBER(spc1000_state::iplk_r)
{
	m_IPLK = m_IPLK ? 0 : 1;
	membank("bank1")->set_entry(m_IPLK);
	membank("bank3")->set_entry(m_IPLK);

	return 0;
}

WRITE8_MEMBER( spc1000_state::cass_w )
{
	attotime time = machine().scheduler().time();
	m_cass->output(BIT(data, 0) ? -1.0 : 1.0);
	if (BIT(data, 1) && ATTOSECONDS_IN_MSEC((time - m_time).as_attoseconds()) > 500) {
		m_cass->change_state((m_cass->get_state() & CASSETTE_MASK_MOTOR) == CASSETTE_MOTOR_DISABLED ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
		m_time = time;
	}
	m_centronics->write_strobe(BIT(data, 2) ? true : false);
}

WRITE8_MEMBER(spc1000_state::gmode_w)
{
	m_GMODE = data;

	// m_GMODE layout: CSS|NA|PS2|PS1|~A/G|GM0|GM1|NA
	//  [PS2,PS1] is used to set screen 0/1 pages
	m_vdg->gm1_w(BIT(data, 1));
	m_vdg->gm0_w(BIT(data, 2));
	m_vdg->ag_w(BIT(data, 3));
	m_vdg->css_w(BIT(data, 7));
	m_page = ((BIT(data, 5) << 1) | BIT(data, 4)) * 0x200;
}

READ8_MEMBER(spc1000_state::gmode_r)
{
	return m_GMODE;
}

READ8_MEMBER( spc1000_state::keyboard_r )
{
	// most games just read kb in $8000-$8009 but a few of them
	// (e.g. Toiler Adventure II and Vela) use mirrored addr instead
	offset &= 0xf;

	if (offset <= 9)
		return m_io_kb[offset]->read();
	else
		return 0xff;
}


static ADDRESS_MAP_START( spc1000_io , AS_IO, 8, spc1000_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x1fff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x2000, 0x3fff) AM_READWRITE(gmode_r, gmode_w)
	AM_RANGE(0x4000, 0x4000) AM_DEVWRITE("ay8910", ay8910_device, address_w)
	AM_RANGE(0x4001, 0x4001) AM_DEVREADWRITE("ay8910", ay8910_device, data_r, data_w)
	AM_RANGE(0x6000, 0x6000) AM_WRITE(cass_w)
	AM_RANGE(0x8000, 0x9fff) AM_READ(keyboard_r)
	AM_RANGE(0xa000, 0xa000) AM_READWRITE(iplk_r, iplk_w)
	AM_RANGE(0xc000, 0xdfff) AM_DEVREADWRITE("ext1", spc1000_exp_device, read, write)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( spc1000 )
	PORT_START("LINE.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_RSHIFT) PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_RCONTROL) PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_PAUSE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Graph") PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^ ~") PORT_CODE(KEYCODE_TILDE) PORT_CHAR('^') PORT_CHAR('~') PORT_CHAR(0x1e)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Home") PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C') PORT_CHAR(0x03)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A') PORT_CHAR(0x01)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q') PORT_CHAR(0x16)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1 !") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')

	PORT_START("LINE.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Caps") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z') PORT_CHAR(0x1a)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("] }") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}') PORT_CHAR(0x1d)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V') PORT_CHAR(0x16)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S') PORT_CHAR(0x13)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W') PORT_CHAR(0x17)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2 \"") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')

	PORT_START("LINE.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8) PORT_CHAR(0x12)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(0x1b)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[ {") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{') PORT_CHAR(0x1b)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B') PORT_CHAR(0x02)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D') PORT_CHAR(0x04)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E') PORT_CHAR(0x05)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3 #") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')

	PORT_START("LINE.4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\ |") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|') PORT_CHAR(0x1c)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N') PORT_CHAR(0x0e)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F') PORT_CHAR(0x06)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R') PORT_CHAR(0x12)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4 $") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')

	PORT_START("LINE.5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M') PORT_CHAR(0x0d)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G') PORT_CHAR(0x07)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T') PORT_CHAR(0x14)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5 %") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')

	PORT_START("LINE.6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@ `") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X') PORT_CHAR(0x18)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H') PORT_CHAR(0x08)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y') PORT_CHAR(0x19)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6 &") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')

	PORT_START("LINE.7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P') PORT_CHAR(0x10)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J') PORT_CHAR(0x0a)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U') PORT_CHAR(0x15)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7 '") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("LINE.8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(": *") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K') PORT_CHAR(0x0b)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I') PORT_CHAR(0x09)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8 (") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')

	PORT_START("LINE.9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_START)    PORT_NAME("IPL") PORT_CODE(KEYCODE_END)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("- =") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("; +") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L') PORT_CHAR(0x0c)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O') PORT_CHAR(0x0e)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9 )") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')

	PORT_START("JOY")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED) // Button 2?
	PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED) // Cassette related
INPUT_PORTS_END


void spc1000_state::machine_start()
{
	UINT8 *mem = memregion("maincpu")->base();
	UINT8 *ram = m_ram->pointer();

	// configure and intialize banks 1 & 3 (read banks)
	membank("bank1")->configure_entry(0, ram);
	membank("bank1")->configure_entry(1, mem);
	membank("bank3")->configure_entry(0, ram + 0x8000);
	membank("bank3")->configure_entry(1, mem);
	membank("bank1")->set_entry(1);
	membank("bank3")->set_entry(1);

	// intialize banks 2 & 4 (write banks)
	membank("bank2")->set_base(ram);
	membank("bank4")->set_base(ram + 0x8000);

		m_time = machine().scheduler().time();
}

void spc1000_state::machine_reset()
{
	m_work_ram = make_unique_clear<UINT8[]>(0x10000);
	m_IPLK = 1;
}

READ8_MEMBER(spc1000_state::mc6847_videoram_r)
{
	if (offset == ~0)
		return 0xff;

	// m_GMODE layout: CSS|NA|PS2|PS1|~A/G|GM0|GM1|NA
	if (!BIT(m_GMODE, 3))
	{   // text mode (~A/G set to A)
		UINT8 data = m_p_videoram[offset + m_page + 0x800];
		m_vdg->inv_w(BIT(data, 0));
		m_vdg->css_w(BIT(data, 1));
		m_vdg->as_w (BIT(data, 2));
		m_vdg->intext_w(BIT(data, 3));
		return m_p_videoram[offset + m_page];
	}
	else
	{    // graphics mode: uses full 6KB of VRAM
		return m_p_videoram[offset];
	}
}

READ8_MEMBER( spc1000_state::porta_r )
{
	UINT8 data = 0x3f;
	data |= (m_cass->input() > 0.0038) ? 0x80 : 0;
	data |= ((m_cass->get_state() & CASSETTE_MASK_UISTATE) != CASSETTE_STOPPED) && ((m_cass->get_state() & CASSETTE_MASK_MOTOR) == CASSETTE_MOTOR_ENABLED)  ? 0x00 : 0x40;
	data &= ~(m_io_joy->read() & 0x3f);
	data &= ~((m_centronics_busy == 0)<< 5);
	return data;
}

// irq is inverted in emulation, so we need this trampoline
WRITE_LINE_MEMBER( spc1000_state::irq_w )
{
	m_maincpu->set_input_line(0, state ? CLEAR_LINE : HOLD_LINE);
}

//-------------------------------------------------
//  address maps
//-------------------------------------------------

extern SLOT_INTERFACE_START(spc1000_exp)
	SLOT_INTERFACE("fdd", SPC1000_FDD_EXP)
	SLOT_INTERFACE("vdp", SPC1000_VDP_EXP)
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( spc1000, spc1000_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(spc1000_mem)
	MCFG_CPU_IO_MAP(spc1000_io)

	/* video hardware */
	MCFG_SCREEN_MC6847_NTSC_ADD("screen", "mc6847")

	MCFG_DEVICE_ADD("mc6847", MC6847_NTSC, XTAL_3_579545MHz)
	MCFG_MC6847_FSYNC_CALLBACK(WRITELINE(spc1000_state, irq_w))
	MCFG_MC6847_INPUT_CALLBACK(READ8(spc1000_state, mc6847_videoram_r))
	MCFG_MC6847_CHARROM_CALLBACK(spc1000_state, get_char_rom)
	MCFG_MC6847_FIXED_MODE(MC6847_MODE_GM2)
	// other lines not connected

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ay8910", AY8910, XTAL_4MHz / 1)
	MCFG_AY8910_PORT_A_READ_CB(READ8(spc1000_state, porta_r))
	MCFG_AY8910_PORT_B_WRITE_CB(DEVWRITE8("cent_data_out", output_latch_device, write))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_DEVICE_ADD("ext1", SPC1000_EXP_SLOT, 0)
	MCFG_DEVICE_SLOT_INTERFACE(spc1000_exp, nullptr, false)

	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(spc1000_state, centronics_busy_w))
	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")
	MCFG_DEVICE_ADD("cent_status_in", INPUT_BUFFER, 0)

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_FORMATS(spc1000_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_DISABLED)

	MCFG_SOFTWARE_LIST_ADD("cass_list", "spc1000_cass")

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( spc1000 )
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("spcall.rom", 0x0000, 0x8000, CRC(19638fc9) SHA1(489f1baa7aebf3c8c660325fb1fd790d84203284))
ROM_END

#if 0
ROM_START( spc1000 )
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("spcall.rom", 0x0000, 0x8000, CRC(2fbb6eca) SHA1(cc9a076b0f00d54b2aec31f1f558b10f43ef61c8))
	/// more roms to come...
ROM_END
#endif


/* Driver */

/*    YEAR  NAME      PARENT  COMPAT   MACHINE    INPUT    CLASS         INIT    COMPANY    FULLNAME       FLAGS */
COMP( 1982, spc1000,  0,      0,       spc1000,   spc1000, driver_device,  0,   "Samsung", "SPC-1000", MACHINE_NOT_WORKING )
