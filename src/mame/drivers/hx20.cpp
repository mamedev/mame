// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    Epson HX-20

    http://fjkraan.home.xs4all.nl/comp/hx20/

****************************************************************************/

/*

    TODO:

    - m6800.c rewrite
    - keyboard interrupt
    - LCD controller
    - serial
    - SW6 read
    - RS-232
    - microcassette
    - printer
    - ROM cartridge
    - floppy TF-20
    - barcode reader

*/

#include "includes/hx20.h"
#include "softlist.h"


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

//-------------------------------------------------
//  update_interrupt -
//-------------------------------------------------

void hx20_state::update_interrupt()
{
	int irq = m_rtc_irq || m_kbrequest;

	m_maincpu->set_input_line(M6800_IRQ_LINE, irq);
}


//-------------------------------------------------
//  ksc_w -
//-------------------------------------------------

WRITE8_MEMBER( hx20_state::ksc_w )
{
	logerror("KSC %02x\n", data);

	m_ksc = data;
}


//-------------------------------------------------
//  krtn07_r -
//-------------------------------------------------

READ8_MEMBER( hx20_state::krtn07_r )
{
	UINT8 data = 0xff;

	if (!BIT(m_ksc, 0)) data &= m_ksc0->read();
	if (!BIT(m_ksc, 1)) data &= m_ksc1->read();
	if (!BIT(m_ksc, 2)) data &= m_ksc2->read();
	if (!BIT(m_ksc, 3)) data &= m_ksc3->read();
	if (!BIT(m_ksc, 4)) data &= m_ksc4->read();
	if (!BIT(m_ksc, 5)) data &= m_ksc5->read();
	if (!BIT(m_ksc, 6)) data &= m_ksc6->read();
	if (!BIT(m_ksc, 7)) data &= m_ksc7->read();

	return data;
}


//-------------------------------------------------
//  krtn89_r -
//-------------------------------------------------

READ8_MEMBER( hx20_state::krtn89_r )
{
	/*

	    bit     description

	    0       KRTN8
	    1       KRTN9
	    2
	    3
	    4
	    5
	    6       _PWSW
	    7       _BUSY

	*/

	UINT8 data = 0xff;

	if (!BIT(m_ksc, 0)) data &= m_ksc0->read() >> 8;
	if (!BIT(m_ksc, 1)) data &= m_ksc1->read() >> 8;
	if (!BIT(m_ksc, 2)) data &= m_ksc2->read() >> 8;
	if (!BIT(m_ksc, 3)) data &= m_ksc3->read() >> 8;
	if (!BIT(m_ksc, 4)) data &= m_ksc4->read() >> 8;
	if (!BIT(m_ksc, 5)) data &= m_ksc5->read() >> 8;
	if (!BIT(m_ksc, 6)) data &= m_ksc6->read() >> 8;
	if (!BIT(m_ksc, 7)) data &= m_ksc7->read() >> 8;

	return data;
}


//-------------------------------------------------
//  lcd_cs_w -
//-------------------------------------------------

WRITE8_MEMBER( hx20_state::lcd_cs_w )
{
	/*

	    bit     description

	    0       LCD CS bit 0
	    1       LCD CS bit 1
	    2       LCD CS bit 2
	    3       LCD C/_D
	    4       KEY INT MASK
	    5       SERIAL POUT
	    6       MO1
	    7       MO2

	*/

	logerror("LCD CS %02x\n", data);

	// LCD
	m_lcdc0->cs_w(1);
	m_lcdc1->cs_w(1);
	m_lcdc2->cs_w(1);
	m_lcdc3->cs_w(1);
	m_lcdc4->cs_w(1);
	m_lcdc5->cs_w(1);

	switch (data & 0x07)
	{
	case 1: m_lcdc0->cs_w(0); break;
	case 2: m_lcdc1->cs_w(0); break;
	case 3: m_lcdc2->cs_w(0); break;
	case 4: m_lcdc3->cs_w(0); break;
	case 5: m_lcdc4->cs_w(0); break;
	case 6: m_lcdc5->cs_w(0); break;
	}

	int cd = BIT(data, 3);

	m_lcdc0->cd_w(cd);
	m_lcdc1->cd_w(cd);
	m_lcdc2->cd_w(cd);
	m_lcdc3->cd_w(cd);
	m_lcdc4->cd_w(cd);
	m_lcdc5->cd_w(cd);

	// serial
	m_sio->pout_w(BIT(data, 5));
}


