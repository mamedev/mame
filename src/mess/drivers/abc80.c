/*

Luxor ABC 80

PCB Layout
----------

55 10470-02

          CN1               CN2                                                CN5
  SW1   |-----|  |------------------------|                                  |-----|
|-------|     |--|                        |----------------------------------|     |---|
|                                                             CN3       CN4            |
|                                                    7912                              |
|            MC1488                                                                    |
|   MC1489                                           7812                              |
|            LS245                     LS138                                           |
|                                                   |-------|                          |
|   |-----CN6-----|   LS241   LS241    LS138  LS32  |SN76477| LS04    LM339            |
|                                                   |-------|                          |
|   |--------------|  |------------|   PROM0  LS132   LS273   7406             LS08    |
|   |   Z80A PIO   |  |    Z80A    |                                                   |
|   |--------------|  |------------|   LS04   LS74A   LS86    LS161   LS166    74393   |
|                                                                                      |
|     ROM3   LS107    4116    4116     LS10   LS257   LS74A   LS08    LS107    PROM2   |
|                                                                                      |
|     ROM2   LS257    4116    4116     LS139  74393   LS107   LS32    LS175    74393   |
|                                                                                      |
|     ROM1   LS257    4116    4116     LS08   LS283   LS10    LS32    PROM1    74393   |
|                                                                                      |
|     ROM0   LS257    4116    4116     LS257  74393   LS375   74S263  LS145    PROM4   |
|                                                                                      |
|     DIPSW1 DIPSW2   4045    4045     LS257  LS245   LS375   LS273   LS166    PROM3   |
|--------------------------------------------------------------------------------------|

Notes:
    All IC's shown.

    ROM0-3  - Texas Instruments TMS4732 4Kx8 General Purpose Mask Programmable ROM
    PROM0-2 - Philips Semiconductors N82S129 256x4 TTL Bipolar PROM
    PROM3-4 - Philips Semiconductors N82S131 512x4 TTL Bipolar PROM
    4116    - Texas Instruments TMS4116-25 16Kx1 Dynamic RAM
    4045    - Texas Instruments TMS4045-15 1Kx4 General Purpose Static RAM with Multiplexed I/O
    Z80A    - Sharp LH0080A Z80A CPU
    Z80APIO - SGS-Thomson Z8420AB1 Z80A PIO
    SN76477 - Texas Instruments SN76477N Complex Sound Generator
    74S263  - Texas Instruments SN74S263N Row Output Character Generator
    MC1488  - Texas Instruments MC1488 Quadruple Line Driver
    MC1489  - Texas Instruments MC1489 Quadruple Line Receiver
    CN1     - RS-232 connector
    CN2     - ABC bus connector (DIN 41612)
    CN3     - video connector
    CN4     - cassette motor connector
    CN5     - cassette connector
    CN6     - keyboard connector
    SW1     - reset switch
    DIPSW1  -
    DIPSW2  -

*/

/*

    TODO:

    - cassette interrupt routine samples the latch too soon
    - proper keyboard controller emulation
    - MyAB 80-column card
    - GeJo 80-column card
    - Mikrodatorn 64K expansion
    - floppy
    - printer
    - IEC
    - Metric ABC CAD 1000

*/

#include "includes/abc80.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define MMU_XM		0x01
#define MMU_ROM		0x02
#define MMU_VRAMS	0x04
#define MMU_RAM		0x08



//**************************************************************************
//  KEYBOARD HACK
//**************************************************************************

