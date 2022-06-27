// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    Bondwell 12/14

    12/05/2009 Skeleton driver.

    - Z80A CPU 4MHz
    - 64KB RAM (BW 12), 128KB RAM (BW 14)
    - 4KB ROM System
    - UPD765A Floppy controller
    - 2 x 5.25" Floppy drives 48 tpi SSDD (BW 12), DSDD (BW 14)
    - MC6845 Video controller
    - 2KB RAM Video buffer
    - 4KB ROM Character set
    - Z80SIO Serial interface
    - MC6821 Parallel interface
    - I8253 Counter-timer
    - MC1408 8-bit DAC sound
    - KB3600 PRO (AY-5-3600 PRO) Keyboard controller

    http://www.eld.leidenuniv.nl/~moene/Home/sitemap/
    http://www.baltissen.org/newhtm/schemas.htm

****************************************************************************/

#include "emu.h"
#include "bw12.h"
#include "bus/rs232/rs232.h"
#include "sound/dac.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

/*

    TODO:

    - floppy motor off timer

*/

void bw12_state::bankswitch()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	switch (m_curbank)
	{
	case 0: /* ROM */
		program.install_read_bank(0x0000, 0x7fff, m_bank);
		program.unmap_write(0x0000, 0x7fff);
		break;

	case 1: /* BK0 */
		program.install_readwrite_bank(0x0000, 0x7fff, m_bank);
		break;

	case 2: /* BK1 */
	case 3: /* BK2 */
		if (m_ram->size() > 64*1024)
		{
			program.install_readwrite_bank(0x0000, 0x7fff, m_bank);
		}
		else
		{
			program.unmap_readwrite(0x0000, 0x7fff);
		}
		break;
	}

	m_bank->set_entry(m_curbank);
}

void bw12_state::floppy_motor_on_off()
{
	if (m_motor0 || m_motor1)
	{
		m_motor_on = 1;
		m_floppy0->mon_w(0);
		m_floppy1->mon_w(0);
		m_floppy_timer->adjust(attotime::never);
	}
	else
		m_floppy_timer->adjust(attotime::from_msec(170));
}

TIMER_DEVICE_CALLBACK_MEMBER(bw12_state::floppy_motor_off_tick)
{
	if (m_motor0 || m_motor1)
		return;
	m_floppy0->mon_w(1);
	m_floppy1->mon_w(1);

	m_motor_on = 0;
}

WRITE_LINE_MEMBER(bw12_state::ls138_a0_w)
{
	m_curbank = (m_curbank & 0x02) | state;
	bankswitch();
}

WRITE_LINE_MEMBER(bw12_state::ls138_a1_w)
{
	m_curbank = (state << 1) | (m_curbank & 0x01);
	bankswitch();
}

WRITE_LINE_MEMBER(bw12_state::init_w)
{
}

WRITE_LINE_MEMBER(bw12_state::motor0_w)
{
	m_motor0 = state;
	floppy_motor_on_off();
}

WRITE_LINE_MEMBER(bw12_state::motor1_w)
{
	m_motor1 = state;
	floppy_motor_on_off();
}

uint8_t bw12_state::ls259_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		m_latch->write_bit(offset >> 1, BIT(offset, 0));

	return 0;
}

/* Memory Maps */

void bw12_state::bw12_mem(address_map &map)
{
	map(0x0000, 0x7fff).bankrw("bank");
	map(0x8000, 0xf7ff).ram();
	map(0xf800, 0xffff).ram().share("video_ram");
}

void bw12_state::bw12_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x0f).r(FUNC(bw12_state::ls259_r)).w(m_latch, FUNC(ls259_device::write_a0));
	map(0x10, 0x10).mirror(0x0e).w(m_crtc, FUNC(mc6845_device::address_w));
	map(0x11, 0x11).mirror(0x0e).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x20, 0x21).mirror(0x0e).m(m_fdc, FUNC(upd765a_device::map));
	map(0x30, 0x33).mirror(0x0c).rw(m_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x40, 0x43).mirror(0x0c).rw(m_sio, FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x50, 0x50).mirror(0x0f).w("dac", FUNC(dac_byte_interface::data_w));
	map(0x60, 0x63).mirror(0x0c).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
}

