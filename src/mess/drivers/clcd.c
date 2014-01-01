/***************************************************************************

        Commodore LCD prototype

        G65SC102PI-2
        G65SC51
        M65C22P2 x 2

random notes:
irq handler

fa80
fc00



fa00: make sure monitor is banked in at c000

fb00/fa80 lda/sta called from ram/fixed?
fb00/fa00 lda/sta called from rom/fixed?
fa80/fa00 lda     called from rom/fixed?

****************************************************************************/


#include "emu.h"
#include "cpu/m6502/m65c02.h"
#include "machine/6522via.h"
#include "machine/mos6551.h"
#include "rendlay.h"

class clcd_state : public driver_device
{
public:
	clcd_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_ram(*this,"ram"),
		keyClockState(0),
		keyReadState(0),
		m_col0(*this,"COL0"),
		m_col1(*this,"COL1"),
		m_col2(*this,"COL2"),
		m_col3(*this,"COL3"),
		m_col4(*this,"COL4"),
		m_col5(*this,"COL5"),
		m_col6(*this,"COL6"),
		m_col7(*this,"COL7"),
		m_special(*this,"SPECIAL"),
		m_maincpu(*this, "maincpu"),
		m_via0(*this, "via0")
	{
	}

	TILE_GET_INFO_MEMBER(get_clcd_tilemap_tile_info)
	{
		int code  = m_ram.target()[((tile_index / 80) * 128) + (tile_index % 80) + 0x800];

		SET_TILE_INFO_MEMBER(0, code & 0x7f, ( code & 0x80 ) >> 7, 0);
	}

	virtual void machine_start()
	{
		membank("bankedroms")->configure_entries(0, 0x100, memregion("bankedroms")->base(), 0x400);
	}

	virtual void video_start()
	{
		m_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(clcd_state::get_clcd_tilemap_tile_info),this), TILEMAP_SCAN_ROWS, 6, 8, 80, 16);
	}

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
	{
		m_tilemap->mark_all_dirty();
		m_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		return 0;
	}

	DECLARE_WRITE8_MEMBER(ramwrite_w)
	{
		// this area might be shared between rom & ram or it might be ram only
//      printf( "ram write:%04x %02x\n", offset, data );
	}

	// these seem to control what appears in the memory space at various addresses.
	// whether they just affect data access or instruction fetching as well is unknown.

	DECLARE_WRITE8_MEMBER(fa00_w)
	{
//      printf( "fa00\n" );
	}

	DECLARE_WRITE8_MEMBER(fa80_w)
	{
//      printf( "fa80\n" );
	}

	DECLARE_WRITE8_MEMBER(fb00_w)
	{
//      printf( "fb00\n" );
	}

	DECLARE_WRITE8_MEMBER(fb80_w)
	{
//      printf( "fb80\n" );
	}

	DECLARE_WRITE8_MEMBER(fc00_w)
	{
//      printf( "fc00\n" );
	}

	DECLARE_WRITE8_MEMBER(fc80_w)
	{
//      printf( "fc80\n" );
	}

	DECLARE_WRITE8_MEMBER(fd00_w)
	{
//      printf( "fd00\n" );
	}

	DECLARE_WRITE8_MEMBER(fd80_w)
	{
//      printf( "fd80\n" );
	}

	DECLARE_WRITE8_MEMBER(fe00_w)
	{
//      printf( "fe00\n" );
	}

	DECLARE_WRITE8_MEMBER(fe80_w)
	{
//      printf( "fe80\n" );
	}

	DECLARE_WRITE8_MEMBER(rombank_w)
	{
//      printf( "rom bank %02x\n", data);
		// this might be for ram banking
		membank("bankedroms")->set_entry(0);
	}

	DECLARE_WRITE8_MEMBER(ff80_w)
	{
//      printf( "ff80:%02x %02x\n", offset, data );
	}

	WRITE8_MEMBER( via0_pa_w )
	{
		keyColumnSelect = data;
	}

	int read_column( int column )
	{
		switch( column )
		{
		case 0:
			return m_col0->read();

		case 1:
			return m_col1->read();

		case 2:
			return m_col2->read();

		case 3:
			return m_col3->read();

		case 4:
			return m_col4->read();

		case 5:
			return m_col5->read();

		case 6:
			return m_col6->read();

		case 7:
			return m_col7->read();
		}

		return 0;
	}

	WRITE8_MEMBER( via0_pb_w )
	{
		int newKeyReadState = data & 1;
		if( keyReadState != newKeyReadState )
		{
			keyReadState = newKeyReadState;

			if( newKeyReadState != 0 )
			{
				keyData = m_special->read();

				for( int i = 0; i < 8; i++ )
				{
					if( ( keyColumnSelect & ( 128 >> i ) ) == 0 )
					{
						keyData |= read_column( i ) << 8;
					}
				}

				keyShift = 0x10000;
			}
		}
	}

	WRITE_LINE_MEMBER( via0_cb1_w )
	{
		int newKeyClockState = state & 1;
		if( keyClockState != newKeyClockState )
		{
			keyClockState = newKeyClockState;

			if( keyClockState )
			{
				keyShift >>= 1;

				m_via0->write_cb2( ( keyData & keyShift ) != 0 );
			}
		}
	}

