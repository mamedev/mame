// license:BSD-3-Clause
// copyright-holders:Curt Coder

/*

    NewBrain
    Grundy Business Systems Ltd.

    32K RAM
    28K ROM

    Z80 @ 2MHz
    COP420 @ 2MHz

    Z80 @ 4MHz (416): INT/NMI=+5V, WAIT=EXTBUSRQ|BUSAKD, RESET=_FDC RESET,
    NEC 765AC @ 4 MHz (418)
    MC6850 ACIA (459)
    Z80CTC (458)
    ADC0809 (427)
    DAC0808 (461)

    Models according to the Software Technical Manual:

    Model M: 'Page Register', Expansion Port onto Z80 bus, Video, ACIA/CTC, User Port
    Model A: Expansion Port, Video, no User Port but has software driver serial port - s/w Printer, s/w V24
    Model V: ACIA/CTC, User Port

*/

/*

    TODO:

    - COPINT @ 84Hz instead of 50Hz? (with clock 3240506 is 50Hz)
    - CLKINT arrives too late and COP reads the VFD data before CPU writes it
    - VFD does not receive data from main CPU
    - bitmapped video
    - accurate video timing
    - cassette
    - EIM
    - floppy
    - CP/M 2.2 ROMs
    - localized ROM sets
    - Micropage ROM/RAM card
    - Z80 PIO board
    - peripheral (PI) box
    - sound card

*/

#include "emu.h"
#include "includes/newbrain.h"

#include "screen.h"

#include "newbrain.lh"
#include "newbraina.lh"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0
#define LOG_COP 0
#define LOG_VFD 0



//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  check_interrupt -
//-------------------------------------------------

void newbrain_state::check_interrupt()
{
	int level = ((!m_clk && !m_clkint) || !m_copint) ? ASSERT_LINE : CLEAR_LINE;

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, level);
}


//-------------------------------------------------
//  mreq_r - memory request read
//-------------------------------------------------

READ8_MEMBER( newbrain_state::mreq_r )
{
	bool romov = 1, raminh = 0;
	int exrm = 0;
	uint8_t data = m_exp->mreq_r(offset, 0xff, romov, exrm, raminh);

	int rom0 = 1, rom1 = 1, rom2 = 1;
	int a15_14_13 = romov ? (offset >> 13) : exrm;
	if (!m_pwrup) a15_14_13 = 7;
	int a15g = BIT(a15_14_13, 2);

	switch (a15_14_13)
	{
	case 5: rom0 = 0; break;
	case 6: rom1 = 0; break;
	case 7: rom2 = 0; break;
	}

	if (!a15g && !raminh)
	{
		data = m_ram->pointer()[offset];
	}

	if (!rom0)
	{
		data = m_rom->base()[0x0000 + (offset & 0x1fff)];
	}

	if (!rom1)
	{
		data = m_rom->base()[0x2000 + (offset & 0x1fff)];
	}

	if (!rom2)
	{
		data = m_rom->base()[0x4000 + (offset & 0x1fff)];
	}

	return data;
}


//-------------------------------------------------
//  mreq_w - memory request write
//-------------------------------------------------

WRITE8_MEMBER( newbrain_state::mreq_w )
{
	bool romov = 1, raminh = 0;
	int exrm = 0;
	m_exp->mreq_w(offset, data, romov, exrm, raminh);

	int a15_14_13 = romov ? (offset >> 13) : exrm;
	if (!m_pwrup) a15_14_13 = 7;
	int a15g = BIT(a15_14_13, 2);

	if (!a15g && !raminh)
	{
		m_ram->pointer()[offset] = data;
	}
}


//-------------------------------------------------
//  iorq_r - I/O request read
//-------------------------------------------------

READ8_MEMBER( newbrain_state::iorq_r )
{
	bool prtov = 0;
	uint8_t data = m_exp->iorq_r(offset, 0xff, prtov);

	if (!prtov)
	{
		switch ((offset >> 2) & 0x07)
		{
		case 1: // EXP1
			switch (offset & 0x03)
			{
			case 0: // CLCLK
				clclk();
				break;

			case 2: // COP
				data = m_cop->microbus_rd(space, 0);
				break;
			}
			break;

		case 5: // UST
			if (BIT(offset, 1))
			{
				data = ust_b_r(space, offset, data);
			}
			else
			{
				data = ust_a_r(space, offset, data);
			}
			break;
		}
	}

	return data;
}