/* Input Ports */

static INPUT_PORTS_START( bw12 )
	/*

	  KB3600 PRO2 Keyboard matrix

	      | Y0  | Y1  | Y2  | Y3  | Y4  | Y5  | Y6  | Y7  | Y8  | Y9  |
	      |     |     |     |     |     |     |     |     |     |     |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X0  |  7  |  8  |  9  |  0  |  1  |  2  |  3  |  4  |  5  |  6  |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X1  |  U  |  I  |  O  |  P  |  Q  |  W  |  E  |  R  |  T  |  Y  |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X2  | F15 | F16 | RET | N.  | SP  | LOCK| F11 | F12 | F13 | F14 |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X3  | F7  | F8  | F9  | F10 | F1  | F2  | F3  | F4  | F5  | F6  |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X4  | LEFT|RIGHT| N3  | BS  |  @  |     |  -  |  ]  | UP  | DOWN|
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X5  | N9  | CL  | N2  | LF  | DEL | HT  |ARROW|  [  | N7  | N8  |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X6  |  M  |  ,  |  .  |  /  |  Z  |  X  |  C  |  V  |  B  |  N  |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X7  |  J  |  K  |  L  |  ;  |  A  |  S  |  D  |  F  |  G  |  H  |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|
	  X8  | N6  |  -  | N1  | N0  | ESC |     |  :  | NRET| N4  | N5  |
	  ----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----|

	*/

	PORT_START("X0")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('|')
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')

	PORT_START("X1")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')

	PORT_START("X2")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F15")
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F16")
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad .") PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CAPS LOCK") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F11") PORT_CODE(KEYCODE_F11) PORT_CHAR(UCHAR_MAMEKEY(F11))
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F12") PORT_CODE(KEYCODE_F12) PORT_CHAR(UCHAR_MAMEKEY(F12))
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F13")
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F14")

	PORT_START("X3")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F7") PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F8") PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F9") PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F10") PORT_CODE(KEYCODE_F10) PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F1") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F2") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F3") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F4") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F5") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F6") PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))

	PORT_START("X4")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("BS") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('@') PORT_CHAR('\\')
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))

	PORT_START("X5")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CL")
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LF")
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL)
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("HT")
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 8") PORT_CODE(KEYCODE_8_PAD)

	PORT_START("X6")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')

	PORT_START("X7")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')

	PORT_START("X8")
	PORT_BIT( 0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC)
	PORT_BIT( 0x020, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad RET") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 5") PORT_CODE(KEYCODE_5_PAD)

	PORT_START("MODIFIERS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
INPUT_PORTS_END

/* Video */

MC6845_UPDATE_ROW( bw12_state::crtc_update_row )
{
	pen_t const *const pen = m_palette->pens();

	for (int column = 0; column < x_count; column++)
	{
		uint8_t const code = m_video_ram[((ma + column) & BW12_VIDEORAM_MASK)];
		uint16_t const addr = code << 4 | (ra & 0x0f);
		uint8_t data = m_char_rom->base()[addr & BW12_CHARROM_MASK];

		if (column == cursor_x)
		{
			data = 0xff;
		}

		for (int bit = 0; bit < 8; bit++)
		{
			int const x = (column * 8) + bit;
			int const color = BIT(data, 7) && de;

			bitmap.pix(vbp + y, hbp + x) = pen[color];

			data <<= 1;
		}
	}
}


/* PIA6821 Interface */

WRITE_LINE_MEMBER( bw12_state::write_centronics_busy )
{
	m_centronics_busy = state;
}

WRITE_LINE_MEMBER( bw12_state::write_centronics_fault )
{
	m_centronics_fault = state;
}

WRITE_LINE_MEMBER( bw12_state::write_centronics_perror )
{
	m_centronics_perror = state;
}

uint8_t bw12_state::pia_pa_r()
{
	/*

	    bit     description

	    PA0     Input from Centronics BUSY status
	    PA1     Input from Centronics ERROR status
	    PA2     Input from Centronics PAPER OUT status
	    PA3     Input from FDC MOTOR
	    PA4     Input from PIT OUT2
	    PA5     Input from keyboard strobe
	    PA6     Input from keyboard serial data
	    PA7     Input from FDC interrupt

	*/

	uint8_t data = 0;

	data |= m_centronics_busy;
	data |= (m_centronics_fault << 1);
	data |= (m_centronics_perror << 2);
	data |= (m_motor_on << 3);
	data |= (m_pit_out2 << 4);
	data |= (m_key_stb << 5);
	data |= (m_key_sin << 6);
	data |= (m_fdc->get_irq() ? 1 : 0) << 7;

	return data;
}

WRITE_LINE_MEMBER( bw12_state::pia_cb2_w )
{
	if (state)
	{
		/* keyboard shift clock */
		m_key_shift++;

		if (m_key_shift < 9)
		{
			m_key_sin = m_key_data[m_key_shift];
		}
	}
}

/* PIT8253 Interface */

WRITE_LINE_MEMBER( bw12_state::pit_out2_w )
{
	m_pit_out2 = state;
}

/* AY-5-3600-PRO-002 Interface */

READ_LINE_MEMBER( bw12_state::ay3600_shift_r )
{
	return BIT(m_modifiers->read(), 0);
}

READ_LINE_MEMBER( bw12_state::ay3600_control_r )
{
	return BIT(m_modifiers->read(), 1);
}

WRITE_LINE_MEMBER( bw12_state::ay3600_data_ready_w )
{
	m_key_stb = state;

	m_pia->cb1_w(state);

	if (state)
	{
		uint16_t data = m_kbc->b_r();

		m_key_data[0] = BIT(data, 6);
		m_key_data[1] = BIT(data, 3);
		m_key_data[2] = BIT(data, 1);
		m_key_data[3] = BIT(data, 0);
		m_key_data[4] = BIT(data, 2);
		m_key_data[5] = BIT(data, 4);
		m_key_data[6] = BIT(data, 5);
		m_key_data[7] = BIT(data, 7);
		m_key_data[8] = BIT(data, 8);

		m_key_shift = 0;
		m_key_sin = m_key_data[m_key_shift];
	}
}

/* Machine Initialization */

void bw12_state::machine_start()
{
	/* setup memory banking */
	m_bank->configure_entry(0, m_rom->base());
	m_bank->configure_entry(1, m_ram->pointer());
	m_bank->configure_entries(2, 2, m_ram->pointer() + 0x10000, 0x8000);

	/* register for state saving */
	save_item(NAME(m_curbank));
	save_item(NAME(m_pit_out2));
	save_item(NAME(m_key_data));
	save_item(NAME(m_key_sin));
	save_item(NAME(m_key_stb));
	save_item(NAME(m_key_shift));
	save_item(NAME(m_motor_on));
	save_item(NAME(m_motor0));
	save_item(NAME(m_motor1));
	save_item(NAME(m_centronics_busy));
	save_item(NAME(m_centronics_fault));
	save_item(NAME(m_centronics_perror));
	machine().save().register_postload(save_prepost_delegate(FUNC(bw12_state::bankswitch), this));
}

void bw12_state::machine_reset()
{
	m_key_stb = 0;
	m_key_shift = 0;
}

static void bw12_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_SSDD);
}