//-------------------------------------------------
//  lcd_data_w -
//-------------------------------------------------

WRITE8_MEMBER( hx20_state::lcd_data_w )
{
	logerror("LCD DATA %02x\n", data);

	m_lcd_data = data;
}


//-------------------------------------------------
//  main_p1_r - main CPU port 1 read
//-------------------------------------------------

READ8_MEMBER( hx20_state::main_p1_r )
{
	/*

	    bit     description

	    0       RS-232 DSR
	    1       RS-232 CTS
	    2
	    3       _INT EX
	    4       _PWA
	    5       _K.B REQUEST
	    6       SERIAL PIN
	    7       CARTRIDGE MI1 (0=ROM, 1=microcassette)

	*/

	UINT8 data = 0x98;

	// RS-232
	data |= m_rs232->dsr_r();
	data |= m_rs232->cts_r() << 1;

	// keyboard
	data |= m_kbrequest << 5;

	// serial
	data |= m_sio_pin << 6;

	return data;
}


//-------------------------------------------------
//  main_p1_w - main CPU port 1 write
//-------------------------------------------------

WRITE8_MEMBER( hx20_state::main_p1_w )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5
	    6
	    7

	*/
}


//-------------------------------------------------
//  main_p2_r - main CPU port 2 read
//-------------------------------------------------

READ8_MEMBER( hx20_state::main_p2_r )
{
	/*

	    bit     description

	    0       bar code reader data
	    1
	    2       SLAVE P34
	    3       RX
	    4
	    5
	    6
	    7

	*/

	UINT8 data = M6801_MODE_4;

	// serial
	data &= ~(!m_slave_flag << 2);

	if (m_slave_sio)
		data |= m_slave_rx << 3;
	else
		data |= m_sio_rx << 3;

	return data;
}


//-------------------------------------------------
//  main_p2_w - main CPU port 2 write
//-------------------------------------------------

WRITE8_MEMBER( hx20_state::main_p2_w )
{
	/*

	    bit     description

	    0
	    1       RS-232 TXD
	    2       serial select (0=peripheral, 1=slave 6301)
	    3
	    4       TX
	    5
	    6
	    7

	*/

	// RS-232
	m_rs232->write_txd(BIT(data, 1));

	// serial
	m_slave_sio = BIT(data, 2);

	if (m_slave_sio)
		m_slave_tx = BIT(data, 4);
	else
		m_sio->tx_w(BIT(data, 4));
}


//-------------------------------------------------
//  slave_p1_r - slave CPU port 1 read
//-------------------------------------------------

READ8_MEMBER( hx20_state::slave_p1_r )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5
	    6       printer reset pulse
	    7       printer timing pulse

	*/

	UINT8 data = 0;

	return data;
}


//-------------------------------------------------
//  slave_p1_w - slave CPU port 1 write
//-------------------------------------------------

WRITE8_MEMBER( hx20_state::slave_p1_w )
{
	/*

	    bit     description

	    0       printer head 1 (1=on)
	    1       printer head 2
	    2       printer head 3
	    3       printer head 4
	    4       printer motor (0=on)
	    5       speaker (1=on)
	    6
	    7

	*/

	// speaker
	m_speaker->level_w(BIT(data, 5));
}


//-------------------------------------------------
//  slave_p2_r - slave CPU port 2 read
//-------------------------------------------------

READ8_MEMBER( hx20_state::slave_p2_r )
{
	/*

	    bit     description

	    0       RS-232 RXD / microcassette (0=read, 1=write)
	    1
	    2
	    3       TX
	    4
	    5
	    6
	    7

	*/

	UINT8 data = M6801_MODE_7;

	// serial
	data |= m_slave_tx << 3;

	return data;
}