//-------------------------------------------------
//  iorq_w - I/O request write
//-------------------------------------------------

WRITE8_MEMBER( newbrain_state::iorq_w )
{
	bool prtov = 0;
	m_exp->iorq_w(offset, 0xff, prtov);

	if (!prtov)
	{
		switch ((offset >> 2) & 0x07)
		{
		case 1: // EXP1
			switch (offset & 0x03)
			{
			case 0: // CLCLK
				clclk();
				break;

			case 2: // COP
				m_cop->microbus_wr(space, offset, data);
				break;

			case 3: // ENRG1
				enrg_w(space, offset, data);
				break;
			}
			break;

		case 2: // TVL
			tvl(data, BIT(offset, 6));
			break;

		case 3: // TVTL
			tvtl_w(space, offset, data);
			break;
		}
	}
}


//-------------------------------------------------
//  clclk - clear clock interrupt
//-------------------------------------------------

void newbrain_state::clclk()
{
	if (LOG) logerror("%s %s CLCLK\n", machine().time().as_string(), machine().describe_context());

	m_clkint = 1;
	check_interrupt();
}


//-------------------------------------------------
//  enrg_w -
//-------------------------------------------------

WRITE8_MEMBER( newbrain_state::enrg_w )
{
	/*

	    bit     signal

	    0       _CLK
	    1
	    2       TVP
	    3
	    4       _RTSD
	    5       DO
	    6
	    7       PO

	*/

	if (LOG) logerror("%s %s ENRG %02x\n", machine().time().as_string(), machine().describe_context(), data);

	// clock enable
	int clk = BIT(data, 0);

	if (m_clk != clk) {
		m_clk = clk;
		check_interrupt();
	}

	// TV enable
	m_tvp = BIT(data, 2);

	// V24
	m_rs232_v24->write_rts(BIT(data, 4));
	m_rs232_v24->write_txd(BIT(data, 5));

	// printer
	m_rs232_prn->write_txd(BIT(data, 7));
}


//-------------------------------------------------
//  ust_r -
//-------------------------------------------------

READ8_MEMBER( newbrain_state::ust_a_r )
{
	/*

	    bit     signal

	    0       +5V
	    1       PWRUP
	    2
	    3
	    4
	    5       _CLKINT
	    6
	    7       _COPINT

	*/

	uint8_t data = 0x5d;

	// powered up
	data |= m_pwrup << 1;

	// interrupts
	data |= m_clkint << 5;
	data |= m_copint << 7;

	return data;
}


//-------------------------------------------------
//  user_r -
//-------------------------------------------------

READ8_MEMBER( newbrain_state::ust_b_r )
{
	/*

	    bit     signal

	    0       RDDK
	    1       _CTSD
	    2
	    3
	    4
	    5       TPIN
	    6
	    7       _CTSP

	*/

	uint8_t data = 0x5c;

	// V24
	data |= m_rs232_v24->rxd_r();
	data |= m_rs232_v24->cts_r() << 1;

	// tape
	data |= tpin() << 5;

	// printer
	data |= m_rs232_prn->cts_r() << 7;

	return data;
}


//-------------------------------------------------
//  cop_in_r -
//-------------------------------------------------

READ8_MEMBER( newbrain_state::cop_in_r )
{
	/*

	    bit     description

	    IN0     K8 (CD4076 Q2)
	    IN1     _RD
	    IN2     _COP
	    IN3     _WR

	*/

	uint8_t data = 0xe;

	// keyboard
	data |= BIT(m_403_q, 2);

	if (LOG_COP) logerror("%s %s IN %01x\n", machine().time().as_string(), machine().describe_context(), data);

	return data;
}


//-------------------------------------------------
//  cop_g_r -
//-------------------------------------------------

READ8_MEMBER( newbrain_state::cop_g_r )
{
	/*

	    bit     description

	    G0
	    G1      K9 (CD4076 Q1)
	    G2      K7 (CD4076 Q0)
	    G3      K3 (CD4076 Q3)

	*/

	uint8_t data = 0;

	// keyboard
	data |= BIT(m_403_q, 1) << 1;
	data |= BIT(m_403_q, 0) << 2;
	data |= BIT(m_403_q, 3) << 3;

	if (LOG_COP) logerror("%s %s G %01x\n", machine().time().as_string(), machine().describe_context(), data);

	return data;
}


