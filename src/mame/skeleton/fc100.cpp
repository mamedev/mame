// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Goldstar FC-100 (FC stands for Famicom)

2014/04/20 Skeleton driver.

Known chips: M5C6847P, AY-3-8910, 8251. XTALS 7.15909, 4.9152

No manuals or schematics available.
Shift-Run to BREAK out of CLOAD.
Cassette uses the uart.
There is an inbuilt Monitor. MON to enter. Commands are D,G,M,R,S,X.


Test of semigraphic 6
10 SCREEN 2:CLS
20 FOR I=0 TO 360
30 PSET(128+SIN(I)*90,91-COS(I)*90), 1
40 NEXT
RUN

TODO:
- Cassette can be 600 or 1200 baud, how is 600 baud selected?
- Hookup Graphics modes and colours
- Unknown i/o ports
- Need cart software (current code is just a guess)


****************************************************************************/


#include "emu.h"

#include "bus/centronics/ctronics.h"
#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "machine/buffer.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/timer.h"
#include "sound/ay8910.h"
#include "video/mc6847.h"

#include "emupal.h"
#include "speaker.h"

#include "formats/fc100_cas.h"


namespace {

class fc100_state : public driver_device
{
public:
	fc100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_vdg(*this, "vdg")
		, m_p_videoram(*this, "videoram")
		, m_p_chargen(*this, "chargen")
		, m_cass(*this, "cassette")
		, m_cart(*this, "cartslot")
		, m_uart(*this, "uart")
		, m_centronics(*this, "centronics")
		, m_keyboard(*this, "KEY.%u", 0U)
	{ }

	void fc100(machine_config &config);

	void init_fc100();

private:
	uint8_t mc6847_videoram_r(offs_t offset);
	uint8_t port00_r(offs_t offset);
	void port31_w(uint8_t data);
	void port33_w(uint8_t data);
	void port43_w(uint8_t data);
	void port60_w(offs_t offset, uint8_t data);
	void port70_w(offs_t offset, uint8_t data);
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_w);
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_r);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_k);

	MC6847_GET_CHARROM_MEMBER(get_char_rom)
	{
		return m_p_chargen[(ch * 16 + line) & 0xfff];
	}
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// graphics signals
	uint8_t m_ag = 0U;
	uint8_t m_gm2 = 0U;
	uint8_t m_gm1 = 0U;
	uint8_t m_gm0 = 0U;
	uint8_t m_as = 0U;
	uint8_t m_css = 0U;
	uint8_t m_intext = 0U;
	uint8_t m_inv = 0U;
	uint8_t m_cass_data[4]{};
	bool m_cassbit = 0;
	bool m_cassold = 0;
	uint8_t m_key_pressed = 0U;
	bool m_banksw_unlocked = 0;

	required_device<cpu_device> m_maincpu;
	required_device<mc6847_base_device> m_vdg;
	required_shared_ptr<uint8_t> m_p_videoram;
	required_region_ptr<u8> m_p_chargen;
	required_device<cassette_image_device> m_cass;
	required_device<generic_slot_device> m_cart;
	required_device<i8251_device> m_uart;
	required_device<centronics_device> m_centronics;
	required_ioport_array<16> m_keyboard;
};


void fc100_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x5fff).rom().region("roms", 0);
	//map(0x6000, 0x6fff)      // mapped by the cartslot
	map(0x7800, 0x7fff).bankr("bankr").bankw("bankw"); // Banked RAM/ROM
	map(0x8000, 0xbfff).ram(); // expansion ram pack - if omitted you get a 'Pages?' prompt at boot
	map(0xc000, 0xffff).ram().share("videoram");
}