//-------------------------------------------------
//  slave_p2_w - slave CPU port 2 write
//-------------------------------------------------

WRITE8_MEMBER( hx20_state::slave_p2_w )
{
	/*

	    bit     description

	    0
	    1       microcassette internal clock, write data
	    2       serial select (0=brake, 1=normal)
	    3
	    4       RX
	    5
	    6
	    7

	*/

	// serial
	m_slave_rx = BIT(data, 4);
}


//-------------------------------------------------
//  slave_p3_r - slave CPU port 3 read
//-------------------------------------------------

READ8_MEMBER( hx20_state::slave_p3_r )
{
	/*

	    bit     description

	    0
	    1
	    2       external cassette read data
	    3
	    4
	    5
	    6
	    7

	*/

	UINT8 data = 0;

	return data;
}


//-------------------------------------------------
//  slave_p3_w - slave CPU port 3 write
//-------------------------------------------------

WRITE8_MEMBER( hx20_state::slave_p3_w )
{
	/*

	    bit     description

	    0       external cassette remote (0=on)
	    1       RS-232 RTS
	    2
	    3       external cassette write data
	    4       slave status flag
	    5       bar code power (1=on)
	    6       RS-232 power (1=on)
	    7       program power (1=on)

	*/

	// RS-232
	m_rs232->write_rts(BIT(data, 1));

	// main
	m_slave_flag = BIT(data, 4);
}


//-------------------------------------------------
//  slave_p4_r - slave CPU port 4 read
//-------------------------------------------------

READ8_MEMBER( hx20_state::slave_p4_r )
{
	/*

	    bit     description

	    0       PLUG 2
	    1
	    2
	    3
	    4
	    5
	    6
	    7       RS-232 CD

	*/

	UINT8 data = 0;

	// RS-232
	data |= m_rs232->dcd_r() << 7;

	return data;
}


//-------------------------------------------------
//  slave_p4_w - slave CPU port 4 write
//-------------------------------------------------

