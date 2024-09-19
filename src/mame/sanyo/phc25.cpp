// license:BSD-3-Clause
// copyright-holders:Curt Coder, Robbbert
/***************************************************************************

        Sanyo PHC-25

    https://web.archive.org/web/20180107213100/http://www.geocities.jp/sanyo_phc_25/
    http://www.phc25.com/

    Z80 @ 4 MHz
    MC6847 video
    3x 8KB BIOS ROM
    1x 4KB chargen ROM
    16KB RAM
    6KB video RAM

    LOCK key (CAPSLOCK) selects upper-case/lower-case on international version
    (phc25), and selects hiragana/upper-case on Japanese version (phc25j).


    TODO: PHC-25
    - sound is strange, volume is often low to non-existent.
    - colours and graphics are different to those shown at
      http://www.phc25.com/collection.htm - who is correct?
    - screen attribute bit 7 is unknown
    - cursor flashes too rapidly, maybe VDG FSYNC issue?
    - Japanese keyboard labels for phc25j.

    TODO: MAP-1010
    - tape control from IO port 0x00 not implemented.
    - dump the Demo tape which contains both audio and data, and
      makes use of tape control.


10 SCREEN3,1,1:COLOR,,1:CLS
20 X1=INT(RND(1)*256):Y1=INT(RND(1)*192):X2=INT(RND(1)*256):Y2=INT(RND(1)*192):C=INT(RND(1)*4)+1:LINE(X1,Y1)-(X2,Y2),C:GOTO 20
RUN


10 SCREEN2,1,1:CLS:FORX=0TO8:LINE(X*24,0)-(X*24+16,191),X,BF:NEXT
RUN

*****************************************************************************/

#include "emu.h"

#include "bus/centronics/ctronics.h"
#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "machine/upd1990a.h"
#include "sound/ay8910.h"
#include "video/mc6847.h"

#include "formats/phc25_cas.h"

#include "softlist_dev.h"
#include "speaker.h"


namespace {

class phc25_state : public driver_device
{
public:
	phc25_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_vram(*this, "videoram")
		, m_maincpu(*this, "maincpu")
		, m_chargen(*this, "chargen")
		, m_vdg(*this, "mc6847")
		, m_centronics(*this, "centronics")
		, m_cent_data_out(*this, "cent_data_out")
		, m_cassette(*this, "cassette")
	{ }

	void phc25(machine_config &config);
	void phc25j(machine_config &config);

protected:
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;

	virtual void port00_w(uint8_t data);
	virtual uint8_t port40_r();
	virtual void port40_w(uint8_t data);

	required_shared_ptr<uint8_t> m_vram;
	required_device<cpu_device> m_maincpu;
	required_region_ptr<uint8_t> m_chargen;
	required_device<mc6847_base_device> m_vdg;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	required_device<cassette_image_device> m_cassette;

private:
	void phc25_mem(address_map &map) ATTR_COLD;
	void phc25_io(address_map &map) ATTR_COLD;

	uint8_t video_ram_r(offs_t offset);
	MC6847_GET_CHARROM_MEMBER(char_rom_r);

	uint8_t m_port40 = 0U;
	int m_centronics_busy = 0;
};

class map1010_state : public phc25_state
{
public:
	map1010_state(const machine_config &mconfig, device_type type, const char *tag)
		: phc25_state(mconfig, type, tag)
		, m_rtc(*this, "rtc")
		, m_key(*this, "KEY%u", 0U)
		, m_control(*this, "CONTROL")
		, m_monitor(*this, "MONITOR")
		, m_rts(*this, "RTS")
	{ }

	void map1010(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(monitor_changed);
	DECLARE_INPUT_CHANGED_MEMBER(rts_changed);

protected:
	void machine_start() override ATTR_COLD;

	virtual void port00_w(uint8_t data) override;
	virtual uint8_t port40_r() override;

private:
	required_device<upd1990a_device> m_rtc;
	required_ioport_array<12> m_key;
	required_ioport m_control;
	required_ioport m_monitor;
	required_ioport m_rts;

	void update_monitor();

	void port20_w(uint8_t data);

	void map1010_mem(address_map &map) ATTR_COLD;
	void map1010_io(address_map &map) ATTR_COLD;
};

/* Read/Write Handlers */

void phc25_state::port00_w(uint8_t data)
{
	m_cent_data_out->write(data);
}

void map1010_state::port00_w(uint8_t data)
{
	// TODO: port used for both printer and tape control.

	phc25_state::port00_w(data);
}


void map1010_state::port20_w(uint8_t data)
{
	m_rtc->c0_w(BIT(data, 0));
	m_rtc->c1_w(BIT(data, 1));
	m_rtc->c2_w(BIT(data, 2));
	m_rtc->data_in_w(BIT(data, 3));
	m_rtc->clk_w(BIT(data, 4));
	m_rtc->stb_w(BIT(data, 7));
}


uint8_t phc25_state::port40_r()
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4       vertical sync
	    5       cassette read
	    6       centronics busy
	    7       horizontal sync