//-------------------------------------------------
//  cop_g_w -
//-------------------------------------------------

void newbrain_state::tm()
{
	cassette_state tm1 = (!m_cop_g3 && !m_cop_k6) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED;
	cassette_state tm2 = (!m_cop_g1 && !m_cop_k6) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED;

	m_cassette1->change_state(tm1, CASSETTE_MASK_MOTOR);
	m_cassette2->change_state(tm2, CASSETTE_MASK_MOTOR);
}

WRITE8_MEMBER( newbrain_state::cop_g_w )
{
	/*

	    bit     description

	    G0      _COPINT
	    G1      _TM1
	    G2
	    G3      _TM2

	*/

	int copint = !BIT(data, 0);

	if (LOG_COP) logerror("%s %s COPINT %u\n", machine().time().as_string(), machine().describe_context(), copint);

	if (m_copint != copint)
	{
		m_copint = copint;
		check_interrupt();
	}

	m_cop_g1 = BIT(data, 1);
	m_cop_g3 = BIT(data, 3);
	tm();
}


//-------------------------------------------------
//  cop_d_w -
//-------------------------------------------------

WRITE8_MEMBER( newbrain_state::cop_d_w )
{
	/*
	    bit     description

	    D0      inverted to K4 -> CD4024 pin 2 (reset)
	    D1      TDO
	    D2      inverted to K6 -> CD4024 pin 1 (clock), CD4076 pin 7 (clock), inverted to DS8881 pin 3 (enable)
	    D3      not connected

	*/

	int k4 = !BIT(data, 0);
	int k6 = !BIT(data, 2);

	if (LOG_COP) logerror("%s %s K4 %u K6 %u\n", machine().time().as_string(), machine().describe_context(), k4, k6);

	m_cop_tdo = BIT(data, 1);
	m_cassette1->output(m_cop_tdo ? -1.0 : +1.0);
	m_cassette2->output(m_cop_tdo ? -1.0 : +1.0);

	if (k4) {
		m_405_q = 0;

		if (LOG_COP) logerror("%s %s keylatch reset\n", machine().time().as_string(), machine().describe_context());
	} else if (m_cop_k6 && !k6) {
		m_405_q++;
		m_405_q &= 0x7f;

		if (LOG_COP) logerror("%s %s keylatch %u\n", machine().time().as_string(), machine().describe_context(), m_405_q);
	}

	if (!m_cop_k6 && k6) {
		m_403_d = m_y[m_405_q & 0x0f]->read() & 0x0f;

		if (LOG_COP) logerror("%s %s keydata %01x\n", machine().time().as_string(), machine().describe_context(), m_403_d);
	}

	if (k6) {
		m_403_q = m_403_d;
	} else {
		m_403_q = 0xf;

		if (LOG_COP) logerror("%s %s keydata disabled\n", machine().time().as_string(), machine().describe_context());

		// COP to VFD serial format, bits 15..0
		// A B J I x H G2 C x F G1 E K L M D
		uint16_t value = bitswap<16>(m_402_q, 11, 7, 1, 13, 10, 3, 2, 12, 9, 5, 6, 4, 0, 8, 14, 15) & 0x3fff;
		m_digits[m_405_q & 0x0f] = value;

		if (LOG_VFD) logerror("%s %s vfd segment %u 402.Q %04x data %04x\n", machine().time().as_string(), machine().describe_context(), m_405_q & 0x0f, m_402_q, value);
	}

	m_cop_k6 = k6;
	tm();
}


//-------------------------------------------------
//  k1_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( newbrain_state::k1_w )
{
	if (LOG_VFD) logerror("%s %s SO %u\n", machine().time().as_string(), machine().describe_context(), state);

	m_cop_so = state;
}


//-------------------------------------------------
//  k2_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( newbrain_state::k2_w )
{
	if (LOG_VFD) logerror("%s %s SK %u\n", machine().time().as_string(), machine().describe_context(), state);

	if (state)
	{
		m_402_q <<= 1;
		m_402_q = (m_402_q & 0xfffe) | m_cop_so;
	}
}


//-------------------------------------------------
//  tdi_r -
//-------------------------------------------------