WRITE8_MEMBER( hx20_state::slave_p4_w )
{
	/*

	    bit     description

	    0
	    1       port enable always on / printer motor control (0=open, 1=brake)
	    2       clear shift register / microcassette power switch (1=on)
	    3       ROM cartridge power switch (1=on) / microcassette command
	    4       ROM address counter clear / clock
	    5       cassette/RS-232 select (0=RS-232, 1=microcassette)
	    6       ROM cartridge select / microcassette clock (0=counter, 1=head switch)
	    7

	*/
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( hx20_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( hx20_mem, AS_PROGRAM, 8, hx20_state )
	AM_RANGE(0x0000, 0x001f) AM_DEVREADWRITE(HD6301V1_MAIN_TAG, hd63701_cpu_device, m6801_io_r, m6801_io_w)
	AM_RANGE(0x0020, 0x0020) AM_WRITE(ksc_w)
	AM_RANGE(0x0022, 0x0022) AM_READ(krtn07_r)
	AM_RANGE(0x0026, 0x0026) AM_WRITE(lcd_cs_w)
	AM_RANGE(0x0028, 0x0028) AM_READ(krtn89_r)
	AM_RANGE(0x002a, 0x002a) AM_WRITE(lcd_data_w)
	AM_RANGE(0x002c, 0x002c) // mask interruption by using IC 8E in sleep mode
	AM_RANGE(0x0040, 0x007f) AM_DEVREADWRITE(MC146818_TAG, mc146818_device, read, write)
	AM_RANGE(0x0080, 0x00ff) AM_RAM
	AM_RANGE(0x0100, 0x3fff) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_ROM AM_REGION(HD6301V1_MAIN_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( hx20_io )
//-------------------------------------------------

static ADDRESS_MAP_START( hx20_io, AS_IO, 8, hx20_state )
	AM_RANGE(M6801_PORT1, M6801_PORT1) AM_READWRITE(main_p1_r, main_p1_w)
	AM_RANGE(M6801_PORT2, M6801_PORT2) AM_READWRITE(main_p2_r, main_p2_w)
	AM_RANGE(M6801_PORT3, M6801_PORT3) AM_NOP // A0-A7, D0-D7
	AM_RANGE(M6801_PORT4, M6801_PORT4) AM_NOP // A8-A15
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( hx20_io )
//-------------------------------------------------

static ADDRESS_MAP_START( hx20_sub_mem, AS_PROGRAM, 8, hx20_state )
	AM_RANGE(0x0000, 0x001f) AM_DEVREADWRITE(HD6301V1_SLAVE_TAG, hd63701_cpu_device, m6801_io_r, m6801_io_w)
	AM_RANGE(0x0080, 0x00ff) AM_RAM
	AM_RANGE(0xf000, 0xffff) AM_ROM AM_REGION(HD6301V1_SLAVE_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( hx20_io )
//-------------------------------------------------

static ADDRESS_MAP_START( hx20_sub_io, AS_IO, 8, hx20_state )
	AM_RANGE(M6801_PORT1, M6801_PORT1) AM_READWRITE(slave_p1_r, slave_p1_w)
	AM_RANGE(M6801_PORT2, M6801_PORT2) AM_READWRITE(slave_p2_r, slave_p2_w)
	AM_RANGE(M6801_PORT3, M6801_PORT3) AM_READWRITE(slave_p3_r, slave_p3_w)
	AM_RANGE(M6801_PORT4, M6801_PORT4) AM_READWRITE(slave_p4_r, slave_p4_w)
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( hx20 )
//-------------------------------------------------

static INPUT_PORTS_START( hx20 )
	PORT_START("KSC0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('_')
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PF1") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SPECIAL ) // SW6:1
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KSC1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PF2") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SPECIAL ) // SW6:2
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KSC2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_CHAR('@') PORT_CHAR('^')
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PF3") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SPECIAL ) // SW6:3
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KSC3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PF4") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SPECIAL ) // SW6:4
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KSC4")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PF5") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KSC5")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD )  PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT" " UTF8_UP) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT" " UTF8_DOWN) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PAPER FEED")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KSC6")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('\r')
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("TAB") PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("NUM")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("GRPH")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CAPS LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KSC7")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("HOME CLR") PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SCRN")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("BREAK")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PAUSE")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("INS DEL") PORT_CODE(KEYCODE_DEL) PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("MENU")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PRTR ON/OFF")
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SW6")
	PORT_DIPNAME( 0x07, 0x07, "Country" ) PORT_DIPLOCATION("SW6:1,2,3")
	PORT_DIPSETTING(    0x00, "Spain" )
	PORT_DIPSETTING(    0x01, "Italy" )
	PORT_DIPSETTING(    0x02, "Sweden" )
	PORT_DIPSETTING(    0x03, "Denmark" )
	PORT_DIPSETTING(    0x04, "England" )
	PORT_DIPSETTING(    0x05, "Germany" )
	PORT_DIPSETTING(    0x06, "France" )
	PORT_DIPSETTING(    0x07, "America" )
	PORT_DIPNAME( 0x08, 0x00, "Floppy Drive TF-20" ) PORT_DIPLOCATION("SW6:4")
	PORT_DIPSETTING(    0x00, "Installed" )
	PORT_DIPSETTING(    0x08, "Not Installed" )
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( hx20e )
//-------------------------------------------------

static INPUT_PORTS_START( hx20e )
	PORT_INCLUDE(hx20)

	PORT_MODIFY("SW6")
	PORT_DIPNAME( 0x07, 0x00, "Country" ) PORT_DIPLOCATION("SW6:1,2,3")
	PORT_DIPSETTING(    0x00, "Sweden (ASCII)" )
	PORT_DIPSETTING(    0x01, "Germany (ASCII)" )
	PORT_DIPSETTING(    0x02, "France (ASCII)" )
	PORT_DIPSETTING(    0x03, "Denmark" )
	PORT_DIPSETTING(    0x04, "Sweden" )
	PORT_DIPSETTING(    0x05, "Germany" )
	PORT_DIPSETTING(    0x06, "France" )
	PORT_DIPSETTING(    0x07, "Norway" )
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  mc146818_interface rtc_intf
//-------------------------------------------------

WRITE_LINE_MEMBER( hx20_state::rtc_irq_w )
{
	m_rtc_irq = state;

	update_interrupt();
}



//**************************************************************************
//  VIDEO
//**************************************************************************

PALETTE_INIT_MEMBER(hx20_state, hx20)
{
	palette.set_pen_color(0, 0xa5, 0xad, 0xa5);
	palette.set_pen_color(1, 0x31, 0x39, 0x10);
}


//-------------------------------------------------
//  SCREEN_UPDATE( hx20 )
//-------------------------------------------------

UINT32 hx20_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_lcdc0->screen_update(screen, bitmap, cliprect);
	m_lcdc1->screen_update(screen, bitmap, cliprect);
	m_lcdc2->screen_update(screen, bitmap, cliprect);
	m_lcdc3->screen_update(screen, bitmap, cliprect);
	m_lcdc4->screen_update(screen, bitmap, cliprect);
	m_lcdc5->screen_update(screen, bitmap, cliprect);

	return 0;
}



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( hx20 )
//-------------------------------------------------