	*/

	uint8_t data = 0;

	/* vertical sync */
	data |= !m_vdg->fs_r() << 4;

	/* cassette read */
	data |= (m_cassette->input() < +0.3) << 5;

	/* centronics busy */
	data |= m_centronics_busy << 6;

	/* horizontal sync */
	data |= !m_vdg->hs_r() << 7;

	return data;
}

uint8_t map1010_state::port40_r()
{
	uint8_t data = phc25_state::port40_r();

	/* rtc data */
	data |= m_rtc->data_out_r();

	return data;
}

void phc25_state::port40_w(uint8_t data)
{
	/*

	    bit     description

	    0       cassette output
	    1       cassette motor
	    2       LED in the LOCK button (on = capslock)
	    3       centronics strobe
	    4
	    5       MC6847 GM0
	    6       MC6847 CSS
	    7       MC6847 A/G

	*/

	/* cassette output */
	m_cassette->output( BIT(data, 0) ? -1.0 : +1.0);

	/* cassette motor */
	m_cassette->change_state(BIT(data, 1) ? CASSETTE_MOTOR_DISABLED : CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);

	/* centronics strobe */
	m_centronics->write_strobe(BIT(data, 3));

	/* MC6847 */
	m_vdg->gm0_w(BIT(data, 5));
	m_vdg->css_w(BIT(data, 6));
	m_vdg->ag_w(BIT(data, 7));
	m_port40 = data;
}

/* Memory Maps */

void phc25_state::phc25_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x5fff).rom().region("maincpu", 0);
	map(0x6000, 0x77ff).ram().share("videoram");
	map(0xc000, 0xffff).ram();
}

void phc25_state::phc25_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(phc25_state::port00_w));
	map(0x40, 0x40).rw(FUNC(phc25_state::port40_r), FUNC(phc25_state::port40_w));
	map(0x80, 0x80).portr("KEY0");
	map(0x81, 0x81).portr("KEY1");
	map(0x82, 0x82).portr("KEY2");
	map(0x83, 0x83).portr("KEY3");
	map(0x84, 0x84).portr("KEY4");
	map(0x85, 0x85).portr("KEY5");
	map(0x86, 0x86).portr("KEY6");
	map(0x87, 0x87).portr("KEY7");
	map(0x88, 0x88).portr("KEY8");
	map(0xc0, 0xc0).w("ay8910", FUNC(ay8910_device::data_w));
	map(0xc1, 0xc1).rw("ay8910", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_w));
}


void map1010_state::map1010_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x5fff).rom().region("maincpu", 0);
	map(0x6000, 0x77ff).ram().share("videoram");
	map(0x7800, 0x785f).lr8(NAME([this](offs_t offset) { return BIT(m_key[offset >> 3]->read(), offset & 7); }));
	map(0x8000, 0xffff).ram();
}

void map1010_state::map1010_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(map1010_state::port00_w));
	map(0x20, 0x20).w(FUNC(map1010_state::port20_w));
	map(0x40, 0x40).rw(FUNC(map1010_state::port40_r), FUNC(map1010_state::port40_w));
	map(0xc0, 0xc0).w("ay8910", FUNC(ay8910_device::data_w));
	map(0xc1, 0xc1).rw("ay8910", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_w));
}

/* Input Ports */