int newbrain_state::tpin()
{
	return (m_cassette1->input() > +1.0) || (m_cassette2->input() > +1.0);
}

READ_LINE_MEMBER( newbrain_state::tdi_r )
{
	return tpin() ^ m_cop_tdo;
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( newbrain_mreq )
//-------------------------------------------------

void newbrain_state::newbrain_mreq(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).rw(FUNC(newbrain_state::mreq_r), FUNC(newbrain_state::mreq_w));
}


//-------------------------------------------------
//  ADDRESS_MAP( newbrain_iorq )
//-------------------------------------------------

void newbrain_state::newbrain_iorq(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).rw(FUNC(newbrain_state::iorq_r), FUNC(newbrain_state::iorq_w));
}



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( newbrain )
//-------------------------------------------------

static INPUT_PORTS_START( newbrain )
	PORT_START("Y0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("STOP") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("Y1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))

	PORT_START("Y2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')

	PORT_START("Y3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')

	PORT_START("Y4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')

	PORT_START("Y5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('(') PORT_CHAR('[')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')

	PORT_START("Y6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR(')') PORT_CHAR(']')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')

	PORT_START("Y7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("* \xC2\xA3") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('*') PORT_CHAR(0x00A3)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')

	PORT_START("Y8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("VIDEO TEXT") PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(RALT)) // Vd
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')

	PORT_START("Y9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')

	PORT_START("Y10")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('@')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')

	PORT_START("Y11")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('-') PORT_CHAR('\\')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')

	PORT_START("Y12")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('+') PORT_CHAR('^')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("INSERT") PORT_CODE(KEYCODE_INSERT) PORT_CHAR(UCHAR_MAMEKEY(INSERT))

	PORT_START("Y13")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("NEW LINE") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13) // NL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')

	PORT_START("Y14")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("HOME") PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME)) // CH

	PORT_START("Y15")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1) // SH
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("GRAPHICS") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT)) // GR
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("REPEAT") // RPT
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CONTROL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL)) // GL
INPUT_PORTS_END



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

int newbrain_state::get_reset_t()
{
	return RES_K(220) * CAP_U(10) * 1000; // t = R128 * C125 = 2.2s
}

int newbrain_state::get_pwrup_t()
{
	return RES_K(560) * CAP_U(10) * 1000; // t = R129 * C127 = 5.6s
}


//-------------------------------------------------
//  machine_start -
//-------------------------------------------------

void newbrain_state::machine_start()
{
	m_digits.resolve();

	// set power up timer
	timer_set(attotime::from_usec(get_pwrup_t()), TIMER_ID_PWRUP);

	// state saving
	save_item(NAME(m_clk));
	save_item(NAME(m_tvp));
	save_item(NAME(m_pwrup));
	save_item(NAME(m_clkint));
	save_item(NAME(m_copint));
	save_item(NAME(m_cop_so));
	save_item(NAME(m_cop_tdo));
	save_item(NAME(m_cop_g1));
	save_item(NAME(m_cop_g3));
	save_item(NAME(m_cop_k6));
	save_item(NAME(m_405_q));
	save_item(NAME(m_403_q));
	save_item(NAME(m_402_q));

	// patch COP ROM to read VFD data
	//memregion(COP420_TAG)->base()[0x20a] = 0xc6;
}


//-------------------------------------------------
//  machine_reset -
//-------------------------------------------------

void newbrain_state::machine_reset()
{
	m_maincpu->reset();
	m_cop->reset();

	m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	m_cop->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);

	address_space &space = m_maincpu->space(AS_PROGRAM);
	enrg_w(space, 0, 0);

	timer_set(attotime::from_usec(get_reset_t()), TIMER_ID_RESET);
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void newbrain_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_ID_RESET:
		if (LOG) logerror("%s %s RESET 1\n", machine().time().as_string(), machine().describe_context());

		m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		m_cop->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		break;

	case TIMER_ID_PWRUP:
		if (LOG) logerror("%s %s PWRUP 1\n", machine().time().as_string(), machine().describe_context());

		m_pwrup = 1;
		break;

	case TIMER_ID_CLKINT:
		if (LOG) logerror("%s CLKINT\n", machine().time().as_string());

		m_clkint = 0;
		check_interrupt();
		break;
	}
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( newbrain )
//-------------------------------------------------

