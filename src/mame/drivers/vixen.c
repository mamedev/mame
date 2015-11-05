// license:BSD-3-Clause
// copyright-holders:Curt Coder, Robbbert
/*

Osborne 4 Vixen

Main PCB Layout
---------------

TODO

Notes:
    Relevant IC's shown.

    CPU     - Zilog Z8400APS Z80A CPU
    FDC     - SMC FDC1797
    8155    - Intel P8155H
    ROM0    -
    ROM1,2  - AMD AM2732-1DC 4Kx8 EPROM
    CN1     - keyboard connector
    CN2     -
    CN3     -
    CN4     - floppy connector
    CN5     - power connector
    CN6     - composite video connector
    SW1     - reset switch
    SW2     -


I/O PCB Layout
--------------

TODO

Notes:
    Relevant IC's shown.

    8155    - Intel P8155H
    8251    - AMD P8251A
    CN1     - IEEE488 connector
    CN2     - RS232 connector
    CN3     -

*/

/*

    TODO:

    - RS232 RI interrupt
    - PCB layouts

*/


#include "includes/vixen.h"
#include "softlist.h"


//**************************************************************************
//  INTERRUPTS
//**************************************************************************

//-------------------------------------------------
//  update_interrupt -
//-------------------------------------------------

void vixen_state::update_interrupt()
{
	int state = (m_cmd_d1 && m_fdint) || m_vsync;// || (!m_enb_srq_int && !m_srq) || (!m_enb_atn_int && !m_atn) || (!m_enb_xmt_int && m_txrdy) || (!m_enb_rcv_int && m_rxrdy);

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, state ? ASSERT_LINE : CLEAR_LINE);
}


READ8_MEMBER( vixen_state::opram_r )
{
	membank("bank3")->set_entry(0); // read videoram
	return m_program->read_byte(offset);
}

READ8_MEMBER( vixen_state::oprom_r )
{
	membank("bank3")->set_entry(1); // read rom
	return m_rom[offset];
}

//-------------------------------------------------
//  status_r - status read
//-------------------------------------------------

READ8_MEMBER( vixen_state::status_r )
{
	/*

	    bit     description

	    0       VSYNC enable
	    1       FDINT enable
	    2       VSYNC
	    3       1
	    4       1
	    5       1
	    6       1
	    7       1

	*/

	UINT8 data = 0xf8;

	// vertical sync interrupt enable
	data |= m_cmd_d0;

	// floppy interrupt enable
	data |= m_cmd_d1 << 1;

	// vertical sync
	data |= m_vsync << 2;

	return data;
}


//-------------------------------------------------
//  cmd_w - command write
//-------------------------------------------------

WRITE8_MEMBER( vixen_state::cmd_w )
{
	/*

	    bit     description

	    0       VSYNC enable
	    1       FDINT enable
	    2
	    3
	    4
	    5
	    6
	    7

	*/

//  logerror("CMD %u\n", data);

	// vertical sync interrupt enable
	m_cmd_d0 = BIT(data, 0);

	if (!m_cmd_d0)
	{
		// clear vertical sync
		m_vsync = 0;
	}

	// floppy interrupt enable
	m_cmd_d1 = BIT(data, 1);

	update_interrupt();
}


//-------------------------------------------------
//  ieee488_r - IEEE488 bus read
//-------------------------------------------------

READ8_MEMBER( vixen_state::ieee488_r )
{
	/*

	    bit     description

	    0       ATN
	    1       DAV
	    2       NDAC
	    3       NRFD
	    4       EOI
	    5       SRQ
	    6       IFC
	    7       REN

	*/

	UINT8 data = 0;

	/* attention */
	data |= m_ieee488->atn_r();

	/* data valid */
	data |= m_ieee488->dav_r() << 1;

	/* data not accepted */
	data |= m_ieee488->ndac_r() << 2;

	/* not ready for data */
	data |= m_ieee488->nrfd_r() << 3;

	/* end or identify */
	data |= m_ieee488->eoi_r() << 4;

	/* service request */
	data |= m_ieee488->srq_r() << 5;

	/* interface clear */
	data |= m_ieee488->ifc_r() << 6;

	/* remote enable */
	data |= m_ieee488->ren_r() << 7;

	return data;
}


//-------------------------------------------------
//  port3_r - serial status read
//-------------------------------------------------