static INPUT_PORTS_START( joyports )
	PORT_START("JOY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("JOY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( phc25 )
	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u2191") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP)) // U+2191 = ↑
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("INS DEL") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) // unlabeled key

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u2193") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) // U+2193 = ↓
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u2190") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) // U+2190 = ←
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('[')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u2192") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) // U+2192 = →
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(']')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F3") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F4") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F2") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F1") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')

	PORT_START("KEY8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("GRAPH") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_INCLUDE( joyports )
INPUT_PORTS_END

static INPUT_PORTS_START( phc25j )
	PORT_INCLUDE( phc25 )
INPUT_PORTS_END

static INPUT_PORTS_START( map1010 )
	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M')

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)  PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('_')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(13) // are RETURN and keypad RET the same key?

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K')

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS_PAD) PORT_CHAR(UCHAR_MAMEKEY(EQUALS_PAD))

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I')

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('_') PORT_CHAR('|')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("INS DEL") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))

	PORT_START("KEY8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F1") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F2") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F3") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F4") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))

	PORT_START("KEY9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u2190") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))   // U+2190 = ←
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u2192") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) // U+2192 = →
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u2191") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))       // U+2191 = ↑
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(u8"\u2193") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))   // U+2193 = ↓
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))

	PORT_START("KEY10")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LSHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RSHIFT") PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("GRAPH") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY11")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("KANA") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_INCLUDE( joyports )

	PORT_START("CONTROL")
	PORT_CONFNAME( 0x01, 0x00, "Control Selector" )
	PORT_CONFSETTING( 0x00, "On Line")
	PORT_CONFSETTING( 0x01, "Manual (Print)" )

	PORT_START("MONITOR")
	PORT_CONFNAME( 0x01, 0x01, "Monitor Selector" ) PORT_CHANGED_MEMBER(DEVICE_SELF, map1010_state, monitor_changed, 0)
	PORT_CONFSETTING( 0x01, "B")
	PORT_CONFSETTING( 0x00, "A" )

	PORT_START("RTS")
	PORT_CONFNAME( 0x01, 0x00, "RTS" ) PORT_CHANGED_MEMBER(DEVICE_SELF, map1010_state, rts_changed, 0)
	PORT_CONFSETTING( 0x00, "A")
	PORT_CONFSETTING( 0x01, "B" )
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(map1010_state::monitor_changed)
{
	/* Channel A/B select to monitor */
	update_monitor();
}

INPUT_CHANGED_MEMBER(map1010_state::rts_changed)
{
	/* Channel A/B select for data */
	m_cassette->set_channel(newval);

	update_monitor();
}

void map1010_state::update_monitor()
{
	/* RTS (unknown meaning) seems to invert the selected monitor channel */
	uint32_t output = m_monitor->read() ^ m_rts->read();

	m_cassette->reset_routes();
	m_cassette->add_route(output, "mono", 0.70);
}

/* Video */

uint8_t phc25_state::video_ram_r(offs_t offset)
{
	if (BIT(m_port40, 7)) // graphics
	{
		return m_vram[offset];
	}
	else    // text
	{
		offset &= 0x7ff;
		m_vdg->inv_w(BIT(m_vram[offset | 0x800], 0)); // cursor attribute
		m_vdg->as_w(BIT(m_vram[offset | 0x800], 1));  // screen2 lores attribute
		m_vdg->css_w(BIT(m_vram[offset | 0x800], 2)); // css attribute
		// bit 7 is set for all text (not spaces), meaning is unknown
		return m_vram[offset];
	}
}

MC6847_GET_CHARROM_MEMBER(phc25_state::char_rom_r)
{
	return m_chargen[(ch * 16 + line) & 0xfff];
}

void phc25_state::machine_reset()
{
	m_port40 = 0;
}

void phc25_state::machine_start()
{
	save_item(NAME(m_port40));
	save_item(NAME(m_centronics_busy));
}

void map1010_state::machine_start()
{
	phc25_state::machine_start();

	m_rtc->cs_w(1);
	m_rtc->oe_w(1);
}

/* Machine Driver */

void phc25_state::phc25(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &phc25_state::phc25_mem);
	m_maincpu->set_addrmap(AS_IO, &phc25_state::phc25_io);

	/* video hardware */
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	MC6847_PAL(config, m_vdg, XTAL(4'433'619));
	m_vdg->set_screen("screen");
	m_vdg->fsync_wr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0).invert();
	m_vdg->input_callback().set(FUNC(phc25_state::video_ram_r));
	m_vdg->set_get_char_rom(FUNC(phc25_state::char_rom_r));
	m_vdg->set_get_fixed_mode(mc6847_pal_device::MODE_GM2 | mc6847_pal_device::MODE_GM1 | mc6847_pal_device::MODE_INTEXT);
	// other lines not connected

	/* sound hardware (Synthesizer PSG-01 add-on) */
	SPEAKER(config, "mono").front_center();
	ay8910_device &psg(AY8910(config, "ay8910", 1'000'000));
	psg.port_a_read_callback().set_ioport("JOY0");
	psg.port_b_read_callback().set_ioport("JOY1");
	psg.add_route(ALL_OUTPUTS, "mono", 2.00);

	/* devices */
	CASSETTE(config, m_cassette);
	m_cassette->set_formats(phc25_cassette_formats);
	m_cassette->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.1);
	m_cassette->set_interface("phc25_cass");

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set([this](int state) { m_centronics_busy = state; });

	OUTPUT_LATCH(config, m_cent_data_out);
	m_centronics->set_output_latch(*m_cent_data_out);

	/* software lists */
	SOFTWARE_LIST(config, "cass_list").set_original("phc25_cass");
}