void newbrain_state::newbrain(machine_config &config)
{
	// basic system hardware
	Z80(config, m_maincpu, XTAL(16'000'000)/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &newbrain_state::newbrain_mreq);
	m_maincpu->set_addrmap(AS_IO, &newbrain_state::newbrain_iorq);

	COP420(config, m_cop, XTAL(16'000'000)/4);
	m_cop->set_config(COP400_CKI_DIVISOR_16, COP400_CKO_OSCILLATOR_OUTPUT, true);
	m_cop->read_g().set(FUNC(newbrain_state::cop_g_r));
	m_cop->write_g().set(FUNC(newbrain_state::cop_g_w));
	m_cop->write_d().set(FUNC(newbrain_state::cop_d_w));
	m_cop->read_in().set(FUNC(newbrain_state::cop_in_r));
	m_cop->write_so().set(FUNC(newbrain_state::k1_w));
	m_cop->write_sk().set(FUNC(newbrain_state::k2_w));
	m_cop->read_si().set(FUNC(newbrain_state::tdi_r));

	// video hardware
	newbrain_video(config);

	// devices
	NEWBRAIN_EXPANSION_SLOT(config, m_exp, XTAL(16'000'000)/4, newbrain_expansion_cards, "eim");

	CASSETTE(config, m_cassette1);
	m_cassette1->set_default_state((cassette_state)(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_MUTED));

	CASSETTE(config, m_cassette2);
	m_cassette2->set_default_state((cassette_state)(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_MUTED));

	RS232_PORT(config, RS232_V24_TAG, default_rs232_devices, nullptr);
	RS232_PORT(config, RS232_PRN_TAG, default_rs232_devices, nullptr);

	// internal ram
	RAM(config, RAM_TAG).set_default_size("32K");
}


//-------------------------------------------------
//  MACHINE_CONFIG( newbrain_ad )
//-------------------------------------------------

void newbrain_state::newbrain_ad(machine_config &config)
{
	newbrain(config);
	config.set_default_layout(layout_newbrain);
}


//-------------------------------------------------
//  MACHINE_CONFIG( newbrain_a )
//-------------------------------------------------

void newbrain_state::newbrain_a(machine_config &config)
{
	newbrain(config);
	config.set_default_layout(layout_newbraina);
}


//-------------------------------------------------
//  MACHINE_CONFIG( newbrain_md )
//-------------------------------------------------

void newbrain_state::newbrain_md(machine_config &config)
{
	newbrain(config);
	config.set_default_layout(layout_newbrain);
}



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( newbrain )
//-------------------------------------------------