void bw12_state::bw12_floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_BW12_FORMAT);
}

static void bw14_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}

void bw12_state::bw14_floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_BW12_FORMAT);
}


/* F4 Character Displayer */
static const gfx_layout bw12_charlayout =
{
	8, 9,                   /* 8 x 9 characters */
	256,                    /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( gfx_bw12 )
	GFXDECODE_ENTRY( "chargen", 0x0000, bw12_charlayout, 0, 1 )
GFXDECODE_END


/* Machine Driver */
void bw12_state::common(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 16_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &bw12_state::bw12_mem);
	m_maincpu->set_addrmap(AS_IO, &bw12_state::bw12_io);

	LS259(config, m_latch); // IC18
	m_latch->q_out_cb<0>().set(FUNC(bw12_state::ls138_a0_w)); // LS138 A0
	m_latch->q_out_cb<1>().set(FUNC(bw12_state::ls138_a1_w)); // LS138 A1
	// Q2 not connected
	m_latch->q_out_cb<3>().set(FUNC(bw12_state::init_w)); // _INIT
	m_latch->q_out_cb<4>().set_output("led0"); // CAP LOCK
	m_latch->q_out_cb<5>().set(FUNC(bw12_state::motor0_w)); // MOTOR 0
	m_latch->q_out_cb<6>().set(FUNC(bw12_state::motor1_w)); // MOTOR 1
	m_latch->q_out_cb<7>().set(m_fdc, FUNC(upd765a_device::tc_line_w)); // FDC TC

	/* video hardware */
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER, rgb_t::amber()));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_screen_update(MC6845_TAG, FUNC(mc6845_device::screen_update));
	screen.set_size(640, 200);
	screen.set_visarea(0, 640-1, 0, 200-1);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_bw12);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	MC6845(config, m_crtc, 16_MHz_XTAL / 8);
	m_crtc->set_screen(SCREEN_TAG);
	m_crtc->set_show_border_area(true);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(bw12_state::crtc_update_row));

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	MC1408(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.125); // ls273.ic5 + mc1408.ic4

	/* devices */
	TIMER(config, FLOPPY_TIMER_TAG).configure_generic(FUNC(bw12_state::floppy_motor_off_tick));
	UPD765A(config, m_fdc, 16_MHz_XTAL / 4, false, true);

	PIA6821(config, m_pia, 0);
	m_pia->readpa_handler().set(FUNC(bw12_state::pia_pa_r));
	m_pia->writepb_handler().set("cent_data_out", FUNC(output_latch_device::write));
	m_pia->ca2_handler().set(CENTRONICS_TAG, FUNC(centronics_device::write_strobe));
	m_pia->cb2_handler().set(FUNC(bw12_state::pia_cb2_w));
	m_pia->irqa_handler().set_inputline(Z80_TAG, INPUT_LINE_IRQ0);
	m_pia->irqb_handler().set_inputline(Z80_TAG, INPUT_LINE_IRQ0);

	Z80SIO(config, m_sio, 16_MHz_XTAL / 4); // SIO/0
	m_sio->out_txda_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_txd));
	m_sio->out_dtra_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_dtr));
	m_sio->out_rtsa_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_rts));
	m_sio->out_txdb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_txd));
	m_sio->out_dtrb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_dtr));
	m_sio->out_rtsb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_rts));
	m_sio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	PIT8253(config, m_pit);
	m_pit->set_clk<0>(1.8432_MHz_XTAL);
	m_pit->out_handler<0>().set(m_sio, FUNC(z80sio_device::rxca_w));
	m_pit->out_handler<0>().append(m_sio, FUNC(z80sio_device::txca_w));
	m_pit->out_handler<0>().append(RS232_A_TAG, FUNC(rs232_port_device::write_etc));
	m_pit->set_clk<1>(1.8432_MHz_XTAL);
	m_pit->out_handler<1>().set(m_sio, FUNC(z80sio_device::rxtxcb_w));
	m_pit->out_handler<1>().append(RS232_B_TAG, FUNC(rs232_port_device::write_etc));
	m_pit->set_clk<2>(1.8432_MHz_XTAL);
	m_pit->out_handler<2>().set(FUNC(bw12_state::pit_out2_w));

	AY3600(config, m_kbc, 0);
	m_kbc->x0().set_ioport("X0");
	m_kbc->x1().set_ioport("X1");
	m_kbc->x2().set_ioport("X2");
	m_kbc->x3().set_ioport("X3");
	m_kbc->x4().set_ioport("X4");
	m_kbc->x5().set_ioport("X5");
	m_kbc->x6().set_ioport("X6");
	m_kbc->x7().set_ioport("X7");
	m_kbc->x8().set_ioport("X8");
	m_kbc->shift().set(FUNC(bw12_state::ay3600_shift_r));
	m_kbc->control().set(FUNC(bw12_state::ay3600_control_r));
	m_kbc->data_ready().set(FUNC(bw12_state::ay3600_data_ready_w));

	rs232_port_device &rs232a(RS232_PORT(config, RS232_A_TAG, default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_sio, FUNC(z80sio_device::rxa_w));
	rs232a.dcd_handler().set(m_sio, FUNC(z80sio_device::dcda_w));
	rs232a.cts_handler().set(m_sio, FUNC(z80sio_device::ctsa_w));

	rs232_port_device &rs232b(RS232_PORT(config, RS232_B_TAG, default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_sio, FUNC(z80sio_device::rxb_w));
	rs232b.dcd_handler().set(m_sio, FUNC(z80sio_device::dcdb_w));
	rs232b.cts_handler().set(m_sio, FUNC(z80sio_device::ctsb_w));

	/* printer */
	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->ack_handler().set(m_pia, FUNC(pia6821_device::ca1_w));
	m_centronics->busy_handler().set(FUNC(bw12_state::write_centronics_busy));
	m_centronics->fault_handler().set(FUNC(bw12_state::write_centronics_fault));
	m_centronics->perror_handler().set(FUNC(bw12_state::write_centronics_perror));

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);
}

void bw12_state::bw12(machine_config &config)
{
	common(config);
	/* floppy drives */
	FLOPPY_CONNECTOR(config, UPD765_TAG ":1", bw12_floppies, "525dd", bw12_state::bw12_floppy_formats);
	FLOPPY_CONNECTOR(config, UPD765_TAG ":2", bw12_floppies, "525dd", bw12_state::bw12_floppy_formats);

	// software lists
	SOFTWARE_LIST(config, "flop_list").set_original("bw12");

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("64K");
}

void bw12_state::bw14(machine_config &config)
{
	common(config);
	/* floppy drives */
	FLOPPY_CONNECTOR(config, UPD765_TAG ":1", bw14_floppies, "525dd", bw12_state::bw14_floppy_formats);
	FLOPPY_CONNECTOR(config, UPD765_TAG ":2", bw14_floppies, "525dd", bw12_state::bw14_floppy_formats);

	// software lists
	SOFTWARE_LIST(config, "flop_list").set_original("bw14");

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("128K");
	}

/* ROMs */

ROM_START( bw12 )
	ROM_REGION( 0x10000, Z80_TAG, 0 )
	ROM_LOAD( "bw14boot.ic41", 0x0000, 0x1000, CRC(782fe341) SHA1(eefe5ad6b1ef77a1caf0af743b74de5fa1c4c19d) )

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "bw14char.ic1",  0x0000, 0x1000, CRC(f9dd68b5) SHA1(50132b759a6d84c22c387c39c0f57535cd380411) )
ROM_END

#define rom_bw14 rom_bw12

ROM_START( bw14d )
	ROM_REGION( 0x10000, Z80_TAG, 0 )
	ROM_LOAD( "bw14boot.ic41", 0x0000, 0x1000, CRC(782fe341) SHA1(eefe5ad6b1ef77a1caf0af743b74de5fa1c4c19d) )

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "gcrd.bin",  0x0000, 0x1000, CRC(638f3e1d) SHA1(5a0b2f47c66fe8db6f58d348ac29074a4db51258) )
ROM_END

/* System Drivers */

/*    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY             FULLNAME       FLAGS */
COMP( 1984, bw12,  0,      0,      bw12,    bw12,  bw12_state, empty_init, "Bondwell Holding", "Bondwell 12", MACHINE_SUPPORTS_SAVE )
COMP( 1984, bw14,  bw12,   0,      bw14,    bw12,  bw12_state, empty_init, "Bondwell Holding", "Bondwell 14", MACHINE_SUPPORTS_SAVE )
COMP( 1984, bw14d, bw12,   0,      bw14,    bw12,  bw12_state, empty_init, "Bondwell Holding", "Bondwell Portable Computer Model 14 (German keyboard)", MACHINE_SUPPORTS_SAVE )