void phc25_state::phc25j(machine_config &config)
{
	phc25(config);

	MC6847_NTSC(config.replace(), m_vdg, XTAL(3'579'545));
	m_vdg->set_screen("screen");
	m_vdg->fsync_wr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0).invert();
	m_vdg->input_callback().set(FUNC(phc25_state::video_ram_r));
	m_vdg->set_get_char_rom(FUNC(phc25_state::char_rom_r));
	m_vdg->set_get_fixed_mode(mc6847_ntsc_device::MODE_GM2 | mc6847_ntsc_device::MODE_GM1 | mc6847_ntsc_device::MODE_INTEXT);
	// other lines not connected
}

void map1010_state::map1010(machine_config &config)
{
	phc25j(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &map1010_state::map1010_mem);
	m_maincpu->set_addrmap(AS_IO, &map1010_state::map1010_io);

	UPD1990A(config, m_rtc, 32.768_kHz_XTAL);

	// TODO: TC9144 device for tape deck control

	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->set_stereo();
	m_cassette->reset_routes();
	m_cassette->set_channel(0);             // Channel A contains data
	m_cassette->add_route(1, "mono", 0.70); // Channel B contains audio

	subdevice<software_list_device>("cass_list")->set_original("");
	subdevice<software_list_device>("cass_list")->set_compatible("phc25_cass");
}

/* ROMs */

ROM_START( phc25 )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "phc25rom.0", 0x0000, 0x2000, CRC(fa28336b) SHA1(582376bee455e124de24ba4ac02326c8a592fa5a)) // 031_00aa.ic13 ?
	ROM_LOAD( "phc25rom.1", 0x2000, 0x2000, CRC(38fd578b) SHA1(dc3db78c0cdc89f1605200d39535be65a4091705)) // 031_01a.ic14 ?
	ROM_LOAD( "phc25rom.2", 0x4000, 0x2000, CRC(54392b27) SHA1(1587827fe9438780b50164727ce3fdea1b98078a)) // 031_02a.ic15 ?

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "031_04a.ic6", 0x0000, 0x1000, CRC(e56fb8c5) SHA1(6fc388c17fb43debfbc1464f767d0ce1375ce27b))
ROM_END

ROM_START( phc25j )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "phc25-11.0", 0x0000, 0x2000, CRC(287e83b0) SHA1(9fe960a8245f28efc04defeeeaceb1e5ec6793b8))
	ROM_LOAD( "phc25-11.1", 0x2000, 0x2000, CRC(6223f945) SHA1(5d44b883b6264cb5d2e21b2269308630c62e0e56))
	ROM_LOAD( "phc25-11.2", 0x4000, 0x2000, CRC(da859ae4) SHA1(6121e85947921e434d0157c378de3d81537f6b9f))
	//ROM_LOAD( "022_00aa.ic", 0x0000, 0x2000, NO_DUMP )
	//ROM_LOAD( "022_01aa.ic", 0x2000, 0x2000, NO_DUMP )
	//ROM_LOAD( "022_02aa.ic", 0x4000, 0x2000, NO_DUMP )

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "022_04a.ic6", 0x0000, 0x1000, CRC(ddb24f2b) SHA1(88b5095a5d3f09f9f508c8779c02823a887b7ba6))
ROM_END

ROM_START( map1010 )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "030_01ba.bin", 0x0000, 0x2000, CRC(cc3fb33a) SHA1(60dc8afe1310a3bb8fd425b6a021df9c2a6a33f4))
	ROM_LOAD( "030_02ba.bin", 0x2000, 0x2000, CRC(1aaaeb2b) SHA1(e906ee69da543b2bfa3850574443eee57555cdda))
	ROM_LOAD( "030_03bb.bin", 0x4000, 0x2000, CRC(a7b01c17) SHA1(f4a24b64331e13f94bdf2421b0a2feb7d2ca8677))

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "chargen.bin", 0x0000, 0x1000, CRC(ddb24f2b) SHA1(88b5095a5d3f09f9f508c8779c02823a887b7ba6))
ROM_END

} // anonymous namespace


//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY  FULLNAME           FLAGS
COMP( 1983, phc25,   0,      0,      phc25,   phc25,   phc25_state,   empty_init, "Sanyo", "PHC-25 (Europe)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
COMP( 1983, phc25j,  phc25,  0,      phc25j,  phc25j,  phc25_state,   empty_init, "Sanyo", "PHC-25 (Japan)",  MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
COMP( 1983, map1010, 0,      0,      map1010, map1010, map1010_state, empty_init, "Seiko", "MAP-1010",        MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