void fc100_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x0f).r(FUNC(fc100_state::port00_r));
	// map(0x10, 0x10).w(FUNC(fc100_state::port10_w));  // vdg, unknown effects
	map(0x21, 0x21).w("psg", FUNC(ay8910_device::data_w));
	map(0x22, 0x22).r("psg", FUNC(ay8910_device::data_r));
	map(0x23, 0x23).w("psg", FUNC(ay8910_device::address_w));
	map(0x31, 0x31).w(FUNC(fc100_state::port31_w));
	map(0x33, 0x33).w(FUNC(fc100_state::port33_w));
	map(0x40, 0x40).w("cent_data_out", FUNC(output_latch_device::write));
	map(0x42, 0x42).nopw(); // bit 0 could be printer select
	map(0x43, 0x43).w(FUNC(fc100_state::port43_w));
	map(0x44, 0x44).r("cent_status_in", FUNC(input_buffer_device::read));
	map(0x60, 0x61).w(FUNC(fc100_state::port60_w));
	map(0x70, 0x71).w(FUNC(fc100_state::port70_w));
	map(0xb0, 0xb0).rw(m_uart, FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
	map(0xb8, 0xb8).rw(m_uart, FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
}

static INPUT_PORTS_START( fc100 )
	PORT_START("KEY.0")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT) PORT_NAME("Graph") // does nothing
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("Shift") PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME("Ctrl")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE PORT_NAME("Caps")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("[") PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("]") PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_NAME("P") PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_NAME("O") PORT_CHAR('o') PORT_CHAR('O')

	PORT_START("KEY.1")
	PORT_BIT(0xF0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("\\") PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Backspace") PORT_CHAR(8)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("=") PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Enter") PORT_CHAR(13)

	PORT_START("KEY.2")
	PORT_BIT(0xF0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Left") PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Down") PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Right") PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_NAME("Up") PORT_CHAR(UCHAR_MAMEKEY(UP))

	PORT_START("KEY.3")
	PORT_BIT(0xF0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_NAME("9") PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_NAME(";") PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("'") PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_NAME("Tab") PORT_CHAR(9)

	PORT_START("KEY.4")
	PORT_BIT(0xF0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_NAME("5") PORT_CHAR('5') PORT_CHAR('^')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_NAME("6") PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_NAME("7") PORT_CHAR('7') PORT_CHAR('%')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_NAME("8") PORT_CHAR('8') PORT_CHAR('*')

	PORT_START("KEY.5")
	PORT_BIT(0xF0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_NAME("1") PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_NAME("2") PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_NAME("3") PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_NAME("4") PORT_CHAR('4') PORT_CHAR('$')

	PORT_START("KEY.6")
	PORT_BIT(0xF0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5) PORT_NAME("F5") PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Space") PORT_CHAR(32)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN) PORT_NAME("Run")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_NAME("0") PORT_CHAR('0') PORT_CHAR(')')

	PORT_START("KEY.7")
	PORT_BIT(0xF0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) PORT_NAME("F1") PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2) PORT_NAME("F2") PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3) PORT_NAME("F3") PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4) PORT_NAME("F4") PORT_CHAR(UCHAR_MAMEKEY(F4)) PORT_CHAR(UCHAR_MAMEKEY(F9))

	PORT_START("KEY.8")
	PORT_BIT(0xF0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("-") PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("/") PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_NAME(".") PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_NAME(",") PORT_CHAR(',') PORT_CHAR('<')

	PORT_START("KEY.9")
	PORT_BIT(0xF0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME) PORT_NAME("Home")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT) PORT_NAME("Ins")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL) PORT_NAME("Del")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC) PORT_NAME("Esc") PORT_CHAR(27)

	PORT_START("KEY.10")
	PORT_BIT(0xF0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_NAME("B") PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_NAME("N") PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_NAME("M") PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_NAME("L") PORT_CHAR('l') PORT_CHAR('L')

	PORT_START("KEY.11")
	PORT_BIT(0xF0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_NAME("Z") PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_NAME("X") PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_NAME("C") PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_NAME("V") PORT_CHAR('v') PORT_CHAR('V')

	PORT_START("KEY.12")
	PORT_BIT(0xF0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_NAME("G") PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_NAME("H") PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_NAME("J") PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_NAME("K") PORT_CHAR('k') PORT_CHAR('K')

	PORT_START("KEY.13")
	PORT_BIT(0xF0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_NAME("A") PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_NAME("S") PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_NAME("D") PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_NAME("F") PORT_CHAR('f') PORT_CHAR('F')

	PORT_START("KEY.14")
	PORT_BIT(0xF0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_NAME("T") PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_NAME("Y") PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_NAME("U") PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_NAME("I") PORT_CHAR('i') PORT_CHAR('I')

	PORT_START("KEY.15")
	PORT_BIT(0xF0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_NAME("Q") PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_NAME("W") PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_NAME("E") PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_NAME("R") PORT_CHAR('r') PORT_CHAR('R')

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

uint8_t fc100_state::port00_r(offs_t offset)
{
	return m_keyboard[offset]->read();
}

// The timer frequency controls the auto-repeat delay and speed
// m_key_pressed:
// 0 = no key pressed, nothing to do
// 1 = key pressed, generate IRQ.
// 2 = key released, generate IRQ.
TIMER_DEVICE_CALLBACK_MEMBER( fc100_state::timer_k)
{
	/* scan the keyboard */
	uint8_t i;

	for (i = 0; i < 16; i++)
	{
		if (m_keyboard[i]->read() < 255)
		{
			// IRQ if key pressed
			m_key_pressed = 1;
			m_maincpu->set_input_line(0, HOLD_LINE);
			return;
		}
	}

	if (m_key_pressed == 1) // IRQ for key released
	{
		m_key_pressed = 2;
		m_maincpu->set_input_line(0, HOLD_LINE);
		return;
	}
	else
	if (m_key_pressed == 2) // release IRQ
	{
		m_key_pressed = 0;
		m_maincpu->set_input_line(0, CLEAR_LINE);
	}
}


//********************* AUDIO **********************************
#if 0
void fc100_state::ay_port_a_w(uint8_t data)
{
	m_ag = BIT(data, 4);
	m_gm2 = BIT(data, 6);
	m_gm1 = BIT(data, 3);
	m_gm0 = BIT(data, 3);
	m_css = m_ag;

	m_vdg->ag_w( m_ag ? ASSERT_LINE : CLEAR_LINE );
	m_vdg->gm2_w( m_gm2 ? ASSERT_LINE : CLEAR_LINE );
	m_vdg->gm1_w( m_gm1 ? ASSERT_LINE : CLEAR_LINE );
	m_vdg->gm0_w( m_gm0 ? ASSERT_LINE : CLEAR_LINE );
	m_vdg->css_w( m_css ? ASSERT_LINE : CLEAR_LINE );
	m_vdg->hack_black_becomes_blue( BIT(data, 1) );
}
#endif

//******************** VIDEO **********************************

uint8_t fc100_state::mc6847_videoram_r(offs_t offset)
{
	if (offset == ~0) return 0xff;

	if ( m_ag )
	{
		if ( m_gm2 )
		{
			// 256 x 192 / 6KB
			offset = ( ( offset & 0x1fc0 ) >> 1 ) | ( offset & 0x1f );
			return m_p_videoram[offset % 0xc00];
		}
		else
		{
			// 256 x 96 / 3KB
			return m_p_videoram[offset % 0xc00];
		}
	}

	// Standard text
	uint8_t data = m_p_videoram[offset];
	uint8_t attr = m_p_videoram[offset+0x200];

	// unknown bits 1,2,4,7
	m_vdg->inv_w( BIT( attr, 0 ));
	m_vdg->css_w( BIT( attr, 1)); // guess
	m_vdg->as_w( BIT( attr, 6 ));

	return data;
}

/* F4 Character Displayer */
static const gfx_layout u53_charlayout =
{
	7, 15,                   /* 7 x 15 characters */
	256,                  /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( gfx_fc100 )
	GFXDECODE_ENTRY( "chargen", 0x0000, u53_charlayout, 0, 1 )
GFXDECODE_END

//********************** CENTRONICS PRINTER ***********************************

void fc100_state::port43_w(uint8_t data)
{
	m_centronics->write_strobe(BIT(data, 2));
	m_centronics->write_init(BIT(data, 3));
}

//********************** UART/CASSETTE ***********************************

void fc100_state::port31_w(uint8_t data)
{
	if (data == 8)
		m_cass->change_state(CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);
}

void fc100_state::port33_w(uint8_t data)
{
	if (data == 0)
		m_cass->change_state(CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
}

TIMER_DEVICE_CALLBACK_MEMBER( fc100_state::kansas_w )
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

TIMER_DEVICE_CALLBACK_MEMBER( fc100_state::kansas_r)
{
	/* cassette - turn 1200/2400Hz to a bit */
	m_cass_data[1]++;
	uint8_t cass_ws = (m_cass->input() > +0.03) ? 1 : 0;

	if (cass_ws != m_cass_data[0])
	{
		m_cass_data[0] = cass_ws;
		m_uart->write_rxd((m_cass_data[1] < 12) ? 1 : 0);
		m_cass_data[1] = 0;
	}
}

//******************** MACHINE ******************************

void fc100_state::machine_start()
{
	if (m_cart->exists())
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x6000, 0x6fff, read8sm_delegate(*m_cart, FUNC(generic_slot_device::read_rom)));

	save_item(NAME(m_ag));
	save_item(NAME(m_gm2));
	save_item(NAME(m_gm1));
	save_item(NAME(m_gm0));
	save_item(NAME(m_as));
	save_item(NAME(m_css));
	save_item(NAME(m_intext));
	save_item(NAME(m_inv));
	save_item(NAME(m_cass_data));
	save_item(NAME(m_cassbit));
	save_item(NAME(m_cassold));
	save_item(NAME(m_key_pressed));
	save_item(NAME(m_banksw_unlocked));
}

void fc100_state::machine_reset()
{
	m_ag = 0;
	m_gm2 = 0;
	m_gm1 = 0;
	m_gm0 = 0;
	m_as = 0;
	m_css = 0;
	m_intext = 0;
	m_inv = 0;

	m_cass_data[0] = m_cass_data[1] = m_cass_data[2] = m_cass_data[3] = 0;
	m_cassbit = 0;
	m_cassold = 0;
	m_key_pressed = 0;
	membank("bankr")->set_entry(0);
	membank("bankw")->set_entry(0);
	m_uart->write_cts(0);
}

void fc100_state::port60_w(offs_t offset, uint8_t data)
{
	if (m_banksw_unlocked)
		membank("bankr")->set_entry(offset);
}

void fc100_state::port70_w(offs_t offset, uint8_t data)
{
	m_banksw_unlocked = (bool)offset;
}

void fc100_state::init_fc100()
{
	uint8_t *ram = memregion("ram")->base();
	uint8_t *cgen = memregion("chargen")->base()+0x800;

	membank("bankr")->configure_entry(0, &cgen[0]);
	membank("bankw")->configure_entry(0, &ram[0]);
	membank("bankr")->configure_entry(1, &ram[0]);
}

void fc100_state::fc100(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(7'159'090)/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &fc100_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &fc100_state::io_map);

	/* video hardware */
	M5C6847P1(config, m_vdg, XTAL(7'159'090)/3);  // Clock not verified
	m_vdg->set_screen("screen");
	m_vdg->input_callback().set(FUNC(fc100_state::mc6847_videoram_r));
	m_vdg->set_get_char_rom(FUNC(fc100_state::get_char_rom));
	m_vdg->set_get_fixed_mode(m5c6847p1_device::MODE_INTEXT);
	// other lines not connected

	SCREEN(config, "screen", SCREEN_TYPE_RASTER);
	GFXDECODE(config, "gfxdecode", "f4palette", gfx_fc100);
	PALETTE(config, "f4palette", palette_device::MONOCHROME);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	ay8910_device &psg(AY8910(config, "psg", XTAL(7'159'090)/3/2));  /* AY-3-8910 - clock not verified */
	psg.port_a_read_callback().set_ioport("JOY0");
	psg.port_b_read_callback().set_ioport("JOY1");
	//psg.port_a_write_callback().set(FUNC(fc100_state::ay_port_a_w));
	//psg.port_b_write_callback().set(FUNC(fc100_state::ay_port_b_w));
	psg.add_route(ALL_OUTPUTS, "mono", 1.50);

	/* Devices */
	CASSETTE(config, m_cass);
	m_cass->set_formats(fc100_cassette_formats);
	m_cass->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);

	I8251(config, m_uart, 0);
	m_uart->txd_handler().set([this] (bool state) { m_cassbit = state; });
	clock_device &uart_clock(CLOCK(config, "uart_clock", XTAL(4'915'200)/16/16)); // gives 19200
	uart_clock.signal_handler().set(m_uart, FUNC(i8251_device::write_txc));
	uart_clock.signal_handler().append(m_uart, FUNC(i8251_device::write_rxc));

	TIMER(config, "kansas_w").configure_periodic(FUNC(fc100_state::kansas_w), attotime::from_hz(4800)); // cass write
	TIMER(config, "kansas_r").configure_periodic(FUNC(fc100_state::kansas_r), attotime::from_hz(40000)); // cass read
	TIMER(config, "timer_k").configure_periodic(FUNC(fc100_state::timer_k), attotime::from_hz(300)); // keyb scan

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "fc100_cart");

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->ack_handler().set("cent_status_in", FUNC(input_buffer_device::write_bit4));
	m_centronics->busy_handler().set("cent_status_in", FUNC(input_buffer_device::write_bit5));

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);

	INPUT_BUFFER(config, "cent_status_in");
}

/* ROM definition */
ROM_START( fc100 )
	ROM_REGION( 0x6000, "roms", 0 )
	ROM_LOAD( "08-01.u48",     0x0000, 0x2000, CRC(24e78e75) SHA1(13121706544256a702635448ed2950a75c13f491) )
	ROM_LOAD( "08-02.u49",     0x2000, 0x2000, CRC(e14fc7e9) SHA1(9c5821e65c1efe698e25668d24c36929ea4c3ad7) )
	ROM_LOAD( "06-03.u50",     0x4000, 0x2000, CRC(d783c84e) SHA1(6d1bf53995e08724d5ecc24198cdda4442eb2eb9) )

	ROM_REGION( 0x800, "mcu", ROMREGION_ERASE00 )
	ROM_LOAD( "mcu.bin", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x800, "ram", ROMREGION_ERASE00 )

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "cg-04-01.u53",  0x0000, 0x1000, CRC(2de75b7f) SHA1(464369d98cbae92ffa322ebaa4404cf5b26825f1) )
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY     FULLNAME  FLAGS
CONS( 1982, fc100, 0,      0,      fc100,   fc100, fc100_state, init_fc100, "Goldstar", "FC-100", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