static INPUT_PORTS_START( fake_keyboard )
	PORT_START("ROW0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("4 \xC2\xA4") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR(0x00A4)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('/')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')

	PORT_START("ROW1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('=')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('+') PORT_CHAR('?')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR(0x00E9) PORT_CHAR(0x00C9)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('<') PORT_CHAR('>')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')

	PORT_START("ROW2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(0x00E5) PORT_CHAR(0x00C5)

	PORT_START("ROW3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(0x00FC) PORT_CHAR(0x00DC)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('\r')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')

	PORT_START("ROW4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(0x00F6) PORT_CHAR(0x00D6)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(0x00E4) PORT_CHAR(0x00C4)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\'') PORT_CHAR('*')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')

	PORT_START("ROW5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR(';')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR(':')

	PORT_START("ROW6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ROW7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LEFT SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RIGHT SHIFT") PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("UPPER CASE") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static const UINT8 abc80_keycodes[7*4][8] =
{
	/* unshifted */
	{ 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38 },
	{ 0x39, 0x30, 0x2B, 0x60, 0x3C, 0x71, 0x77, 0x65 },
	{ 0x72, 0x74, 0x79, 0x75, 0x69, 0x6F, 0x70, 0x7D },
	{ 0x7E, 0x0D, 0x61, 0x73, 0x64, 0x66, 0x67, 0x68 },
	{ 0x6A, 0x6B, 0x6C, 0x7C, 0x7B, 0x27, 0x08, 0x7A },
	{ 0x78, 0x63, 0x76, 0x62, 0x6E, 0x6D, 0x2C, 0x2E },
	{ 0x2D, 0x09, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00 },

	/* shift */
	{ 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x2f, 0x28 },
	{ 0x29, 0x3d, 0x3f, 0x40, 0x3e, 0x51, 0x57, 0x45 },
	{ 0x52, 0x54, 0x59, 0x55, 0x49, 0x4f, 0x50, 0x5d },
	{ 0x5e, 0x0d, 0x41, 0x53, 0x44, 0x46, 0x47, 0x48 },
	{ 0x4a, 0x4b, 0x4c, 0x5c, 0x5b, 0x2a, 0x08, 0x5a },
	{ 0x58, 0x43, 0x56, 0x42, 0x4e, 0x4d, 0x3b, 0x3a },
	{ 0x5f, 0x09, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00 },

	/* control */
	{ 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38 },
	{ 0x39, 0x30, 0x2b, 0x00, 0x7f, 0x11, 0x17, 0x05 },
	{ 0x12, 0x14, 0x19, 0x15, 0x09, 0x0f, 0x10, 0x1d },
	{ 0x1e, 0x0d, 0x01, 0x13, 0x04, 0x06, 0x07, 0x08 },
	{ 0x0a, 0x0b, 0x0c, 0x1c, 0x1b, 0x27, 0x08, 0x1a },
	{ 0x18, 0x03, 0x16, 0x02, 0x0e, 0x0d, 0x2c, 0x2e },
	{ 0x2d, 0x09, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00 },

	/* control-shift */
	{ 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x2f, 0x28 },
	{ 0x29, 0x3d, 0x3f, 0x00, 0x7f, 0x11, 0x17, 0x05 },
	{ 0x12, 0x14, 0x19, 0x15, 0x09, 0x1f, 0x00, 0x1d },
	{ 0x1e, 0x0d, 0x01, 0x13, 0x04, 0x06, 0x07, 0x08 },
	{ 0x0a, 0x1b, 0x1c, 0x1c, 0x1b, 0x2a, 0x08, 0x1a },
	{ 0x18, 0x03, 0x16, 0x02, 0x1e, 0x1d, 0x3b, 0x3a },
	{ 0x5f, 0x09, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00 }
};

void abc80_state::scan_keyboard()
{
	static const char *const keynames[] = { "ROW0", "ROW1", "ROW2", "ROW3", "ROW4", "ROW5", "ROW6" };
	int table = 0, row, col;

	if (ioport("ROW7")->read() & 0x07)
	{
		/* shift, upper case */
		table |= 0x01;
	}

	if (ioport("ROW7")->read() & 0x08)
	{
		/* ctrl */
		table |= 0x02;
	}

	/* clear key strobe */
	m_key_strobe = 0;

	/* scan keyboard */
	for (row = 0; row < 7; row++)
	{
		UINT8 data = ioport(keynames[row])->read();

		for (col = 0; col < 8; col++)
		{
			if (BIT(data, col))
			{
				UINT8 keydata = abc80_keycodes[row + (table * 7)][col];

				/* set key strobe */
				m_key_strobe = 1;

				if (m_key_data != keydata)
				{
					UINT8 pio_data = 0x80 | keydata;

					/* latch key data */
					m_key_data = keydata;

					m_pio->port_a_write(pio_data);
					return;
				}
			}
		}
	}

	if (!m_key_strobe && m_key_data)
	{
		m_pio->port_a_write(m_key_data);

		timer_set(attotime::from_msec(50), TIMER_ID_FAKE_KEYBOARD_CLEAR);
	}
}



//**************************************************************************
//  MEMORY MANAGEMENT
//**************************************************************************

//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( abc80_state::read )
{
	UINT8 data = 0xff;
	UINT8 mmu = m_mmu_rom[0x40 | (offset >> 10)];

	if (!(mmu & MMU_XM))
	{
		data = m_bus->xmemfl_r(space, offset);
	}
	else if (!(mmu & MMU_ROM))
	{
		data = memregion(Z80_TAG)->base()[offset & 0x3fff];
	}
	else if (mmu & MMU_VRAMS)
	{
		data = m_video_ram[offset & 0x3ff];
	}
	else if (!(mmu & MMU_RAM))
	{
		data = m_ram->pointer()[offset & 0x3fff];
	}

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( abc80_state::write )
{
	UINT8 mmu = m_mmu_rom[0x40 | (offset >> 10)];

	if (!(mmu & MMU_XM))
	{
		m_bus->xmemw_w(space, offset, data);
	}
	else if (mmu & MMU_VRAMS)
	{
		m_video_ram[offset & 0x3ff] = data;
	}
	else if (!(mmu & MMU_RAM))
	{
		m_ram->pointer()[offset & 0x3fff] = data;
	}
}



//**************************************************************************
//  SOUND
//**************************************************************************

//-------------------------------------------------
//  vco_voltage_w - CSG VCO voltage select
//-------------------------------------------------

WRITE_LINE_MEMBER( abc80_state::vco_voltage_w )
{
	sn76477_vco_voltage_w(m_psg, state ? 2.5 : 0);
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( abc80_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( abc80_mem, AS_PROGRAM, 8, abc80_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(read, write)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( abc80_io )
//-------------------------------------------------

static ADDRESS_MAP_START( abc80_io, AS_IO, 8, abc80_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0x17)
	AM_RANGE(0x00, 0x00) AM_DEVREADWRITE(ABCBUS_TAG, abcbus_slot_device, inp_r, utp_w)
	AM_RANGE(0x01, 0x01) AM_DEVREADWRITE(ABCBUS_TAG, abcbus_slot_device, stat_r, cs_w)
	AM_RANGE(0x02, 0x02) AM_DEVWRITE(ABCBUS_TAG, abcbus_slot_device, c1_w)
	AM_RANGE(0x03, 0x03) AM_DEVWRITE(ABCBUS_TAG, abcbus_slot_device, c2_w)
	AM_RANGE(0x04, 0x04) AM_DEVWRITE(ABCBUS_TAG, abcbus_slot_device, c3_w)
	AM_RANGE(0x05, 0x05) AM_DEVWRITE(ABCBUS_TAG, abcbus_slot_device, c4_w)
	AM_RANGE(0x06, 0x06) AM_WRITE_PORT("SN76477")
	AM_RANGE(0x07, 0x07) AM_DEVREAD(ABCBUS_TAG, abcbus_slot_device, rst_r)
	AM_RANGE(0x10, 0x13) AM_MIRROR(0x04) AM_DEVREADWRITE(Z80PIO_TAG, z80pio_device, read_alt, write_alt)
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( abc80 )
//-------------------------------------------------

static INPUT_PORTS_START( abc80 )
	PORT_INCLUDE(fake_keyboard)

	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SN76477")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_SPECIAL) PORT_WRITE_LINE_DEVICE(SN76477_TAG, sn76477_enable_w)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, abc80_state, vco_voltage_w)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_WRITE_LINE_DEVICE(SN76477_TAG, sn76477_vco_w)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_WRITE_LINE_DEVICE(SN76477_TAG, sn76477_mixer_b_w)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_WRITE_LINE_DEVICE(SN76477_TAG, sn76477_mixer_a_w)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_WRITE_LINE_DEVICE(SN76477_TAG, sn76477_mixer_c_w)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_WRITE_LINE_DEVICE(SN76477_TAG, sn76477_envelope_2_w)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_WRITE_LINE_DEVICE(SN76477_TAG, sn76477_envelope_1_w)
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  sn76477_interface csg_intf
//-------------------------------------------------

static const sn76477_interface csg_intf =
{
	RES_K(47),		//  4  noise_res        R26 47k
	RES_K(330),		//  5  filter_res       R24 330k
	CAP_P(390),		//  6  filter_cap       C52 390p
	RES_K(47),		//  7  decay_res        R23 47k
	CAP_U(10),		//  8  attack_decay_cap C50 10u/35V
	RES_K(2.2),		// 10  attack_res       R21 2.2k
	RES_K(33),		// 11  amplitude_res    R19 33k
	RES_K(10),		// 12  feedback_res     R18 10k
	0,				// 16  vco_voltage      0V or 2.5V
	CAP_N(10) ,		// 17  vco_cap          C48 10n
	RES_K(100),		// 18  vco_res          R20 100k
	0,				// 19  pitch_voltage    N/C
	RES_K(220),		// 20  slf_res          R22 220k
	CAP_U(1),		// 21  slf_cap          C51 1u/35V
	CAP_U(0.1),		// 23  oneshot_cap      C53 0.1u
	RES_K(330)		// 24  oneshot_res      R25 330k
};


//-------------------------------------------------
//  Z80PIO_INTERFACE( pio_intf )
//-------------------------------------------------

READ8_MEMBER( abc80_state::pio_pa_r )
{
	/*

        PIO Port A

        bit     description

        0       keyboard data
        1       keyboard data
        2       keyboard data
        3       keyboard data
        4       keyboard data
        5       keyboard data
        6       keyboard data
        7       keyboard strobe

    */

    UINT8 data = 0;

    //data |= m_kb->data_r();
    data |= m_key_data;
    data |= (m_key_strobe << 7);

	return data;
};

READ8_MEMBER( abc80_state::pio_pb_r )
{
	/*

        PIO Channel B

        0  R    RS-232C RxD
        1  R    RS-232C _CTS
        2  R    RS-232C _DCD
        3  W    RS-232C TxD
        4  W    RS-232C _RTS
        5  W    Cassette Motor
        6  W    Cassette Data
        7  R    Cassette Data

    */

	UINT8 data = 0;

	// receive data
	data |= m_rs232->rx();

	// clear to send
	data |= m_rs232->cts_r() << 1;

	// data carrier detect
	data |= m_rs232->dcd_r() << 2;

	// cassette data
	data |= m_tape_in_latch << 7;

	//logerror("read tape latch %u\n", m_tape_in_latch);

	return data;
};

WRITE8_MEMBER( abc80_state::pio_pb_w )
{
	/*

        PIO Channel B

        0  R    RS-232C RxD
        1  R    RS-232C _CTS
        2  R    RS-232C _DCD
        3  W    RS-232C TxD
        4  W    RS-232C _RTS
        5  W    Cassette Motor
        6  W    Cassette Data
        7  R    Cassette Data

    */

    // transmit data
    m_rs232->tx(BIT(data, 3));

    // request to send
    m_rs232->rts_w(BIT(data, 4));

	// cassette motor
	if (BIT(data, 5))
	{
		m_cassette->change_state(CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);
		m_cassette_timer->enable(true);
	}
	else
	{
		m_cassette->change_state(CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
		m_cassette_timer->enable(false);
	}

	// cassette data
	m_cassette->output(BIT(data, 6) ? -1.0 : +1.0);

	// cassette input latch
	if (BIT(data, 6))
	{
		//logerror("clear tape in latch\n");

		m_tape_in_latch = 1;

		m_pio->port_b_write(m_tape_in_latch << 7);
	}
};

static Z80PIO_INTERFACE( pio_intf )
{
	DEVCB_CPU_INPUT_LINE(Z80_TAG, INPUT_LINE_IRQ0),	/* callback when change interrupt status */
	DEVCB_DRIVER_MEMBER(abc80_state, pio_pa_r),		/* port A read callback */
	DEVCB_NULL,										/* port A write callback */
	DEVCB_NULL,										/* portA ready active callback */
	DEVCB_DRIVER_MEMBER(abc80_state, pio_pb_r),		/* port B read callback */
	DEVCB_DRIVER_MEMBER(abc80_state, pio_pb_w),		/* port B write callback */
	DEVCB_NULL										/* portB ready active callback */
};


//-------------------------------------------------
//  Z80PIO_INTERFACE( pio_intf )
//-------------------------------------------------

static const z80_daisy_config abc80_daisy_chain[] =
{
	{ Z80PIO_TAG },
	{ NULL }
};


//-------------------------------------------------
//  cassette_interface abc80_cassette_interface
//-------------------------------------------------

static const cassette_interface abc80_cassette_interface =
{
	cassette_default_formats,
	NULL,
	(cassette_state)(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_MUTED),
	NULL,
	NULL
};


//-------------------------------------------------
//  ABC80_KEYBOARD_INTERFACE( kb_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( abc80_state::keydown_w )
{
	m_key_strobe = state;

	m_pio->port_a_write(m_key_strobe << 7);
}

static ABC80_KEYBOARD_INTERFACE( kb_intf )
{
	DEVCB_DRIVER_LINE_MEMBER(abc80_state, keydown_w)
};


//-------------------------------------------------
//  ABCBUS_INTERFACE( abcbus_intf )
//-------------------------------------------------

static ABCBUS_INTERFACE( abcbus_intf )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};


//-------------------------------------------------
//  rs232_port_interface rs232_intf
//-------------------------------------------------

static SLOT_INTERFACE_START( rs232_devices )
SLOT_INTERFACE_END

static const rs232_port_interface rs232_intf =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void abc80_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_ID_PIO:
		m_pio_astb = !m_pio_astb;

		m_pio->strobe_a(m_pio_astb);
		break;

	case TIMER_ID_CASSETTE:
		{
			int tape_in = m_cassette->input() > 0;
			//logerror("tape bit %u\n", tape_in);

			if (m_tape_in_latch && !m_tape_in && tape_in)
			{
				//logerror("-------- set tape in latch\n");
				m_tape_in_latch = 0;

				m_pio->port_b_write(m_tape_in_latch << 7);
			}

			m_tape_in = tape_in;
		}
		break;

	case TIMER_ID_BLINK:
		m_blink = !m_blink;
		break;

	case TIMER_ID_VSYNC_ON:
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
		break;

	case TIMER_ID_VSYNC_OFF:
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
		break;

	case TIMER_ID_FAKE_KEYBOARD_SCAN:
		scan_keyboard();
		break;

	case TIMER_ID_FAKE_KEYBOARD_CLEAR:
		m_key_data = 0;
		break;
	}
}


//-------------------------------------------------
//  MACHINE_START( abc80 )
//-------------------------------------------------

void abc80_state::machine_start()
{
	// start timers
	m_cassette_timer = timer_alloc(TIMER_ID_CASSETTE);
	m_cassette_timer->adjust(attotime::from_hz(44100), 0, attotime::from_hz(44100));
	m_cassette_timer->enable(false);

	m_kb_timer = timer_alloc(TIMER_ID_FAKE_KEYBOARD_SCAN);
	m_kb_timer->adjust(attotime::from_usec(2500), 0, attotime::from_usec(2500));

	// find memory regions
	m_mmu_rom = memregion("mmu")->base();

	// register for state saving
	save_item(NAME(m_key_data));
	save_item(NAME(m_key_strobe));
	save_item(NAME(m_pio_astb));
	save_item(NAME(m_tape_in));
	save_item(NAME(m_tape_in_latch));
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( abc80 )
//-------------------------------------------------

static MACHINE_CONFIG_START( abc80, abc80_state )
	// basic machine hardware
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_11_9808MHz/2/2)	// 2.9952 MHz
	MCFG_CPU_PROGRAM_MAP(abc80_mem)
	MCFG_CPU_IO_MAP(abc80_io)
	MCFG_CPU_CONFIG(abc80_daisy_chain)

	// video hardware
	MCFG_FRAGMENT_ADD(abc80_video)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SN76477_TAG, SN76477, 0)
	MCFG_SOUND_CONFIG(csg_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	// devices
	MCFG_Z80PIO_ADD(Z80PIO_TAG, XTAL_11_9808MHz/2/2, pio_intf)
	MCFG_CASSETTE_ADD(CASSETTE_TAG, abc80_cassette_interface)
	MCFG_ABC80_KEYBOARD_ADD(kb_intf)
	MCFG_ABCBUS_SLOT_ADD(ABCBUS_TAG, abcbus_intf, abcbus_cards, NULL, NULL) // "slow", abc830_slow)
	MCFG_RS232_PORT_ADD(RS232_TAG, rs232_intf, rs232_devices, NULL, NULL)

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("16K")

	// software list
	MCFG_SOFTWARE_LIST_ADD("flop_list", "abc80")
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( abc80 )
//-------------------------------------------------

ROM_START( abc80 )
	ROM_REGION( 0x4000, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS("v2")
	ROM_SYSTEM_BIOS( 0, "v1", "V1" )
	ROMX_LOAD( "3506_3.a5", 0x0000, 0x1000, CRC(7c004fb6) SHA1(9aee1d085122f4537c3e6ecdab9d799bd429ef52), ROM_BIOS(1) )
	ROMX_LOAD( "3507_3.a3", 0x1000, 0x1000, CRC(d1850a84) SHA1(f7719f3af9173601a2aa23ae38ae00de1a387ad8), ROM_BIOS(1) )
	ROMX_LOAD( "3508_3.a4", 0x2000, 0x1000, CRC(b55528e9) SHA1(3e5017e8cacad1f13215242f1bbd89d1d3eee131), ROM_BIOS(1) )
	ROMX_LOAD( "3509_3.a2", 0x3000, 0x1000, CRC(659cab1e) SHA1(181db748cef22cdcccd311a60aa6189c85343db7), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "v2", "V2" )
	ROMX_LOAD( "3506_3_v2.a5", 0x0000, 0x1000, CRC(e2afbf48) SHA1(9883396edd334835a844dcaa792d29599a8c67b9), ROM_BIOS(2) )
	ROMX_LOAD( "3507_3_v2.a3", 0x1000, 0x1000, CRC(d224412a) SHA1(30968054bba7c2aecb4d54864b75a446c1b8fdb1), ROM_BIOS(2) )
	ROMX_LOAD( "3508_3_v2.a4", 0x2000, 0x1000, CRC(1502ba5b) SHA1(5df45909c2c4296e5701c6c99dfaa9b10b3a729b), ROM_BIOS(2) )
	ROMX_LOAD( "3509_3_v2.a2", 0x3000, 0x1000, CRC(bc8860b7) SHA1(28b6cf7f5a4f81e017c2af091c3719657f981710), ROM_BIOS(2) )

	ROM_REGION( 0xa00, "chargen", 0 )
	ROM_LOAD( "sn74s263.h2", 0x0000, 0x0a00, BAD_DUMP CRC(9e064e91) SHA1(354783c8f2865f73dc55918c9810c66f3aca751f) ) // created by hand

	ROM_REGION( 0x100, "hsync", 0 )
	ROM_LOAD( "abc80_11.k5", 0x0000, 0x0100, CRC(e4f7e018) SHA1(63e718a39537f37286ea183e6469808c271dbfa5) ) // "64 40029-01" 82S129 256x4 horizontal sync

	ROM_REGION( 0x200, "vsync", 0 )
	ROM_LOAD( "abc80_21.k2", 0x0000, 0x0200, CRC(445a45b9) SHA1(bcc1c4fafe68b3500b03de785ca32abd63cea252) ) // "64 40030-01" 82S131 512x4 vertical sync

	ROM_REGION( 0x100, "attr", 0 )
	ROM_LOAD( "abc80_12.j3", 0x0000, 0x0100, CRC(6c46811c) SHA1(2d3bdf2d3a2a88ddb1c0c637967e1b2b9541a928) ) // "64 40056-01" 82S129 256x4 attribute

	ROM_REGION( 0x200, "line", 0 )
	ROM_LOAD( "abc80_22.k1", 0x0000, 0x0200, CRC(74de7a0b) SHA1(96f37b0ca65aa8af4242bad38124f410b7f657fe) ) // "64 40058-01" 82S131 512x4 chargen 74S263 row address

	ROM_REGION( 0x100, "mmu", 0 )
	ROM_LOAD( "abc80_13.e7", 0x0000, 0x0100, CRC(f7738834) SHA1(b02df3e678fb50c9cb75b4a97615222d3b4577a3) ) // "64 40057-01" 82S129 256x4 address decoder
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT  COMPAT  MACHINE INPUT  INIT                  COMPANY                FULLNAME    FLAGS
COMP( 1978, abc80,  0,      0,      abc80,  abc80, driver_device,  0,      "Luxor Datorer AB",	"ABC 80",	GAME_SUPPORTS_SAVE | GAME_IMPERFECT_KEYBOARD )