private:
	required_shared_ptr<UINT8> m_ram;
	tilemap_t *m_tilemap;
	int keyData;
	int keyShift;
	int keyColumnSelect;
	int keyClockState;
	int keyReadState;
	virtual void palette_init();
	required_ioport m_col0;
	required_ioport m_col1;
	required_ioport m_col2;
	required_ioport m_col3;
	required_ioport m_col4;
	required_ioport m_col5;
	required_ioport m_col6;
	required_ioport m_col7;
	required_ioport m_special;
	required_device<cpu_device> m_maincpu;
	required_device<via6522_device> m_via0;
};

static ADDRESS_MAP_START( clcd_mem, AS_PROGRAM, 8, clcd_state )
	AM_RANGE(0x0000, 0x3fff) AM_RAM AM_SHARE("ram")
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bankedroms") AM_WRITE(ramwrite_w)
	AM_RANGE(0x8000, 0xf6ff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0xf800, 0xf80f) AM_DEVREADWRITE("via0", via6522_device, read, write)
	AM_RANGE(0xf880, 0xf88f) AM_DEVREADWRITE("via1", via6522_device, read, write)
	AM_RANGE(0xf980, 0xf981) AM_DEVREADWRITE("acia", mos6551_device, read, write)
	AM_RANGE(0xfa00, 0xffff) AM_ROM AM_REGION("maincpu", 0x7a00)
	AM_RANGE(0xfa00, 0xfa00) AM_WRITE(fa00_w)
	AM_RANGE(0xfa80, 0xfa80) AM_WRITE(fa80_w)
	AM_RANGE(0xfb00, 0xfb00) AM_WRITE(fb00_w)
	AM_RANGE(0xfb80, 0xfb80) AM_WRITE(fb80_w)
	AM_RANGE(0xfc00, 0xfc00) AM_WRITE(fc00_w)
	AM_RANGE(0xfc80, 0xfc80) AM_WRITE(fc80_w)
	AM_RANGE(0xfd00, 0xfd00) AM_WRITE(fd00_w)
	AM_RANGE(0xfd80, 0xfd80) AM_WRITE(fd80_w)
	AM_RANGE(0xfe00, 0xfe00) AM_WRITE(fe00_w)
	AM_RANGE(0xfe80, 0xfe80) AM_WRITE(fe80_w)
	AM_RANGE(0xff00, 0xff00) AM_WRITE(rombank_w)
	AM_RANGE(0xff80, 0xff83) AM_WRITE(ff80_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( clcd )
	PORT_START( "COL0" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F7")              PORT_CODE(KEYCODE_F7)           PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("/ ?")             PORT_CODE(KEYCODE_SLASH)        PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(", <")             PORT_CODE(KEYCODE_COMMA)        PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("N")               PORT_CODE(KEYCODE_N)            PORT_CHAR('N')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("V")               PORT_CODE(KEYCODE_V)            PORT_CHAR('V')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("X")               PORT_CODE(KEYCODE_X)            PORT_CHAR('X')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F6")              PORT_CODE(KEYCODE_F6)           PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("@")               PORT_CODE(KEYCODE_QUOTE)        PORT_CHAR('@')

	PORT_START( "COL1" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Q")               PORT_CODE(KEYCODE_Q)            PORT_CHAR('Q')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("+")               PORT_CODE(KEYCODE_PLUS_PAD)     PORT_CHAR('+')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("-")               PORT_CODE(KEYCODE_MINUS)        PORT_CHAR('-')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("O")               PORT_CODE(KEYCODE_O)            PORT_CHAR('O')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("U")               PORT_CODE(KEYCODE_U)            PORT_CHAR('U')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("T")               PORT_CODE(KEYCODE_T)            PORT_CHAR('T')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("E")               PORT_CODE(KEYCODE_E)            PORT_CHAR('E')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F3")              PORT_CODE(KEYCODE_F3)           PORT_CHAR(UCHAR_MAMEKEY(F3))

	PORT_START( "COL2" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F8")              PORT_CODE(KEYCODE_F8)           PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("=")               PORT_CODE(KEYCODE_EQUALS)       PORT_CHAR('=')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(": [")             PORT_CODE(KEYCODE_OPENBRACE)    PORT_CHAR(':') PORT_CHAR('[')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("K")               PORT_CODE(KEYCODE_K)            PORT_CHAR('K')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("H")               PORT_CODE(KEYCODE_H)            PORT_CHAR('H')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F")               PORT_CODE(KEYCODE_F)            PORT_CHAR('F')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("S")               PORT_CODE(KEYCODE_S)            PORT_CHAR('S')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F2")              PORT_CODE(KEYCODE_F2)           PORT_CHAR(UCHAR_MAMEKEY(F2))

	PORT_START( "COL3" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SPACE")           PORT_CODE(KEYCODE_SPACE)        PORT_CHAR(' ')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ESC")             PORT_CODE(KEYCODE_ESC)          PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(". >")             PORT_CODE(KEYCODE_STOP)         PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("M")               PORT_CODE(KEYCODE_M)            PORT_CHAR('M')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("B")               PORT_CODE(KEYCODE_B)            PORT_CHAR('B')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("C")               PORT_CODE(KEYCODE_C)            PORT_CHAR('C')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Z")               PORT_CODE(KEYCODE_Z)            PORT_CHAR('Z')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F1")              PORT_CODE(KEYCODE_F1)           PORT_CHAR(UCHAR_MAMEKEY(F1))

	PORT_START( "COL4" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("2 \"")            PORT_CODE(KEYCODE_2)            PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RIGHT")           PORT_CODE(KEYCODE_RIGHT)        PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("UP")              PORT_CODE(KEYCODE_UP)           PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("0 ^")             PORT_CODE(KEYCODE_0)            PORT_CHAR('0')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("8 (")             PORT_CODE(KEYCODE_8)            PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("6 &")             PORT_CODE(KEYCODE_6)            PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("4 $")             PORT_CODE(KEYCODE_4)            PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F4")              PORT_CODE(KEYCODE_F4)           PORT_CHAR(UCHAR_MAMEKEY(F4))

	PORT_START( "COL5" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F5")              PORT_CODE(KEYCODE_F5)           PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("; ]")             PORT_CODE(KEYCODE_CLOSEBRACE)   PORT_CHAR(';') PORT_CHAR(']')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("L")               PORT_CODE(KEYCODE_L)            PORT_CHAR('L')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("J")               PORT_CODE(KEYCODE_J)            PORT_CHAR('J')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("G")               PORT_CODE(KEYCODE_G)            PORT_CHAR('G')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("D")               PORT_CODE(KEYCODE_D)            PORT_CHAR('D')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("A")               PORT_CODE(KEYCODE_A)            PORT_CHAR('A')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("TAB")             PORT_CODE(KEYCODE_TAB)          PORT_CHAR('\t')

	PORT_START( "COL6" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("HOME CLR")        PORT_CODE(KEYCODE_HOME)         PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("*")               PORT_CODE(KEYCODE_BACKSLASH)    PORT_CHAR('*')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("P")               PORT_CODE(KEYCODE_P)            PORT_CHAR('P')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("I")               PORT_CODE(KEYCODE_I)            PORT_CHAR('I')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Y")               PORT_CODE(KEYCODE_Y)            PORT_CHAR('Y')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("R")               PORT_CODE(KEYCODE_R)            PORT_CHAR('R')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("W")               PORT_CODE(KEYCODE_W)            PORT_CHAR('W')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RETURN")          PORT_CODE(KEYCODE_ENTER)        PORT_CHAR('\r')

	PORT_START( "COL7" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("1 !")             PORT_CODE(KEYCODE_1)            PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LEFT")            PORT_CODE(KEYCODE_LEFT)         PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("DOWN")            PORT_CODE(KEYCODE_DOWN)         PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("9 )")             PORT_CODE(KEYCODE_9)            PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("7 \'")            PORT_CODE(KEYCODE_7)            PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("5 %")             PORT_CODE(KEYCODE_5)            PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("3 #")             PORT_CODE(KEYCODE_3)            PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("DEL INST")        PORT_CODE(KEYCODE_BACKSPACE)    PORT_CHAR('\b') PORT_CHAR(UCHAR_MAMEKEY(INSERT))

	PORT_START( "SPECIAL" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("STOP")            PORT_CODE(KEYCODE_END)          PORT_CHAR(UCHAR_MAMEKEY(END))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CAPSLOCK")        PORT_CODE(KEYCODE_CAPSLOCK)     PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LEFT SHIFT")      PORT_CODE(KEYCODE_LSHIFT)       PORT_CHAR(UCHAR_MAMEKEY(LSHIFT))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CONTROL")         PORT_CODE(KEYCODE_LCONTROL)     PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CBM")             PORT_CODE(KEYCODE_LALT)         PORT_CHAR(UCHAR_MAMEKEY(LALT))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // clears screen and goes into infinite loop
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // clears screen and goes into infinite loop
INPUT_PORTS_END


void clcd_state::palette_init()
{
	palette_set_color(machine(), 0, MAKE_RGB(32,240,32));
	palette_set_color(machine(), 1, MAKE_RGB(32,32,32));

	palette_set_color(machine(), 2, MAKE_RGB(32,32,32));
	palette_set_color(machine(), 3, MAKE_RGB(32,240,32));
}

static const gfx_layout charset_8x8 =
{
	6,8,
	128,
	1,
	{ 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	6*8
};

static GFXDECODE_START( clcd )
	GFXDECODE_ENTRY( "maincpu", 0x7700, charset_8x8, 0, 1 )
GFXDECODE_END

static MACHINE_CONFIG_START( clcd, clcd_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M65C02, 2000000)
	MCFG_CPU_PROGRAM_MAP(clcd_mem)

	MCFG_DEVICE_ADD("via0", VIA6522, 0)
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(clcd_state, via0_pa_w))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(clcd_state, via0_pb_w))
	MCFG_VIA6522_CB1_HANDLER(WRITELINE(clcd_state, via0_cb1_w))
	MCFG_VIA6522_IRQ_HANDLER(DEVWRITELINE("maincpu", m65c02_device, irq_line))

	MCFG_DEVICE_ADD("via1", VIA6522, 0)
	MCFG_VIA6522_IRQ_HANDLER(DEVWRITELINE("maincpu", m65c02_device, nmi_line))

	MCFG_DEVICE_ADD("acia", MOS6551, XTAL_1_8432MHz)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(80)
	MCFG_SCREEN_UPDATE_DRIVER(clcd_state, screen_update)
	MCFG_SCREEN_SIZE(480, 128)
	MCFG_SCREEN_VISIBLE_AREA(0, 480-1, 0, 128-1)

	MCFG_DEFAULT_LAYOUT(layout_lcd)

	MCFG_PALETTE_LENGTH(4)
	MCFG_GFXDECODE(clcd)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( clcd )

	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "kizapr.u102",        0x00000, 0x8000, CRC(59103d52) SHA1(e49c20b237a78b54c2cb26b133d5903bb60bd8ef))

	ROM_REGION( 0x40000, "bankedroms", 0 )
	ROM_LOAD( "sizapr.u103",        0x00000, 0x8000, CRC(0aa91d9f) SHA1(f0842f370607f95d0a0ec6afafb81bc063c32745))
	ROM_LOAD( "sept-m-13apr.u104",  0x08000, 0x8000, CRC(41028c3c) SHA1(fcab6f0bbeef178eb8e5ecf82d9c348d8f318a8f))
	ROM_LOAD( "ss-calc-13apr.u105", 0x10000, 0x8000, CRC(88a587a7) SHA1(b08f3169b7cd696bb6a9b6e6e87a077345377ac4))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY                         FULLNAME       FLAGS */
COMP( 1985, clcd,   0,      0,       clcd,      clcd, driver_device,     0, "Commodore Business Machines", "LCD (Prototype)", GAME_NOT_WORKING | GAME_NO_SOUND )