void hx20_state::machine_start()
{
	// state saving
	save_item(NAME(m_slave_rx));
	save_item(NAME(m_slave_tx));
	save_item(NAME(m_slave_flag));
	save_item(NAME(m_ksc));
	save_item(NAME(m_kbrequest));
	save_item(NAME(m_lcd_data));
}



//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( hx20 )
//-------------------------------------------------

static MACHINE_CONFIG_START( hx20, hx20_state )
	// basic machine hardware
	MCFG_CPU_ADD(HD6301V1_MAIN_TAG, HD63701, XTAL_2_4576MHz)
	MCFG_CPU_PROGRAM_MAP(hx20_mem)
	MCFG_CPU_IO_MAP(hx20_io)

	MCFG_CPU_ADD(HD6301V1_SLAVE_TAG, HD63701, XTAL_2_4576MHz)
	MCFG_CPU_PROGRAM_MAP(hx20_sub_mem)
	MCFG_CPU_IO_MAP(hx20_sub_io)

	// video hardware
	MCFG_DEFAULT_LAYOUT(layout_lcd)
	MCFG_SCREEN_ADD(SCREEN_TAG, LCD)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(120, 32)
	MCFG_SCREEN_VISIBLE_AREA(0, 120-1, 0, 32-1)
	MCFG_SCREEN_UPDATE_DRIVER(hx20_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(hx20_state, hx20)

	MCFG_UPD7227_ADD(UPD7227_0_TAG, 0, 0)
	MCFG_UPD7227_ADD(UPD7227_1_TAG, 40, 0)
	MCFG_UPD7227_ADD(UPD7227_2_TAG, 80, 0)
	MCFG_UPD7227_ADD(UPD7227_3_TAG, 0, 16)
	MCFG_UPD7227_ADD(UPD7227_4_TAG, 40, 16)
	MCFG_UPD7227_ADD(UPD7227_5_TAG, 80, 16)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	// devices
	MCFG_MC146818_ADD(MC146818_TAG, XTAL_4_194304Mhz)
	MCFG_MC146818_IRQ_HANDLER(WRITELINE(hx20_state, rtc_irq_w))
	MCFG_RS232_PORT_ADD(RS232_TAG, default_rs232_devices, NULL)
	MCFG_CASSETTE_ADD(CASSETTE_TAG)
	MCFG_EPSON_SIO_ADD("sio", "tf20")
	MCFG_EPSON_SIO_RX(WRITELINE(hx20_state, sio_rx_w))
	MCFG_EPSON_SIO_PIN(WRITELINE(hx20_state, sio_pin_w))

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("16K")
	MCFG_RAM_EXTRA_OPTIONS("32K")

	// software lists
	MCFG_SOFTWARE_LIST_ADD("epson_cpm_list", "epson_cpm")
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( hx20 )
//-------------------------------------------------

ROM_START( ehx20 )
	ROM_REGION( 0x8000, HD6301V1_MAIN_TAG, ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS( "v11" )
	ROM_SYSTEM_BIOS( 0, "v10", "version 1.0" )
	ROMX_LOAD( "hx20_v10.12e", 0x0000, 0x2000, CRC(ed7482c6) SHA1(8fba63037f2418aee9e933a353b052a5ed816ead), ROM_BIOS(1) )
	ROMX_LOAD( "hx20_v10.13e", 0x2000, 0x2000, CRC(f5cc8868) SHA1(3248a1ddf0d8df7e9f2fe96955385218d760c4ad), ROM_BIOS(1) )
	ROMX_LOAD( "hx20_v10.14e", 0x4000, 0x2000, CRC(27d743ed) SHA1(ebae367b0fa5f42ac78424df2534312296fd6fdc), ROM_BIOS(1) )
	ROMX_LOAD( "hx20_v10.15e", 0x6000, 0x2000, CRC(33fbb1ab) SHA1(292ace94b4dad267aa7786dc64e68ac6f3c98aa7), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "v11", "version 1.1" )
	ROMX_LOAD( "hx20_v11.12e", 0x0000, 0x2000, CRC(4de0b4b6) SHA1(f15c537824b7effde9d9b9a21e92a081fb089371), ROM_BIOS(2) )
	ROMX_LOAD( "hx20_v11.13e", 0x2000, 0x2000, CRC(10d6ae76) SHA1(3163954ed9981f70f590ee98bcc8e19e4be6527a), ROM_BIOS(2) )
	ROMX_LOAD( "hx20_v11.14e", 0x4000, 0x2000, CRC(26c203a1) SHA1(b282d7233b2689820fcf718dbe1e93d623b67e4f), ROM_BIOS(2) )
	ROMX_LOAD( "hx20_v11.15e", 0x6000, 0x2000, CRC(101cb3e8) SHA1(e0b5cf107a9387e34a0e46f54328b89696c0bdc5), ROM_BIOS(2) )

	ROM_REGION( 0x1000, HD6301V1_SLAVE_TAG, 0 )
	ROM_LOAD( "hd6301v1.6d", 0x0000, 0x1000, CRC(b36f5b99) SHA1(c6b54163bb268e4f4f5c79aa2e83ec51f775b16a) )
ROM_END


//-------------------------------------------------
//  ROM( hx20e )
//-------------------------------------------------

ROM_START( ehx20e )
	ROM_REGION( 0x8000, HD6301V1_MAIN_TAG, ROMREGION_ERASEFF )
	ROMX_LOAD( "hx20_v11e.12e", 0x0000, 0x2000, CRC(4de0b4b6) SHA1(f15c537824b7effde9d9b9a21e92a081fb089371), ROM_BIOS(3) )
	ROMX_LOAD( "hx20_v11e.13e", 0x2000, 0x2000, CRC(10d6ae76) SHA1(3163954ed9981f70f590ee98bcc8e19e4be6527a), ROM_BIOS(3) )
	ROMX_LOAD( "hx20_v11e.14e", 0x4000, 0x2000, CRC(26c203a1) SHA1(b282d7233b2689820fcf718dbe1e93d623b67e4f), ROM_BIOS(3) )
	ROMX_LOAD( "hx20_v11e.15e", 0x6000, 0x2000, CRC(fd339aa5) SHA1(860c3579c45e96c5e6a877f4fbe77abacb0d674e), ROM_BIOS(3) )

	ROM_REGION( 0x1000, HD6301V1_SLAVE_TAG, 0 )
	ROM_LOAD( "hd6301v1.6d", 0x0000, 0x1000, CRC(b36f5b99) SHA1(c6b54163bb268e4f4f5c79aa2e83ec51f775b16a) )
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT                     COMPANY   FULLNAME           FLAGS
COMP( 1983, ehx20,  0,      0,       hx20,      hx20,    driver_device,    0,     "Epson",  "Epson HX-20",           MACHINE_NOT_WORKING )
COMP( 1983, ehx20e, ehx20,  0,       hx20,      hx20e,   driver_device,    0,     "Epson",  "Epson HX-20 (Europe)",  MACHINE_NOT_WORKING )