READ8_MEMBER( vixen_state::port3_r )
{
	/*

	    bit     description

	    0       RI
	    1       DCD
	    2       1
	    3       1
	    4       1
	    5       1
	    6       1
	    7       1

	*/

	UINT8 data = 0xfc;

	// ring indicator
	data |= m_rs232->ri_r();

	// data carrier detect
	data |= m_rs232->dcd_r() << 1;

	return data;
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( vixen_mem )
//-------------------------------------------------

// when M1 is inactive: read and write of data
static ADDRESS_MAP_START( vixen_mem, AS_PROGRAM, 8, vixen_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xffff) AM_READ_BANK("bank3") AM_WRITE_BANK("bank4") AM_SHARE("video_ram")
ADDRESS_MAP_END

// when M1 is active: read opcodes
static ADDRESS_MAP_START( bios_mem, AS_DECRYPTED_OPCODES, 8, vixen_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xefff) AM_READ(opram_r)
	AM_RANGE(0xf000, 0xffff) AM_READ(oprom_r)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( vixen_io )
//-------------------------------------------------

static ADDRESS_MAP_START( vixen_io, AS_IO, 8, vixen_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE(FDC1797_TAG, fd1797_t, read, write)
	AM_RANGE(0x04, 0x04) AM_MIRROR(0x03) AM_READWRITE(status_r, cmd_w)
	AM_RANGE(0x08, 0x08) AM_MIRROR(0x01) AM_DEVREADWRITE(P8155H_TAG, i8155_device, read, write)
	AM_RANGE(0x0c, 0x0d) AM_DEVWRITE(P8155H_TAG, i8155_device, ale_w)
	AM_RANGE(0x10, 0x10) AM_MIRROR(0x07) AM_DEVREAD(IEEE488_TAG, ieee488_device, dio_r)
	AM_RANGE(0x18, 0x18) AM_MIRROR(0x07) AM_READ(ieee488_r)
	AM_RANGE(0x20, 0x21) AM_MIRROR(0x04) AM_DEVWRITE(P8155H_IO_TAG, i8155_device, ale_w)
	AM_RANGE(0x28, 0x28) AM_MIRROR(0x05) AM_DEVREADWRITE(P8155H_IO_TAG, i8155_device, read, write)
	AM_RANGE(0x30, 0x30) AM_MIRROR(0x06) AM_DEVREADWRITE(P8251A_TAG, i8251_device, data_r, data_w)
	AM_RANGE(0x31, 0x31) AM_MIRROR(0x06) AM_DEVREADWRITE(P8251A_TAG, i8251_device, status_r, control_w)
	AM_RANGE(0x38, 0x38) AM_MIRROR(0x07) AM_READ(port3_r)
//  AM_RANGE(0xf0, 0xff) Hard Disk?
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( vixen )
//-------------------------------------------------

INPUT_PORTS_START( vixen )
	PORT_START("KEY.0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(0x1B)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB) PORT_CHAR(0x09)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(0x0D)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(0x27) PORT_CHAR(0x22)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR(']')

	PORT_START("KEY.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')

	PORT_START("KEY.2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q') PORT_CHAR(0x11)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w') PORT_CHAR(0x17)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e') PORT_CHAR(0x05)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r') PORT_CHAR(0x12)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t') PORT_CHAR(0x14)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y') PORT_CHAR(0x19)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u') PORT_CHAR(0x15)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i') PORT_CHAR(0x09)

	PORT_START("KEY.3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a') PORT_CHAR(0x01)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s') PORT_CHAR(0x13)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d') PORT_CHAR(0x04)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f') PORT_CHAR(0x06)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g') PORT_CHAR(0x07)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h') PORT_CHAR(0x08)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j') PORT_CHAR(0x0a)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k') PORT_CHAR(0x0b)

	PORT_START("KEY.4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z') PORT_CHAR(0x1a)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x') PORT_CHAR(0x18)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c') PORT_CHAR(0x03)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v') PORT_CHAR(0x16)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b') PORT_CHAR(0x02)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n') PORT_CHAR(0x0e)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m') PORT_CHAR(0x0d)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')

	PORT_START("KEY.5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("UP") PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LEFT) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p') PORT_CHAR(0x10)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o') PORT_CHAR(0x0f)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')

	PORT_START("KEY.6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RIGHT") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DOWN") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("- _") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_') PORT_CHAR(0x1F)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?') PORT_CHAR(0x7E)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\\ |") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|') PORT_CHAR(0x1C)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+') PORT_CHAR(0x60)

	PORT_START("KEY.7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL) PORT_CHAR(127)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('{') PORT_CHAR('}')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("FUNC") PORT_CODE(KEYCODE_END)
INPUT_PORTS_END



//**************************************************************************
//  VIDEO
//**************************************************************************

//-------------------------------------------------
//  TIMER_DEVICE_CALLBACK_MEMBER( vsync_tick )
//-------------------------------------------------

TIMER_DEVICE_CALLBACK_MEMBER(vixen_state::vsync_tick)
{
	if (m_cmd_d0)
	{
		m_vsync = 1;
		update_interrupt();
	}
}


void vixen_state::video_start()
{
	// register for state saving
	save_item(NAME(m_alt));
	save_item(NAME(m_256));
	save_item(NAME(m_vsync));
}


//-------------------------------------------------
//  SCREEN_UPDATE( vixen )
//-------------------------------------------------

UINT32 vixen_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *pen = m_palette->pens();
	UINT8 x, y, chr, gfx, inv, ra;

	for (y = 0; y < 26; y++)
	{
		for (ra = 0; ra < 10; ra++)
		{
			for (x = 0; x < 128; x++)
			{
				UINT16 sync_addr = ((y+1) << 7) + x + 1; // it's out by a row and a column
				UINT8 sync_data = m_sync_rom[sync_addr & 0xfff];
				bool blank = BIT(sync_data, 4);
				/*
				int clrchadr = BIT(sync_data, 7);
				int hsync = BIT(sync_data, 6);
				int clrtxadr = BIT(sync_data, 5);
				int vsync = BIT(sync_data, 3);
				int comp_sync = BIT(sync_data, 2);

				logerror("SYNC %03x:%02x TXADR %u SCAN %u CHADR %u : COMPSYNC %u VSYNC %u BLANK %u CLRTXADR %u HSYNC %u CLRCHADR %u\n",
				    sync_addr,sync_data,txadr,scan,chadr,comp_sync,vsync,blank,clrtxadr,hsync,clrchadr);
				*/

				chr = m_video_ram[(y<<7) + x];

				if (m_256)
				{
					gfx = m_char_rom[(BIT(chr, 7) << 11) | (ra << 7) | (chr & 0x7f)];
					inv = m_alt ? 0xff : 0;
				}
				else
				{
					gfx = m_char_rom[(ra << 7) | (chr & 0x7f)];
					inv = BIT(chr, 7) ? 0xff : 0;
				}

				gfx = (blank) ? 0 : (gfx ^ inv);

				for (int b = 0; b < 8; b++)
				{
					int color = BIT(gfx, 7 - b);

					bitmap.pix32((y * 10) + ra, (x * 8) + b) = pen[color];
				}
			}
		}
	}

	return 0;
}



//**************************************************************************
//  SOUND
//**************************************************************************

//-------------------------------------------------
//  DISCRETE_SOUND( vixen )
//-------------------------------------------------

static DISCRETE_SOUND_START( vixen )
	DISCRETE_INPUT_LOGIC(NODE_01)
	DISCRETE_SQUAREWAVE(NODE_02, NODE_01, XTAL_23_9616MHz/15360, 100, 50, 0, 90)
	DISCRETE_OUTPUT(NODE_02, 2000)
DISCRETE_SOUND_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  I8155 interface
//-------------------------------------------------

READ8_MEMBER( vixen_state::i8155_pa_r )
{
	UINT8 data = 0xff;

	for (int i = 0; i < 8; i++)
		if (!BIT(m_col, i)) data &= m_key[i]->read();

	return data;
}

WRITE8_MEMBER( vixen_state::i8155_pb_w )
{
	m_col = data;
}

WRITE8_MEMBER( vixen_state::i8155_pc_w )
{
	/*

	    bit     description

	    0       DSEL1/
	    1       DSEL2/
	    2       DDEN/
	    3       ALT CHARSET/
	    4       256 CHARS
	    5       BEEP ENABLE
	    6
	    7

	*/

	// drive select
	floppy_image_device *floppy = NULL;

	if (!BIT(data, 0)) floppy = m_floppy0->get_device();
	if (!BIT(data, 1)) floppy = m_floppy1->get_device();

	m_fdc->set_floppy(floppy);

	if (floppy) floppy->mon_w(0);

	// density select
	m_fdc->dden_w(BIT(data, 2));

	// charset
	m_alt = !BIT(data, 3);
	m_256 = !BIT(data, 4);

	// beep enable
	m_discrete->write(space, NODE_01, !BIT(data, 5));
}

//-------------------------------------------------
//  I8155 IO interface
//-------------------------------------------------

WRITE8_MEMBER( vixen_state::io_i8155_pb_w )
{
	/*

	    bit     description

	    PB0     ATN
	    PB1     DAV
	    PB2     NDAC
	    PB3     NRFD
	    PB4     EOI
	    PB5     SRQ
	    PB6     IFC
	    PB7     REN

	*/

	/* data valid */
	m_ieee488->atn_w(BIT(data, 0));

	/* end or identify */
	m_ieee488->dav_w(BIT(data, 1));

	/* remote enable */
	m_ieee488->ndac_w(BIT(data, 2));

	/* attention */
	m_ieee488->nrfd_w(BIT(data, 3));

	/* interface clear */
	m_ieee488->eoi_w(BIT(data, 4));

	/* service request */
	m_ieee488->srq_w(BIT(data, 5));

	/* not ready for data */
	m_ieee488->ifc_w(BIT(data, 6));

	/* data not accepted */
	m_ieee488->ren_w(BIT(data, 7));
}

WRITE8_MEMBER( vixen_state::io_i8155_pc_w )
{
	/*

	    bit     description

	    PC0     select internal clock
	    PC1     ENB RING INT
	    PC2     ENB RCV INT
	    PC3     ENB XMT INT
	    PC4     ENB ATN INT
	    PC5     ENB SRQ INT
	    PC6
	    PC7

	*/

	m_int_clk = BIT(data, 0);
	m_enb_ring_int = BIT(data, 1);
	m_enb_rcv_int = BIT(data, 2);
	m_enb_xmt_int = BIT(data, 3);
	m_enb_atn_int = BIT(data, 4);
	m_enb_srq_int = BIT(data, 5);
}

WRITE_LINE_MEMBER( vixen_state::io_i8155_to_w )
{
	if (m_int_clk)
	{
		m_usart->write_txc(state);
		m_usart->write_rxc(state);
	}
}

//-------------------------------------------------
//  i8251_interface usart_intf
//-------------------------------------------------

WRITE_LINE_MEMBER( vixen_state::rxrdy_w )
{
	m_rxrdy = state;
	update_interrupt();
}

WRITE_LINE_MEMBER( vixen_state::txrdy_w )
{
	m_txrdy = state;
	update_interrupt();
}

//-------------------------------------------------
//  IEEE488_INTERFACE( ieee488_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( vixen_state::srq_w )
{
	m_srq = state;
	update_interrupt();
}

WRITE_LINE_MEMBER( vixen_state::atn_w )
{
	m_atn = state;
	update_interrupt();
}

static SLOT_INTERFACE_START( vixen_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END

WRITE_LINE_MEMBER( vixen_state::fdc_intrq_w )
{
	m_fdint = state;
	update_interrupt();
}



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  IRQ_CALLBACK_MEMBER( vixen_int_ack )
//-------------------------------------------------

IRQ_CALLBACK_MEMBER(vixen_state::vixen_int_ack)
{
	// D0 is pulled low
	return 0xfe;
}


//-------------------------------------------------
//  MACHINE_START( vixen )
//-------------------------------------------------

void vixen_state::machine_start()
{
	// configure memory banking

	membank("bank3")->configure_entry(0, m_video_ram);
	membank("bank3")->configure_entry(1, m_rom);

	membank("bank4")->configure_entry(0, m_video_ram);

	// register for state saving
	save_item(NAME(m_col));
	save_item(NAME(m_cmd_d0));
	save_item(NAME(m_cmd_d1));
	save_item(NAME(m_fdint));
}


void vixen_state::machine_reset()
{
	membank("bank3")->set_entry(1);

	m_vsync = 0;
	m_cmd_d0 = 0;
	m_cmd_d1 = 0;
	update_interrupt();

	m_fdc->reset();
	m_io_i8155->reset();
	m_usart->reset();
	m_maincpu->set_state_int(Z80_PC, 0xf000);
}



//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( vixen )
//-------------------------------------------------

static MACHINE_CONFIG_START( vixen, vixen_state )
	// basic machine hardware
	MCFG_CPU_ADD(Z8400A_TAG, Z80, XTAL_23_9616MHz/6)
	MCFG_CPU_PROGRAM_MAP(vixen_mem)
	MCFG_CPU_DECRYPTED_OPCODES_MAP(bios_mem)
	MCFG_CPU_IO_MAP(vixen_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(vixen_state,vixen_int_ack)

	// video hardware
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(vixen_state, screen_update)
	MCFG_SCREEN_RAW_PARAMS(XTAL_23_9616MHz/2, 96*8, 0*8, 81*8, 27*10, 0*10, 26*10)

	MCFG_TIMER_DRIVER_ADD_SCANLINE("vsync", vixen_state, vsync_tick, SCREEN_TAG, 26*10, 27*10)

	MCFG_PALETTE_ADD_MONOCHROME_AMBER("palette")

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(DISCRETE_TAG, DISCRETE, 0)
	MCFG_DISCRETE_INTF(vixen)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)

	// devices
	MCFG_DEVICE_ADD(P8155H_TAG, I8155, XTAL_23_9616MHz/6)
	MCFG_I8155_IN_PORTA_CB(READ8(vixen_state, i8155_pa_r))
	MCFG_I8155_OUT_PORTB_CB(WRITE8(vixen_state, i8155_pb_w))
	MCFG_I8155_OUT_PORTC_CB(WRITE8(vixen_state, i8155_pc_w))

	MCFG_DEVICE_ADD(P8155H_IO_TAG, I8155, XTAL_23_9616MHz/6)
	MCFG_I8155_OUT_PORTA_CB(DEVWRITE8(IEEE488_TAG, ieee488_device, dio_w))
	MCFG_I8155_OUT_PORTB_CB(WRITE8(vixen_state, io_i8155_pb_w))
	MCFG_I8155_OUT_PORTC_CB(WRITE8(vixen_state, io_i8155_pc_w))
	MCFG_I8155_OUT_TIMEROUT_CB(WRITELINE(vixen_state, io_i8155_to_w))

	MCFG_DEVICE_ADD(P8251A_TAG, I8251, 0)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_rts))
	MCFG_I8251_RXRDY_HANDLER(WRITELINE(vixen_state, rxrdy_w))
	MCFG_I8251_TXRDY_HANDLER(WRITELINE(vixen_state, txrdy_w))

	MCFG_RS232_PORT_ADD(RS232_TAG, default_rs232_devices, NULL)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(P8251A_TAG, i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE(P8251A_TAG, i8251_device, write_dsr))

	MCFG_FD1797_ADD(FDC1797_TAG, XTAL_23_9616MHz/24)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(vixen_state, fdc_intrq_w))
	MCFG_FLOPPY_DRIVE_ADD(FDC1797_TAG":0", vixen_floppies, "525dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD(FDC1797_TAG":1", vixen_floppies, "525dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_IEEE488_BUS_ADD()
	MCFG_IEEE488_SRQ_CALLBACK(WRITELINE(vixen_state, srq_w))
	MCFG_IEEE488_ATN_CALLBACK(WRITELINE(vixen_state, atn_w))

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("disk_list", "vixen")

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( vixen )
//-------------------------------------------------

ROM_START( vixen )
	ROM_REGION( 0x1000, Z8400A_TAG, 0 )
	ROM_LOAD( "osborne 4 mon rom v1.04 3p40082-03 a0a9.4c", 0x0000, 0x1000, CRC(5f1038ce) SHA1(e6809fac23650bbb4689e58edc768d917d80a2df) ) // OSBORNE 4 MON ROM / V1.04  3P40082-03 / A0A9 (c) OCC 1985

	ROM_REGION( 0x1000, "video", 0 )
	ROM_LOAD( "v1.10.3j", 0x0000, 0x1000, CRC(1f93e2d7) SHA1(0c479bfd3ac8d9959c285c020d0096930a9c6867) )

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "v1.00 l.1j", 0x0000, 0x1000, CRC(f97c50d9) SHA1(39f73afad68508c4b8a4d241c064f9978098d8f2) )
ROM_END



//**************************************************************************
//  DRIVER INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  DRIVER_INIT( vixen )
//-------------------------------------------------


DRIVER_INIT_MEMBER(vixen_state,vixen)
{
	m_program = &m_maincpu->space(AS_PROGRAM);
}



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT  COMPAT  MACHINE    INPUT    CLASS         INIT    COMPANY      FULLNAME       FLAGS
COMP( 1984, vixen,  0,       0,     vixen,     vixen,   vixen_state,  vixen,  "Osborne",   "Vixen",      0 )