ROM_START( newbrain )
	ROM_REGION( 0x6000, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS( "rom20" )

	ROM_SYSTEM_BIOS( 0, "issue1", "Issue 1 (v?)" )
	ROMX_LOAD( "aben.ic6",     0x0000, 0x2000, CRC(308f1f72) SHA1(a6fd9945a3dca47636887da2125fde3f9b1d4e25), ROM_BIOS(0) )
	ROMX_LOAD( "cd iss 1.ic7", 0x2000, 0x2000, CRC(6b4d9429) SHA1(ef688be4e75aced61f487c928258c8932a0ae00a), ROM_BIOS(0) )
	ROMX_LOAD( "ef iss 1.ic8", 0x4000, 0x2000, CRC(20dd0b49) SHA1(74b517ca223cefb588e9f49e72ff2d4f1627efc6), ROM_BIOS(0) )

	ROM_SYSTEM_BIOS( 1, "issue2", "Issue 2 (v1.9)" )
	ROMX_LOAD( "aben19.ic6",   0x0000, 0x2000, CRC(d0283eb1) SHA1(351d248e69a77fa552c2584049006911fb381ff0), ROM_BIOS(1) )
	ROMX_LOAD( "cdi2.ic7",     0x2000, 0x2000, CRC(6b4d9429) SHA1(ef688be4e75aced61f487c928258c8932a0ae00a), ROM_BIOS(1) )
	ROMX_LOAD( "ef iss 1.ic8", 0x4000, 0x2000, CRC(20dd0b49) SHA1(74b517ca223cefb588e9f49e72ff2d4f1627efc6), ROM_BIOS(1) )

	ROM_SYSTEM_BIOS( 2, "issue3", "Issue 3 (v1.91)" )
	ROMX_LOAD( "aben191.ic6",  0x0000, 0x2000, CRC(b7be8d89) SHA1(cce8d0ae7aa40245907ea38b7956c62d039d45b7), ROM_BIOS(2) )
	ROMX_LOAD( "cdi3.ic7",     0x2000, 0x2000, CRC(6b4d9429) SHA1(ef688be4e75aced61f487c928258c8932a0ae00a), ROM_BIOS(2) )
	ROMX_LOAD( "ef iss 1.ic8", 0x4000, 0x2000, CRC(20dd0b49) SHA1(74b517ca223cefb588e9f49e72ff2d4f1627efc6), ROM_BIOS(2) )

	ROM_SYSTEM_BIOS( 3, "series2", "Series 2 (v?)" )
	ROMX_LOAD( "abs2.ic6",     0x0000, 0x2000, CRC(9a042acb) SHA1(80d83a2ea3089504aa68b6cf978d80d296cd9bda), ROM_BIOS(3) )
	ROMX_LOAD( "cds2.ic7",     0x2000, 0x2000, CRC(6b4d9429) SHA1(ef688be4e75aced61f487c928258c8932a0ae00a), ROM_BIOS(3) )
	ROMX_LOAD( "efs2.ic8",     0x4000, 0x2000, CRC(b222d798) SHA1(c0c816b4d4135b762f2c5f1b24209d0096f22e56), ROM_BIOS(3) )

	ROM_SYSTEM_BIOS( 4, "rom20", "? (v2.0)" )
	ROMX_LOAD( "aben20.rom",   0x0000, 0x2000, CRC(3d76d0c8) SHA1(753b4530a518ad832e4b81c4e5430355ba3f62e0), ROM_BIOS(4) )
	ROMX_LOAD( "cd20tci.rom",  0x2000, 0x4000, CRC(f65b2350) SHA1(1ada7fbf207809537ec1ffb69808524300622ada), ROM_BIOS(4) )

	ROM_REGION( 0x400, COP420_TAG, 0 )
	ROM_LOAD( "cop420-guw.ic419", 0x000, 0x400, CRC(a1388ee7) SHA1(5822e16aa794545600bf7a9dbee2ef467ca2a3e0) ) // COP420-GUW/N

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "char eprom iss 1.ic453", 0x0000, 0x1000, CRC(6a38b7a2) SHA1(29f3e672fc41792ac2f2b405e571d79235193561) ) // 8248R7
ROM_END


//-------------------------------------------------
//  ROM( newbraina )
//-------------------------------------------------

#define rom_newbraina rom_newbrain


//-------------------------------------------------
//  ROM( newbrainmd )
//-------------------------------------------------

ROM_START( newbrainmd )
	ROM_REGION( 0x10000, Z80_TAG, 0 )
	ROM_LOAD( "cdmd.rom", 0x2000, 0x2000, CRC(6b4d9429) SHA1(ef688be4e75aced61f487c928258c8932a0ae00a) )
	ROM_LOAD( "efmd.rom", 0x4000, 0x2000, CRC(20dd0b49) SHA1(74b517ca223cefb588e9f49e72ff2d4f1627efc6) )

	ROM_REGION( 0x400, COP420_TAG, 0 )
	ROM_LOAD( "cop420-guw.ic419", 0x000, 0x400, CRC(a1388ee7) SHA1(5822e16aa794545600bf7a9dbee2ef467ca2a3e0) ) // COP420-GUW/N

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "char eprom iss 1.ic453", 0x0000, 0x1000, CRC(6a38b7a2) SHA1(29f3e672fc41792ac2f2b405e571d79235193561) ) // 8248R7
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME        PARENT    COMPAT  MACHINE      INPUT     CLASS           INIT        COMPANY                        FULLNAME       FLAGS
COMP( 1981, newbrain,   0,        0,      newbrain_ad, newbrain, newbrain_state, empty_init, "Grundy Business Systems Ltd", "NewBrain AD", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND )
COMP( 1981, newbraina,  newbrain, 0,      newbrain_a,  newbrain, newbrain_state, empty_init, "Grundy Business Systems Ltd", "NewBrain A",  MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND )
COMP( 1981, newbrainmd, newbrain, 0,      newbrain_md, newbrain, newbrain_state, empty_init, "Grundy Business Systems Ltd", "NewBrain MD", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND )
