/*

    Tandy Radio Shack TRS-80 Model II/12/16/16B/6000

    http://home.iae.nl/users/pb0aia/cm/modelii.html

*/

/*

    TODO:

    - keyboard CPU ROM
    - graphics board
    - Tandy 6000 HD

        chdman -createblankhd tandy6000hd.chd 306 6 34 256

*/

#include "includes/trs80m2.h"



//**************************************************************************
//  KEYBOARD HACK
//**************************************************************************

static const UINT8 trs80m2_keycodes[3][9][8] =
{
	// unshifted
	{
	{ 0x1e, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37 },
	{ 0x38, 0x39, 0x30, 0x2d, 0x3d, 0x08, 0x7f, 0x2d },
	{ 0x37, 0x38, 0x39, 0x09, 0x71, 0x77, 0x65, 0x72 },
	{ 0x74, 0x79, 0x75, 0x69, 0x6f, 0x70, 0x5b, 0x5d },
	{ 0x1b, 0x2b, 0x34, 0x35, 0x36, 0x61, 0x73, 0x64 },
	{ 0x66, 0x67, 0x68, 0x6a, 0x6b, 0x6c, 0x3b, 0x27 },
	{ 0x0d, 0x0a, 0x01, 0x31, 0x32, 0x33, 0x7a, 0x78 },
	{ 0x63, 0x76, 0x62, 0x6e, 0x6d, 0x2c, 0x2e, 0x2f },
	{ 0x04, 0x02, 0x03, 0x30, 0x2e, 0x20, 0x00, 0x00 }
	},

	// shifted
	{
	{ 0x1e, 0x21, 0x40, 0x23, 0x24, 0x25, 0x5e, 0x26 },
	{ 0x2a, 0x28, 0x29, 0x5f, 0x2b, 0x08, 0x7f, 0x2d },
	{ 0x37, 0x38, 0x39, 0x09, 0x51, 0x57, 0x45, 0x52 },
	{ 0x54, 0x59, 0x55, 0x49, 0x4f, 0x50, 0x7b, 0x7d },
	{ 0x1b, 0x2b, 0x34, 0x35, 0x36, 0x41, 0x53, 0x44 },
	{ 0x46, 0x47, 0x48, 0x4a, 0x4b, 0x4c, 0x3a, 0x22 },
	{ 0x0d, 0x0a, 0x01, 0x31, 0x32, 0x33, 0x5a, 0x58 },
	{ 0x43, 0x56, 0x42, 0x4e, 0x4d, 0x3c, 0x3e, 0x3f },
	{ 0x04, 0x02, 0x03, 0x30, 0x2e, 0x20, 0x00, 0x00 }
	},

	// control
	{
	{ 0x9e, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97 },
	{ 0x98, 0x99, 0x90, 0x1f, 0x9a, 0x88, 0xff, 0xad },
	{ 0xb7, 0xb8, 0xb9, 0x89, 0x11, 0x17, 0x05, 0x12 },
	{ 0x14, 0x19, 0x15, 0x09, 0x0f, 0x10, 0x1b, 0x1d },
	{ 0x9b, 0xab, 0xb4, 0xb5, 0xb6, 0x01, 0x13, 0x04 },
	{ 0x06, 0x07, 0x08, 0x0a, 0x0b, 0x0c, 0x7e, 0x60 },
	{ 0x8d, 0x8a, 0x81, 0xb1, 0xb2, 0xb3, 0x1a, 0x18 },
	{ 0x03, 0x16, 0x02, 0x0e, 0x0d, 0x1c, 0x7c, 0x5c },
	{ 0x84, 0x82, 0x83, 0xb0, 0xae, 0x00, 0x00, 0x00 }
	}
};

void trs80m2_state::scan_keyboard()
{
	if (!m_kbirq) return;

	static const char *const keynames[] = { "ROW0", "ROW1", "ROW2", "ROW3", "ROW4", "ROW5", "ROW6", "ROW7", "ROW8" };
	int table = 0, row, col;

	if (ioport("ROW9")->read() & 0x07)
	{
		// shift, upper case
		table = 1;
	}

	if (ioport("ROW9")->read() & 0x18)
	{
		// ctrl
		table = 2;
	}

	// scan keyboard
	for (row = 0; row < 9; row++)
	{
		UINT8 data = ioport(keynames[row])->read();

		for (col = 0; col < 8; col++)
		{
			if (BIT(data, col))
			{
				// latch key data
				m_key_data = trs80m2_keycodes[table][row][col];

				// trigger keyboard interrupt
				m_kbirq = 0;
				m_ctc->trg3(m_kbirq);

				return;
			}
		}
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(trs80m2_state::trs80m2_keyboard_tick)
{
	scan_keyboard();
}



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ8_MEMBER( trs80m2_state::read )
{
	UINT8 data = 0;

	if (offset < 0x800)
	{
		if (m_boot_rom)
		{
			data = memregion(Z80_TAG)->base()[offset];
		}
		else
		{
			data = m_ram->pointer()[offset];
		}
	}
	else if (offset < 0x8000)
	{
		data = m_ram->pointer()[offset];
	}
	else
	{
		if (m_msel && offset >= 0xf800)
		{
			data = m_video_ram[offset & 0x7ff];
		}
		else if (m_bank)
		{
			offs_t addr = (m_bank << 15) | (offset & 0x7fff);

			if (addr < m_ram->size())
			{
				data = m_ram->pointer()[addr];
			}
		}
	}

	return data;
}

WRITE8_MEMBER( trs80m2_state::write )
{
	if (offset < 0x8000)
	{
		m_ram->pointer()[offset] = data;
	}
	else
	{
		if (m_msel && offset >= 0xf800)
		{
			m_video_ram[offset & 0x7ff] = data;
		}
		else if (m_bank)
		{
			offs_t addr = (m_bank << 15) | (offset & 0x7fff);

			if (addr < m_ram->size())
			{
				m_ram->pointer()[addr] = data;
			}
		}
	}
}

WRITE8_MEMBER( trs80m2_state::rom_enable_w )
{
	/*

        bit     description

        0       BOOT ROM
        1
        2
        3
        4
        5
        6
        7

    */

	m_boot_rom = BIT(data, 0);
}

WRITE8_MEMBER( trs80m2_state::drvslt_w )
{
	/*

        bit     signal

        0       DS1
        1       DS2
        2       DS3
        3       DS4
        4
        5
        6       SDSEL
        7       FM/MFM

    */

	// drive select
	m_floppy = NULL;

	if (!BIT(data, 0)) m_floppy = m_floppy0->get_device();
	if (!BIT(data, 1)) m_floppy = m_floppy1->get_device();
	if (!BIT(data, 2)) m_floppy = m_floppy2->get_device();
	if (!BIT(data, 3)) m_floppy = m_floppy3->get_device();

	m_fdc->set_floppy(m_floppy);

	// side select
	if (m_floppy) m_floppy->ss_w(!BIT(data, 6));

	// FM/MFM
	m_fdc->dden_w(BIT(data, 7));
}

READ8_MEMBER( trs80m2_state::keyboard_r )
{
	// clear keyboard interrupt
	if (!m_kbirq)
	{
		m_kbirq = 1;
		m_ctc->trg3(m_kbirq);
		m_kb->busy_w(m_kbirq);
	}

	m_key_bit = 0;

	return m_key_data;
}

READ8_MEMBER( trs80m2_state::rtc_r )
{
	// clear RTC interrupt
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);

	return 0;
}

READ8_MEMBER( trs80m2_state::nmi_r )
{
	/*

        bit     signal              description

        0
        1
        2
        3
        4       80/40 CHAR EN       80/40 character mode
        5       ENABLE RTC INT      RTC interrupt enable
        6       DE                  display enabled
        7       KBIRQ               keyboard interrupt

    */

	UINT8 data = 0;

	// 80/40 character mode*/
	data |= m_80_40_char_en << 4;

	// RTC interrupt enable
	data |= m_enable_rtc_int << 5;

	// display enabled
	data |= m_de << 6;

	// keyboard interrupt
	data |= !m_kbirq << 7;

	return data;
}

WRITE8_MEMBER( trs80m2_state::nmi_w )
{
	/*

        bit     signal              description

        0                           memory bank select bit 0
        1                           memory bank select bit 1
        2                           memory bank select bit 2
        3                           memory bank select bit 3
        4       80/40 CHAR EN       80/40 character mode
        5       ENABLE RTC INT      RTC interrupt enable
        6       BLNKVID             video display enable
        7                           video RAM enable

    */

	// memory bank select
	m_bank = data & 0x0f;

	// 80/40 character mode
	m_80_40_char_en = BIT(data, 4);
	m_crtc->set_clock(m_80_40_char_en ? XTAL_12_48MHz/16 : XTAL_12_48MHz/8);

	// RTC interrupt enable
	m_enable_rtc_int = BIT(data, 5);

	if (m_enable_rtc_int && m_rtc_int)
	{
		// trigger RTC interrupt
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	}

	// video display enable
	m_blnkvid = BIT(data, 6);

	// video RAM enable
	m_msel = BIT(data, 7);
}

READ8_MEMBER( trs80m2_state::fdc_r )
{
	return m_fdc->gen_r(offset) ^ 0xff;
}

WRITE8_MEMBER( trs80m2_state::fdc_w )
{
	m_fdc->gen_w(offset, data ^ 0xff);
}

WRITE8_MEMBER( trs80m16_state::tcl_w )
{
	/*

        bit     description

        0       CONT0
        1       CONT1
        2       HALT
        3       RESET
        4       CONT4
        5       CONT5
        6       CONT6
        7       A14

    */

	m_subcpu->set_input_line(INPUT_LINE_HALT, BIT(data, 2) ? ASSERT_LINE : CLEAR_LINE);
	m_subcpu->set_input_line(INPUT_LINE_RESET, BIT(data, 3) ? ASSERT_LINE : CLEAR_LINE);

	pic8259_ir0_w(m_pic, BIT(data, 4));
	pic8259_ir1_w(m_pic, BIT(data, 5));
	pic8259_ir2_w(m_pic, BIT(data, 6));

	m_ual = (m_ual & 0x1fe) | BIT(data, 7);
}

WRITE8_MEMBER( trs80m16_state::ual_w )
{
	/*

        bit     description

        0       A15
        1       A16
        2       A17
        3       A18
        4       A19
        5       A20
        6       A21
        7       A22

    */

	m_ual = (data << 1) | BIT(m_ual, 0);
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( z80_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( z80_mem, AS_PROGRAM, 8, trs80m2_state )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(read, write)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( z80_io )
//-------------------------------------------------

static ADDRESS_MAP_START( z80_io, AS_IO, 8, trs80m2_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xe0, 0xe3) AM_DEVREADWRITE(Z80PIO_TAG, z80pio_device, read, write)
	AM_RANGE(0xe4, 0xe7) AM_READWRITE(fdc_r, fdc_w)
	AM_RANGE(0xef, 0xef) AM_WRITE(drvslt_w)
	AM_RANGE(0xf0, 0xf3) AM_DEVREADWRITE(Z80CTC_TAG, z80ctc_device, read, write)
	AM_RANGE(0xf4, 0xf7) AM_DEVREADWRITE_LEGACY(Z80SIO_TAG, z80dart_cd_ba_r, z80dart_cd_ba_w)
	AM_RANGE(0xf8, 0xf8) AM_DEVREADWRITE_LEGACY(Z80DMA_TAG, z80dma_r, z80dma_w)
	AM_RANGE(0xf9, 0xf9) AM_WRITE(rom_enable_w)
	AM_RANGE(0xfc, 0xfc) AM_READ(keyboard_r) AM_DEVWRITE(MC6845_TAG, mc6845_device, address_w)
	AM_RANGE(0xfd, 0xfd) AM_DEVREADWRITE(MC6845_TAG, mc6845_device, register_r, register_w)
	AM_RANGE(0xfe, 0xfe) AM_READ(rtc_r)
	AM_RANGE(0xff, 0xff) AM_READWRITE(nmi_r, nmi_w)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( m16_z80_io )
//-------------------------------------------------

static ADDRESS_MAP_START( m16_z80_io, AS_IO, 8, trs80m16_state )
	AM_IMPORT_FROM(z80_io)
	AM_RANGE(0xde, 0xde) AM_WRITE(tcl_w)
	AM_RANGE(0xdf, 0xdf) AM_WRITE(ual_w)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( m68000_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( m68000_mem, AS_PROGRAM, 16, trs80m2_state )
//  AM_RANGE(0x7800d0, 0x7800d1) PIC
//  AM_RANGE(0x7800d2, 0x7800d3) limit/offset 2
//  AM_RANGE(0x7800d4, 0x7800d5) limit/offset 1
//  AM_RANGE(0x7800d6, 0x7800d7) Z80 IRQ
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( trs80m2 )
//-------------------------------------------------

static INPUT_PORTS_START( trs80m2 )
	PORT_START("ROW0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_NAME("HELP") PORT_CODE(KEYCODE_TILDE)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')

	PORT_START("ROW1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_NAME("BACKSPACE") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_NAME("Keypad -") PORT_CODE(KEYCODE_MINUS_PAD)

	PORT_START("ROW2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_NAME("Keypad 7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_NAME("Keypad 8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_NAME("Keypad 9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_NAME("TAB") PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')

	PORT_START("ROW3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')

	PORT_START("ROW4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_NAME("Keypad +") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_NAME("Keypad 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_NAME("Keypad 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_NAME("Keypad 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')

	PORT_START("ROW5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('"')

	PORT_START("ROW6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('\r')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_NAME("LINE FEED") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_NAME("Keypad 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_NAME("Keypad 2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_NAME("Keypad 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')

	PORT_START("ROW7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("ROW8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_NAME("Keypad 0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_NAME("Keypad .") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_IMPULSE(1) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD )

	PORT_START("ROW9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LEFT SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RIGHT SHIFT") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LEFT CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RIGHT CTRL") PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))
INPUT_PORTS_END



//**************************************************************************
//  VIDEO
//**************************************************************************

static MC6845_UPDATE_ROW( trs80m2_update_row )
{
	trs80m2_state *state = device->machine().driver_data<trs80m2_state>();

	int x = 0;

	for (int column = 0; column < x_count; column++)
	{
		UINT8 code = state->m_video_ram[(ma + column) & 0x7ff];
		offs_t address = ((code & 0x7f) << 4) | (ra & 0x0f);
		UINT8 data = state->m_char_rom[address];

		int dcursor = (column == cursor_x);
		int drevid = BIT(code, 7);

		for (int bit = 0; bit < 8; bit++)
		{
			int dout = BIT(data, 7);
			int color = dcursor ^ drevid ^ dout;

			bitmap.pix32(y, x++) = RGB_MONOCHROME_GREEN[color];

			data <<= 1;
		}
	}
}

WRITE_LINE_MEMBER( trs80m2_state::de_w )
{
	m_de = state;
}

WRITE_LINE_MEMBER( trs80m2_state::vsync_w )
{
	if (state)
	{
		m_rtc_int = !m_rtc_int;

		if (m_enable_rtc_int && m_rtc_int)
		{
			// trigger RTC interrupt
			m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
		}
	}
}

static const mc6845_interface mc6845_intf =
{
	SCREEN_TAG,
	8,
	NULL,
	trs80m2_update_row,
	NULL,
	DEVCB_DRIVER_LINE_MEMBER(trs80m2_state, de_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_LINE_MEMBER(trs80m2_state, vsync_w),
	NULL
};

void trs80m2_state::video_start()
{
	// find memory regions
	m_char_rom = memregion(MC6845_TAG)->base();

	// allocate memory
	m_video_ram = auto_alloc_array(machine(), UINT8, 0x800);
}

UINT32 trs80m2_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_blnkvid)
	{
		bitmap.fill(RGB_BLACK, cliprect);
	}
	else
	{
		m_crtc->screen_update(screen, bitmap, cliprect);
	}

	return 0;
}



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  TRS80M2_KEYBOARD_INTERFACE( kb_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( trs80m2_state::kb_clock_w )
{
	int kbdata = m_kb->data_r();

	if (m_key_bit == 8)
	{
		if (!m_kbdata && kbdata)
		{
			// trigger keyboard interrupt
			m_kbirq = 0;
			m_ctc->trg3(m_kbirq);
			m_kb->busy_w(m_kbirq);
		}
	}
	else
	{
		if (!m_kbclk && state)
		{
			// shift in keyboard data bit
			m_key_data <<= 1;
			m_key_data |= kbdata;
			m_key_bit++;
		}
	}

	m_kbdata = kbdata;
	m_kbclk = state;
}

static TRS80M2_KEYBOARD_INTERFACE( kb_intf )
{
	DEVCB_DRIVER_LINE_MEMBER(trs80m2_state, kb_clock_w)
};


//-------------------------------------------------
//  Z80DMA_INTERFACE( dma_intf )
//-------------------------------------------------

static UINT8 memory_read_byte(address_space &space, offs_t address, UINT8 mem_mask) { return space.read_byte(address); }
static void memory_write_byte(address_space &space, offs_t address, UINT8 data, UINT8 mem_mask) { space.write_byte(address, data); }

static Z80DMA_INTERFACE( dma_intf )
{
	DEVCB_CPU_INPUT_LINE(Z80_TAG, INPUT_LINE_HALT),
	DEVCB_CPU_INPUT_LINE(Z80_TAG, INPUT_LINE_IRQ0),
	DEVCB_NULL,
	DEVCB_MEMORY_HANDLER(Z80_TAG, PROGRAM, memory_read_byte),
	DEVCB_MEMORY_HANDLER(Z80_TAG, PROGRAM, memory_write_byte),
	DEVCB_MEMORY_HANDLER(Z80_TAG, IO, memory_read_byte),
	DEVCB_MEMORY_HANDLER(Z80_TAG, IO, memory_write_byte)
};


//-------------------------------------------------
//  Z80PIO_INTERFACE( pio_intf )
//-------------------------------------------------

READ8_MEMBER( trs80m2_state::pio_pa_r )
{
	/*

        bit     signal      description

        0       INTRQ       FDC INT request
        1       _TWOSID     2-sided diskette
        2       _DSKCHG     disk change
        3       PRIME       prime
        4       FAULT       printer fault
        5       PSEL        printer select
        6       PE          paper empty
        7       BUSY        printer busy

    */

	UINT8 data = 0;

	// floppy interrupt
	data |= m_fdc->intrq_r();

	// 2-sided diskette
	data |= (m_floppy ? m_floppy->twosid_r() : 1) << 1;

	// disk change
	data |= (m_floppy ? m_floppy->dskchg_r() : 1) << 2;

	// printer fault
	data |= m_centronics->fault_r() << 4;

	// paper empty
	data |= !m_centronics->pe_r() << 6;

	// printer busy
	data |= m_centronics->busy_r() << 7;

	return data;
}

WRITE8_MEMBER( trs80m2_state::pio_pa_w )
{
	/*

        bit     signal      description

        0       INTRQ       FDC INT request
        1       _TWOSID     2-sided diskette
        2       _DSKCHG     disk change
        3       PRIME       prime
        4       FAULT       printer fault
        5       PSEL        printer select
        6       PE          paper empty
        7       BUSY        printer busy

    */

	// prime
	m_centronics->init_prime_w(BIT(data, 3));
}

WRITE_LINE_MEMBER( trs80m2_state::strobe_w )
{
	m_centronics->strobe_w(!state);
}

static Z80PIO_INTERFACE( pio_intf )
{
	DEVCB_CPU_INPUT_LINE(Z80_TAG, INPUT_LINE_IRQ0),				// interrupt callback
	DEVCB_DRIVER_MEMBER(trs80m2_state, pio_pa_r),				// port A read callback
	DEVCB_DRIVER_MEMBER(trs80m2_state, pio_pa_w),				// port A write callback
	DEVCB_NULL,													// port A ready callback
	DEVCB_NULL,													// port B read callback
	DEVCB_DEVICE_MEMBER(CENTRONICS_TAG, centronics_device, write),	// port B write callback
	DEVCB_DRIVER_LINE_MEMBER(trs80m2_state, strobe_w)			// port B ready callback
};


//-------------------------------------------------
//  centronics_interface centronics_intf
//-------------------------------------------------

static const centronics_interface centronics_intf =
{
	DEVCB_DEVICE_LINE_MEMBER(Z80PIO_TAG, z80pio_device, strobe_b),	// ACK output
	DEVCB_NULL,										// BUSY output
	DEVCB_NULL										// NOT BUSY output
};


//-------------------------------------------------
//  Z80DART_INTERFACE( sio_intf )
//-------------------------------------------------

static Z80DART_INTERFACE( sio_intf )
{
	0, 0, 0, 0,

	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_CPU_INPUT_LINE(Z80_TAG, INPUT_LINE_IRQ0)
};


//-------------------------------------------------
//  Z80CTC_INTERFACE( ctc_intf )
//-------------------------------------------------

TIMER_DEVICE_CALLBACK_MEMBER(trs80m2_state::ctc_tick)
{
	m_ctc->trg0(1);
	m_ctc->trg0(0);

	m_ctc->trg1(1);
	m_ctc->trg1(0);

	m_ctc->trg2(1);
	m_ctc->trg2(0);
}

static Z80CTC_INTERFACE( ctc_intf )
{
	DEVCB_CPU_INPUT_LINE(Z80_TAG, INPUT_LINE_IRQ0),	// interrupt handler
	DEVCB_DEVICE_LINE(Z80SIO_TAG, z80dart_rxca_w),	// ZC/TO0 callback
	DEVCB_DEVICE_LINE(Z80SIO_TAG, z80dart_txca_w),	// ZC/TO1 callback
	DEVCB_DEVICE_LINE(Z80SIO_TAG, z80dart_rxtxcb_w)	// ZC/TO2 callback
};


//-------------------------------------------------
//  wd17xx_interface fdc_intf
//-------------------------------------------------

static SLOT_INTERFACE_START( trs80m2_floppies )
	SLOT_INTERFACE( "8ssdd", FLOPPY_8_SSDD ) // Shugart SA-800
	SLOT_INTERFACE( "8dsdd", FLOPPY_8_DSDD ) // Shugart SA-850
SLOT_INTERFACE_END

void trs80m2_state::fdc_intrq_w(bool state)
{
	m_pio->port_a_write(state);
}

void trs80m2_state::fdc_drq_w(bool state)
{
	m_dmac->rdy_w(state);
}


//-------------------------------------------------
//  z80_daisy_config trs80m2_daisy_chain
//-------------------------------------------------

static const z80_daisy_config trs80m2_daisy_chain[] =
{
	{ Z80CTC_TAG },
	{ Z80SIO_TAG },
	{ Z80DMA_TAG },
	{ Z80PIO_TAG },
	{ NULL }
};


//-------------------------------------------------
//  pic8259_interface pic_intf
//-------------------------------------------------

static IRQ_CALLBACK( trs80m16_irq_callback )
{
	trs80m16_state *state = device->machine().driver_data<trs80m16_state>();

	return pic8259_acknowledge(state->m_pic);
}

static const struct pic8259_interface pic_intf =
{
	DEVCB_CPU_INPUT_LINE(M68000_TAG, M68K_IRQ_5),
	DEVCB_LINE_VCC,
	DEVCB_NULL
};



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  machine_start
//-------------------------------------------------

void trs80m2_state::machine_start()
{
	// floppy callbacks
	m_fdc->setup_intrq_cb(wd_fdc_t::line_cb(FUNC(trs80m2_state::fdc_intrq_w), this));
	m_fdc->setup_drq_cb(wd_fdc_t::line_cb(FUNC(trs80m2_state::fdc_drq_w), this));

	// register for state saving
	save_item(NAME(m_boot_rom));
	save_item(NAME(m_bank));
	save_item(NAME(m_msel));
	save_item(NAME(m_key_latch));
	save_item(NAME(m_key_data));
	save_item(NAME(m_key_bit));
	save_item(NAME(m_kbclk));
	save_item(NAME(m_kbdata));
	save_item(NAME(m_kbirq));
	save_item(NAME(m_blnkvid));
	save_item(NAME(m_80_40_char_en));
	save_item(NAME(m_de));
	save_item(NAME(m_rtc_int));
	save_item(NAME(m_enable_rtc_int));
}

void trs80m16_state::machine_start()
{
	trs80m2_state::machine_start();

	// register CPU IRQ callback
	m_maincpu->set_irq_acknowledge_callback(trs80m16_irq_callback);

	// register for state saving
	save_item(NAME(m_ual));
	save_item(NAME(m_limit));
	save_item(NAME(m_offset));
}


//-------------------------------------------------
//  machine_reset
//-------------------------------------------------

void trs80m2_state::machine_reset()
{
	// clear keyboard interrupt
	m_kbirq = 1;
	m_ctc->trg3(m_kbirq);
	m_kb->busy_w(m_kbirq);

	// enable boot ROM
	m_boot_rom = 1;

	// disable video RAM
	m_msel = 0;
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( trs80m2 )
//-------------------------------------------------

static MACHINE_CONFIG_START( trs80m2, trs80m2_state )
	// basic machine hardware
    MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_8MHz/2)
	MCFG_CPU_CONFIG(trs80m2_daisy_chain)
    MCFG_CPU_PROGRAM_MAP(z80_mem)
    MCFG_CPU_IO_MAP(z80_io)

	// video hardware
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_UPDATE_DRIVER(trs80m2_state, screen_update)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 479)

	MCFG_PALETTE_LENGTH(2)
	MCFG_PALETTE_INIT(black_and_white)

	MCFG_MC6845_ADD(MC6845_TAG, MC6845, XTAL_12_48MHz/8, mc6845_intf)

	// devices
	MCFG_FD1791x_ADD(FD1791_TAG, XTAL_8MHz/8)
	MCFG_Z80CTC_ADD(Z80CTC_TAG, XTAL_8MHz/2, ctc_intf)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("ctc", trs80m2_state, ctc_tick, attotime::from_hz(XTAL_8MHz/2/2))
	MCFG_Z80DMA_ADD(Z80DMA_TAG, XTAL_8MHz/2, dma_intf)
	MCFG_Z80PIO_ADD(Z80PIO_TAG, XTAL_8MHz/2, pio_intf)
	MCFG_Z80SIO0_ADD(Z80SIO_TAG, XTAL_8MHz/2, sio_intf)
	MCFG_CENTRONICS_PRINTER_ADD(CENTRONICS_TAG, centronics_intf)
	MCFG_FLOPPY_DRIVE_ADD(FD1791_TAG":0", trs80m2_floppies, "8dsdd", NULL, floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FD1791_TAG":1", trs80m2_floppies, NULL,    NULL, floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FD1791_TAG":2", trs80m2_floppies, NULL,    NULL, floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FD1791_TAG":3", trs80m2_floppies, NULL,    NULL, floppy_image_device::default_floppy_formats)
	MCFG_TRS80M2_KEYBOARD_ADD(kb_intf)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("keyboard", trs80m2_state, trs80m2_keyboard_tick, attotime::from_hz(60))

	// internal RAM
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("32K")
	MCFG_RAM_EXTRA_OPTIONS("64K,96K,128K,160K,192K,224K,256K,288K,320K,352K,384K,416K,448K,480K,512K")

	// software list
	MCFG_SOFTWARE_LIST_ADD("flop_list", "trs80m2")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( trs80m16 )
//-------------------------------------------------

static MACHINE_CONFIG_START( trs80m16, trs80m16_state )
	// basic machine hardware
    MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_8MHz/2)
	MCFG_CPU_CONFIG(trs80m2_daisy_chain)
    MCFG_CPU_PROGRAM_MAP(z80_mem)
    MCFG_CPU_IO_MAP(m16_z80_io)

	MCFG_CPU_ADD(M68000_TAG, M68000, XTAL_24MHz/4)
	MCFG_CPU_PROGRAM_MAP(m68000_mem)
	MCFG_DEVICE_DISABLE()

	// video hardware
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_UPDATE_DRIVER(trs80m2_state, screen_update)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 479)

	MCFG_MC6845_ADD(MC6845_TAG, MC6845, XTAL_12_48MHz/8, mc6845_intf)

	// devices
	MCFG_FD1791x_ADD(FD1791_TAG, XTAL_8MHz/8)
	MCFG_Z80CTC_ADD(Z80CTC_TAG, XTAL_8MHz/2, ctc_intf)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("ctc", trs80m2_state, ctc_tick, attotime::from_hz(XTAL_8MHz/2/2))
	MCFG_Z80DMA_ADD(Z80DMA_TAG, XTAL_8MHz/2, dma_intf)
	MCFG_Z80PIO_ADD(Z80PIO_TAG, XTAL_8MHz/2, pio_intf)
	MCFG_Z80SIO0_ADD(Z80SIO_TAG, XTAL_8MHz/2, sio_intf)
	MCFG_CENTRONICS_PRINTER_ADD(CENTRONICS_TAG, centronics_intf)
	MCFG_FLOPPY_DRIVE_ADD(FD1791_TAG":0", trs80m2_floppies, "8ssdd", NULL, floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FD1791_TAG":1", trs80m2_floppies, NULL,    NULL, floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FD1791_TAG":2", trs80m2_floppies, NULL,    NULL, floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FD1791_TAG":3", trs80m2_floppies, NULL,    NULL, floppy_image_device::default_floppy_formats)
	MCFG_PIC8259_ADD(AM9519A_TAG, pic_intf)
	MCFG_TRS80M2_KEYBOARD_ADD(kb_intf)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("keyboard", trs80m2_state, trs80m2_keyboard_tick, attotime::from_hz(60))

	// internal RAM
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("256K")
	MCFG_RAM_EXTRA_OPTIONS("512K,768K,1M")

	// software list
	MCFG_SOFTWARE_LIST_ADD("flop_list", "trs80m2")
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

/*

    TRS-80 Model II/16 Z80 CPU Board ROM

    It would seem that every processor board I find has a different ROM on it!  It seems that the early ROMs
    don't boot directly from a hard drive.  But there seems to be many versions of ROMs.  I've placed them in
    order of serial number in the list below.  There also appears to be at least two board revisions, "C" and "D".

    cpu_c8ff.bin/hex:
    Mask Programmable PROM, Equivilant to Intel 2716 EPROM, with checksum C8FF came from a cpu board with
    serial number 120353 out of a Model II with serial number 2002102 and catalog number 26-6002.  The board
    was labeled, "Revision C".  This appears to be an early ROM and according to a very helpful fellow
    collector, Aaron in Australia, doesn't allow boot directly from a hard disk.

    cpu_9733.bin/hex:
    An actual SGS-Ates (Now STMicroelectronics) 2716 EPROM, with checksum 9733 came from a cpu board with
    serial number 161993 out of a pile of random cards that I have.  I don't know what machine it originated
    from.  The board was labeled, "Revision C".  This appears to be a later ROM in that it is able to boot
    directly from an 8MB hard disk.  The EPROM had a windows sticker on it labeled, "U54".

    cpu_2119.bin/hex:
    An actual Texas Instruments TMS2516 EPROM, with checksum 2119 came from a cpu board with serial number
    178892 out of a Model 16 with serial number 64014509 and catalog number 26-4002.  The board was labeled,
    "Revision D".  This appears to be a later ROM and does appear to allow boot directly from an 8MB hard disk.

    cpu_2bff.bin/hex:
    Mask Programmable PROM, Equivilant to Intel 2716 EPROM, with checksum 2BFF came from a cpu board with
    serial number 187173 our of a pile of random cards that I have.  I don't know what machine it originated
    from.  The board was labeled, "Revision D".  This appears to be a later ROM in that it is able to boot
    directly from an 8MB hard disk.

*/

//-------------------------------------------------
//  ROM( trs80m2 )
//-------------------------------------------------

ROM_START( trs80m2 )
	ROM_REGION( 0x800, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS( "9733" )
	ROM_SYSTEM_BIOS( 0, "c8ff", "Version 1" )
	ROMX_LOAD( "8043216.u11", 0x0000, 0x0800, CRC(7017a373) SHA1(1c7127fcc99fc351a40d3a3199ba478e783c452e), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "2bff", "Version 2 (1981-07-29)" )
	ROMX_LOAD( "8047316.u11", 0x0000, 0x0800, CRC(c6c71d8b) SHA1(7107e2cbbe769851a4460680c2deff8e76a101b5), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "2119", "Version 3 (1982-05-07)" )
	ROMX_LOAD( "cpu_2119.u11", 0x0000, 0x0800, CRC(7a663049) SHA1(f308439ce266df717bfe79adcdad6024b4faa141), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 3, "1bbe", "Version 3 (1982-05-07, Alt)" )
	ROMX_LOAD( "cpu_11be.u11", 0x0000, 0x0800, CRC(8edceea7) SHA1(3d797acedd8a71a82c695129ca764f85aa9022b2), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 4, "fc86", "Version 4 (1982-11-18)" )
	ROMX_LOAD( "cpu_fc86.u11", 0x0000, 0x0800, NO_DUMP, ROM_BIOS(5) )
	ROM_SYSTEM_BIOS( 5, "9733", "Version 5 (1983-07-29)" )
	ROMX_LOAD( "u54.u11", 0x0000, 0x0800, CRC(823924b1) SHA1(aee0625bcbd8620b28ab705e15ad9bea804c8476), ROM_BIOS(6) )

	ROM_REGION( 0x800, MC6845_TAG, 0 )
	ROM_LOAD( "8043316.u9", 0x0000, 0x0800, CRC(04425b03) SHA1(32a29dc202b7fcf21838289cc3bffc51ef943dab) )
ROM_END


//-------------------------------------------------
//  ROM( trs80m16 )
//-------------------------------------------------

#define rom_trs80m16 rom_trs80m2



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT   INIT     COMPANY             FULLNAME        FLAGS
COMP( 1979, trs80m2,	0,			0,		trs80m2,	trs80m2, driver_device,		0,		"Tandy Radio Shack",	"TRS-80 Model II",	GAME_NO_SOUND_HW | GAME_IMPERFECT_KEYBOARD )
COMP( 1982, trs80m16,	trs80m2,	0,		trs80m16,	trs80m2, driver_device,		0,		"Tandy Radio Shack",	"TRS-80 Model 16",	GAME_NO_SOUND_HW | GAME_NOT_WORKING | GAME_IMPERFECT_KEYBOARD )
//COMP( 1983, trs80m12, trs80m2,    0,      trs80m16,   trs80m2, driver_device,     0,      "Tandy Radio Shack",    "TRS-80 Model 12",  GAME_NO_SOUND_HW | GAME_NOT_WORKING | GAME_IMPERFECT_KEYBOARD )
//COMP( 1984, trs80m16b,trs80m2,    0,      trs80m16,   trs80m2, driver_device,     0,      "Tandy Radio Shack",    "TRS-80 Model 16B", GAME_NO_SOUND_HW | GAME_NOT_WORKING | GAME_IMPERFECT_KEYBOARD )
//COMP( 1985, tandy6k,  trs80m2,    0,      tandy6k,    trs80m2, driver_device,     0,      "Tandy Radio Shack",    "Tandy 6000 HD",    GAME_NO_SOUND_HW | GAME_NOT_WORKING | GAME_IMPERFECT_KEYBOARD )
