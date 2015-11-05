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

#include "includes/bw12.h"
#include "bus/rs232/rs232.h"
#include "softlist.h"
/*

    TODO:

    - floppy motor off timer

*/

void bw12_state::bankswitch()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	switch (m_bank)
	{
	case 0: /* ROM */
		program.install_read_bank(0x0000, 0x7fff, "bank1");
		program.unmap_write(0x0000, 0x7fff);
		break;

	case 1: /* BK0 */
		program.install_readwrite_bank(0x0000, 0x7fff, "bank1");
		break;

	case 2: /* BK1 */
	case 3: /* BK2 */
		if (m_ram->size() > 64*1024)
		{
			program.install_readwrite_bank(0x0000, 0x7fff, "bank1");
		}
		else
		{
			program.unmap_readwrite(0x0000, 0x7fff);
		}
		break;
	}

	membank("bank1")->set_entry(m_bank);
}

void bw12_state::floppy_motor_off()
{
	m_floppy0->mon_w(1);
	m_floppy1->mon_w(1);

	m_motor_on = 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(bw12_state::floppy_motor_off_tick)
{
	floppy_motor_off();
}

void bw12_state::set_floppy_motor_off_timer()
{
	if (m_motor0 || m_motor1)
	{
		m_motor_on = 1;
		m_floppy_timer->enable(false);
	}
	else
	{
		/* trigger floppy motor off NE556 timer */
		/*

		    R18 = RES_K(100)
		    C11 = CAP_U(4.7)

		*/

		//m_floppy_timer->adjust(attotime::zero);
		floppy_motor_off();
	}
}

void bw12_state::ls259_w(int address, int data)
{
	switch (address)
	{
	case 0: /* LS138 A0 */
		m_bank = (m_bank & 0x02) | data;
		bankswitch();
		break;

	case 1: /* LS138 A1 */
		m_bank = (data << 1) | (m_bank & 0x01);
		bankswitch();
		break;

	case 2: /* not connected */
		break;

	case 3: /* _INIT */
		break;

	case 4: /* CAP LOCK */
		output_set_led_value(0, data);
		break;

	case 5: /* MOTOR 0 */
		m_motor0 = data;
		break;

	case 6: /* MOTOR 1 */
		m_motor1 = data;
		break;

	case 7: /* FDC TC */
		m_fdc->tc_w(data);
		break;
	}

	m_motor_on = m_motor0 || m_motor1;

	m_floppy0->mon_w(!m_motor_on);
	m_floppy1->mon_w(!m_motor_on);
}

WRITE8_MEMBER( bw12_state::ls259_w )
{
	int d = BIT(offset, 0);
	int a = (offset >> 1) & 0x07;

	ls259_w(a, d);
}

READ8_MEMBER( bw12_state::ls259_r )
{
	ls259_w(space, offset, 0);

	return 0;
}

/* Memory Maps */

static ADDRESS_MAP_START( bw12_mem, AS_PROGRAM, 8, bw12_state )
	AM_RANGE(0x0000, 0x7fff) AM_RAMBANK("bank1")
	AM_RANGE(0x8000, 0xf7ff) AM_RAM
	AM_RANGE(0xf800, 0xffff) AM_RAM AM_SHARE("video_ram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( bw12_io, AS_IO, 8, bw12_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x0f) AM_READWRITE(ls259_r, ls259_w)
	AM_RANGE(0x10, 0x10) AM_MIRROR(0x0e) AM_DEVWRITE(MC6845_TAG, mc6845_device, address_w)
	AM_RANGE(0x11, 0x11) AM_MIRROR(0x0e) AM_DEVREADWRITE(MC6845_TAG, mc6845_device, register_r, register_w)
	AM_RANGE(0x20, 0x21) AM_MIRROR(0x0e) AM_DEVICE(UPD765_TAG, upd765a_device, map)
	AM_RANGE(0x30, 0x33) AM_MIRROR(0x0c) AM_DEVREADWRITE(PIA6821_TAG, pia6821_device, read, write)
	AM_RANGE(0x40, 0x43) AM_MIRROR(0x0c) AM_DEVREADWRITE(Z80SIO_TAG, z80sio0_device, ba_cd_r, ba_cd_w)
	AM_RANGE(0x50, 0x50) AM_MIRROR(0x0f) AM_DEVWRITE(MC1408_TAG, dac_device, write_unsigned8)
	AM_RANGE(0x60, 0x63) AM_MIRROR(0x0c) AM_DEVREADWRITE(PIT8253_TAG, pit8253_device, read, write)
ADDRESS_MAP_END

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
	const pen_t *pen = m_palette->pens();
	int column, bit;

	for (column = 0; column < x_count; column++)
	{
		UINT8 code = m_video_ram[((ma + column) & BW12_VIDEORAM_MASK)];
		UINT16 addr = code << 4 | (ra & 0x0f);
		UINT8 data = m_char_rom->base()[addr & BW12_CHARROM_MASK];

		if (column == cursor_x)
		{
			data = 0xff;
		}

		for (bit = 0; bit < 8; bit++)
		{
			int x = (column * 8) + bit;
			int color = BIT(data, 7) && de;

			bitmap.pix32(vbp + y, hbp + x) = pen[color];

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

READ8_MEMBER( bw12_state::pia_pa_r )
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

	UINT8 data = 0;

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

		if (m_key_shift < 10)
		{
			m_key_sin = m_key_data[m_key_shift];
		}
	}
}

/* PIT8253 Interface */

WRITE_LINE_MEMBER( bw12_state::pit_out0_w )
{
	m_sio->txca_w(state);
	m_sio->rxca_w(state);
}

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
		UINT16 data = m_kbc->b_r();

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
	membank("bank1")->configure_entry(0, m_rom->base());
	membank("bank1")->configure_entry(1, m_ram->pointer());
	membank("bank1")->configure_entries(2, 2, m_ram->pointer() + 0x10000, 0x8000);

	/* register for state saving */
	save_item(NAME(m_bank));
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
	for (int i = 0; i < 8; i++)
	{
		ls259_w(i, 0);
	}
}

static SLOT_INTERFACE_START( bw12_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_SSDD )
SLOT_INTERFACE_END

FLOPPY_FORMATS_MEMBER( bw12_state::bw12_floppy_formats )
	FLOPPY_BW12_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( bw14_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END

FLOPPY_FORMATS_MEMBER( bw12_state::bw14_floppy_formats )
	FLOPPY_BW12_FORMAT
FLOPPY_FORMATS_END


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

static GFXDECODE_START( bw12 )
	GFXDECODE_ENTRY( "chargen", 0x0000, bw12_charlayout, 0, 1 )
GFXDECODE_END


/* Machine Driver */
static MACHINE_CONFIG_START( common, bw12_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_16MHz/4)
	MCFG_CPU_PROGRAM_MAP(bw12_mem)
	MCFG_CPU_IO_MAP(bw12_io)

	/* video hardware */
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DEVICE(MC6845_TAG, mc6845_device, screen_update)
	MCFG_SCREEN_SIZE(640, 200)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 200-1)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", bw12)
	MCFG_PALETTE_ADD_MONOCHROME_AMBER("palette")

	MCFG_MC6845_ADD(MC6845_TAG, MC6845, SCREEN_TAG, XTAL_16MHz/8)
	MCFG_MC6845_SHOW_BORDER_AREA(true)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(bw12_state, crtc_update_row)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD(MC1408_TAG, DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* devices */
	MCFG_TIMER_DRIVER_ADD(FLOPPY_TIMER_TAG, bw12_state, floppy_motor_off_tick)
	MCFG_UPD765A_ADD(UPD765_TAG, false, true)

	MCFG_DEVICE_ADD(PIA6821_TAG, PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(bw12_state, pia_pa_r))
	MCFG_PIA_WRITEPB_HANDLER(DEVWRITE8("cent_data_out", output_latch_device, write))
	MCFG_PIA_CA2_HANDLER(DEVWRITELINE(CENTRONICS_TAG, centronics_device, write_strobe))
	MCFG_PIA_CB2_HANDLER(WRITELINE(bw12_state, pia_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE(Z80_TAG, z80_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE(Z80_TAG, z80_device, irq_line))

	MCFG_Z80SIO0_ADD(Z80SIO_TAG, XTAL_16MHz/4, 0, 0, 0, 0)
	MCFG_Z80DART_OUT_TXDA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_rts))
	MCFG_Z80DART_OUT_TXDB_CB(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRB_CB(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSB_CB(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_rts))
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD(PIT8253_TAG, PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL_1_8432MHz)
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(bw12_state, pit_out0_w))
	MCFG_PIT8253_CLK1(XTAL_1_8432MHz)
	MCFG_PIT8253_OUT1_HANDLER(DEVWRITELINE(Z80SIO_TAG, z80dart_device, rxtxcb_w))
	MCFG_PIT8253_CLK2(XTAL_1_8432MHz)
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(bw12_state, pit_out2_w))

	MCFG_DEVICE_ADD(AY3600PRO002_TAG, AY3600, 0)
	MCFG_AY3600_MATRIX_X0(IOPORT("X0"))
	MCFG_AY3600_MATRIX_X1(IOPORT("X1"))
	MCFG_AY3600_MATRIX_X2(IOPORT("X2"))
	MCFG_AY3600_MATRIX_X3(IOPORT("X3"))
	MCFG_AY3600_MATRIX_X4(IOPORT("X4"))
	MCFG_AY3600_MATRIX_X5(IOPORT("X5"))
	MCFG_AY3600_MATRIX_X6(IOPORT("X6"))
	MCFG_AY3600_MATRIX_X7(IOPORT("X7"))
	MCFG_AY3600_MATRIX_X8(IOPORT("X8"))
	MCFG_AY3600_SHIFT_CB(READLINE(bw12_state, ay3600_shift_r))
	MCFG_AY3600_CONTROL_CB(READLINE(bw12_state, ay3600_control_r))
	MCFG_AY3600_DATA_READY_CB(WRITELINE(bw12_state, ay3600_data_ready_w))

	MCFG_RS232_PORT_ADD(RS232_A_TAG, default_rs232_devices, NULL)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(Z80SIO_TAG, z80dart_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(Z80SIO_TAG, z80dart_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(Z80SIO_TAG, z80dart_device, ctsa_w))

	MCFG_RS232_PORT_ADD(RS232_B_TAG, default_rs232_devices, NULL)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(Z80SIO_TAG, z80dart_device, rxb_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(Z80SIO_TAG, z80dart_device, dcdb_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(Z80SIO_TAG, z80dart_device, ctsb_w))

	/* printer */
	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_ACK_HANDLER(DEVWRITELINE(PIA6821_TAG, pia6821_device, ca1_w))
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(bw12_state, write_centronics_busy))
	MCFG_CENTRONICS_FAULT_HANDLER(WRITELINE(bw12_state, write_centronics_fault))
	MCFG_CENTRONICS_PERROR_HANDLER(WRITELINE(bw12_state, write_centronics_perror))

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", CENTRONICS_TAG)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( bw12, common )
	/* floppy drives */
	MCFG_FLOPPY_DRIVE_ADD(UPD765_TAG ":1", bw12_floppies, "525dd", bw12_state::bw12_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(UPD765_TAG ":2", bw12_floppies, "525dd", bw12_state::bw12_floppy_formats)

	// software lists
	MCFG_SOFTWARE_LIST_ADD("flop_list", "bw12")

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( bw14, common )
	/* floppy drives */
	MCFG_FLOPPY_DRIVE_ADD(UPD765_TAG ":1", bw14_floppies, "525dd", bw12_state::bw14_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(UPD765_TAG ":2", bw14_floppies, "525dd", bw12_state::bw14_floppy_formats)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")
MACHINE_CONFIG_END

/* ROMs */

ROM_START( bw12 )
	ROM_REGION( 0x10000, Z80_TAG, 0 )
	ROM_LOAD( "bw14boot.ic41", 0x0000, 0x1000, CRC(782fe341) SHA1(eefe5ad6b1ef77a1caf0af743b74de5fa1c4c19d) )

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "bw14char.ic1",  0x0000, 0x1000, CRC(f9dd68b5) SHA1(50132b759a6d84c22c387c39c0f57535cd380411) )
ROM_END

#define rom_bw14 rom_bw12

/* System Drivers */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE INPUT   INIT    COMPANY               FULLNAME        FLAGS */
COMP( 1984, bw12,   0,      0,      bw12,   bw12, driver_device,   0,      "Bondwell Holding",   "Bondwell 12", MACHINE_SUPPORTS_SAVE )
COMP( 1984, bw14,   bw12,   0,      bw14,   bw12, driver_device,   0,      "Bondwell Holding",   "Bondwell 14", MACHINE_SUPPORTS_SAVE )
