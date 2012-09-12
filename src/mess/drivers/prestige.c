/*

VTech PreComputer Prestige Elite

PCB Layout
----------

|-------------------------------------------|
|   |-------CN1-------|     CN2             |
|                                           |
|                                       CN3 |
|                                           |
|CN10                                       |
|CN9            RAM                         --|
|CN8                                          |
|                                             |
|                       Z80         ROM       |
|               04010                     CN4 |
|CN7                                          |
|CN6                                          |
|                       |------CN5------|   --|
|-------------------------------------------|

Notes:
    All IC's shown.

    ROM     - VTech LH5S8R14, labeled "1998 27-6020-02" (dumped as 1Mx8)
    Z80     - Z80 family SOC?
    RAM     - LG Semicon GM76C256CLLFW55 32Kx8 Static RAM
    04010   - ?
    CN1     - Centronics connector
    CN2     - mouse connector
    CN3     - LCD ribbon cable
    CN4     - expansion connector
    CN5     - keyboard ribbon cable
    CN6     - speaker wire
    CN7     - volume wire
    CN8     - ? wire
    CN9     - power wire
    CN10    - NVRAM battery wire
*/

/*

    Undumped cartridges:

    80-1410   Super Science
    80-1533   Famous Things & Places
    80-0989   Bible Knowledge
    80-1001   Fantasy Trivia
    80-1002   General Knowledge II
    80-1003   Sports History
    80-2314   Familiar Faces
    80-2315   Historical Happenings
    80-2333   Arts, Entertainment & More
    80-2334   Customs & Cultures
    80-1531   32K RAM Memory Expansion Cartridge
    80-12051  Space Scholar
    80-12053  Frenzy of Facts
    80-12052  Spreadsheet Success

*/

/*

    TODO:

    - identify unknown chips (maybe related to the sound??)
    - better IRQ timing
    - the mouse is too slow and sometime freezes
    - finish to decode the memory banking
    - sound
    - cartridges

*/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/ram.h"
#include "imagedev/cartslot.h"
#include "rendlay.h"


class prestige_state : public driver_device
{
public:
	prestige_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_ram(*this, RAM_TAG)
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;

	UINT8 m_bank[7];
	UINT8 m_kb_matrix;
	UINT8 m_irq_counter;
	UINT8 m_mousex;
	UINT8 m_mousey;
	UINT8 *m_vram;

	virtual void machine_start();
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER( bankswitch_r );
	DECLARE_WRITE8_MEMBER( bankswitch_w );
	DECLARE_READ8_MEMBER( kb_r );
	DECLARE_WRITE8_MEMBER( kb_w );
	DECLARE_READ8_MEMBER( mouse_r );
	DECLARE_WRITE8_MEMBER( mouse_w );
};


READ8_MEMBER( prestige_state::bankswitch_r )
{
	return m_bank[offset];
}

WRITE8_MEMBER( prestige_state::bankswitch_w )
{
	address_space *program = m_maincpu->space(AS_PROGRAM);

	switch (offset)
	{
	case 0:
		membank("bank1")->set_entry(data & 0x3f);
		break;

	case 1:
		if (m_bank[5] & 0x08)
			membank("bank2")->set_entry(0x40 + (data & 1));
		else
			membank("bank2")->set_entry(data & 0x3f);
		break;

	case 2:
		if (m_bank[5] & 0x04)
			membank("bank3")->set_entry(0x40 + (data & 1));
		else
			membank("bank3")->set_entry(data & 0x3f);
		break;

	case 3:
		if (m_bank[5] & 0x02)
			membank("bank4")->set_entry(0x04 + (data & 0x03));
		else
			membank("bank4")->set_entry(data & 0x03);
		break;

	case 4:
		membank("bank5")->set_entry(data & 0x03);
		break;

	case 5:
		if (ioport("CART_TYPE")->read() == 0x01)
		{
			//cartridge memory is writable
			if (data & 0x08)
				program->install_readwrite_bank(0x4000, 0x7fff, "bank2");
			else
				program->unmap_write(0x4000, 0x7fff);

			if (data & 0x04)
				program->install_readwrite_bank(0x8000, 0xbfff, "bank3");
			else
				program->unmap_write(0x8000, 0xbfff);

			program->install_readwrite_bank(0xc000, 0xdfff, "bank4");
		}
		else
		{
			//cartridge memory is read-only
			if (data & 0x02)
				program->unmap_write(0xc000, 0xdfff);
			else
				program->install_readwrite_bank(0xc000, 0xdfff, "bank4");

			program->unmap_write(0x4000, 0xbfff);
		}
		break;
	case 6:
		break;
	}

	m_bank[offset] = data;
}

READ8_MEMBER( prestige_state::kb_r )
{
	static const char *const bitnames[2][8] =
	{
		{"LINE0", "LINE1", "LINE2", "LINE3", "LINE4", "LINE5", "LINE6", "LINE7"},
		{"LINE8", "LINE9", "LINEA", "LINEB", "LINEC", "LINED", "LINEE", "LINEF"}
	};

	UINT8 data = 0xff;

	for (int line=0; line<8; line++)
		if (!(m_kb_matrix & (1<<line)))
			data &= ioport(bitnames[offset][line])->read();

	return data;
}

WRITE8_MEMBER( prestige_state::kb_w )
{
	m_kb_matrix = data;
}

READ8_MEMBER( prestige_state::mouse_r )
{
	INT16 data = 0;

	switch( offset )
	{
		case 0:		//x-axis
			data = (ioport("MOUSEX")->read() - m_mousex);
			break;
		case 1:		//y-axis
			data = (ioport("MOUSEY")->read() - m_mousey);
			break;
	}

	return 0x80 + data;
}

WRITE8_MEMBER( prestige_state::mouse_w )
{
	switch( offset )
	{
		case 0:		//x-axis
			m_mousex = ioport("MOUSEX")->read();
			break;
		case 1:		//y-axis
			m_mousey = ioport("MOUSEY")->read();
			break;
	}
}

static ADDRESS_MAP_START(prestige_mem, AS_PROGRAM, 8, prestige_state)
	AM_RANGE(0x0000, 0x3fff) AM_ROMBANK("bank1")
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank2")
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank3")
	AM_RANGE(0xc000, 0xdfff) AM_RAMBANK("bank4")
	AM_RANGE(0xe000, 0xffff) AM_RAMBANK("bank5")
ADDRESS_MAP_END

static ADDRESS_MAP_START( prestige_io , AS_IO, 8, prestige_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x04, 0x05) AM_READWRITE(mouse_r, mouse_w)
	AM_RANGE(0x50, 0x56) AM_READWRITE(bankswitch_r, bankswitch_w)
	AM_RANGE(0x40, 0x40) AM_WRITE(kb_w)
	AM_RANGE(0x41, 0x42) AM_READ(kb_r)
ADDRESS_MAP_END

/* Input ports */
INPUT_PORTS_START( prestige )
	PORT_START("CART_TYPE")
	PORT_CONFNAME( 0x01, 0x01, "Cartridge Type" )
	PORT_CONFSETTING( 0x00, "ROM" )
	PORT_CONFSETTING( 0x01, "RAM" )

	PORT_START("MOUSEX")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X ) PORT_SENSITIVITY(10) PORT_KEYDELTA(0)

	PORT_START("MOUSEY")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y ) PORT_SENSITIVITY(10) PORT_KEYDELTA(0)

	PORT_START("LINE0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left mouse button")	PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1")	PORT_CODE(KEYCODE_1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9")	PORT_CODE(KEYCODE_9)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("e")	PORT_CODE(KEYCODE_E)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xca\xbb")	PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("g")	PORT_CODE(KEYCODE_G)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left Shift")	PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",")	PORT_CODE(KEYCODE_COMMA)

	PORT_START("LINE1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right mouse button")	PORT_CODE(MOUSECODE_BUTTON2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2")	PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0")	PORT_CODE(KEYCODE_0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("r")	PORT_CODE(KEYCODE_R)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("+")	PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("h")	PORT_CODE(KEYCODE_H)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("z")	PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".")	PORT_CODE(KEYCODE_STOP)

	PORT_START("LINE2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Mouse Up (KB)")	PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3")	PORT_CODE(KEYCODE_3)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("'")	PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("t")	PORT_CODE(KEYCODE_T)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ins") PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("j")	PORT_CODE(KEYCODE_J)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("x")	PORT_CODE(KEYCODE_X)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-")	PORT_CODE(KEYCODE_MINUS)

	PORT_START("LINE3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Mouse Left (KB)")	PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4")	PORT_CODE(KEYCODE_4)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xc2\xa1")	PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("y")	PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Caps Lock")	PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("k")	PORT_CODE(KEYCODE_K)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("c")	PORT_CODE(KEYCODE_C)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up")	PORT_CODE(KEYCODE_UP)

	PORT_START("LINE4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Mouse Right (KB)")	PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5")	PORT_CODE(KEYCODE_5)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Backspace")	PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("u")	PORT_CODE(KEYCODE_U)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("a")	PORT_CODE(KEYCODE_A)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("l")	PORT_CODE(KEYCODE_L)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("v")	PORT_CODE(KEYCODE_V)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right Shift")	PORT_CODE(KEYCODE_RSHIFT)

	PORT_START("LINE5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Mouse Down (KB)")	PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6")	PORT_CODE(KEYCODE_6)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("i")	PORT_CODE(KEYCODE_I)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("s")	PORT_CODE(KEYCODE_S)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xc3\xb1")	PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("b")	PORT_CODE(KEYCODE_B)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Help")	PORT_CODE(KEYCODE_PGUP)

	PORT_START("LINE6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("??")	PORT_CODE(KEYCODE_F10)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7")	PORT_CODE(KEYCODE_7)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("q")	PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("o")	PORT_CODE(KEYCODE_O)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("d")	PORT_CODE(KEYCODE_D)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("??")	PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("n")	PORT_CODE(KEYCODE_N)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Symbol")	PORT_CODE(KEYCODE_PGDN)

	PORT_START("LINE7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("OFF")	PORT_CODE(KEYCODE_F9)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8")	PORT_CODE(KEYCODE_8)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("w")	PORT_CODE(KEYCODE_W)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("p")	PORT_CODE(KEYCODE_P)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("f")	PORT_CODE(KEYCODE_F)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter")	PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("m")	PORT_CODE(KEYCODE_M)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Answer")	PORT_CODE(KEYCODE_END)

	PORT_START("LINE8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space")	PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Word Games")	PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Player")	PORT_CODE(KEYCODE_F2)
	PORT_BIT(0xf8, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINE9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Alt")	PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Mathematics")	PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Level")		PORT_CODE(KEYCODE_F4)
	PORT_BIT(0xf8, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINEA")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Repeat")	PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Trivia")	PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Cartridge")	PORT_CODE(KEYCODE_F6)
	PORT_BIT(0xf8, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINEB")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left")	PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Logic Games")	PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Business Basics")	PORT_CODE(KEYCODE_F8)
	PORT_BIT(0xf8, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINEC")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down")	PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0xfe, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINED")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right")	PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0xfe, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINEE")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left mouse button (KB)")	PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0xfe, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("LINEF")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right mouse button (KB)")	PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT(0xfe, IP_ACTIVE_LOW, IPT_UNUSED)

INPUT_PORTS_END

static IRQ_CALLBACK( prestige_int_ack )
{
	UINT32 vector;
	prestige_state *state = device->machine().driver_data<prestige_state>();

	state->m_maincpu->set_input_line(0, CLEAR_LINE);

	if (state->m_irq_counter == 0x02)
	{
		state->m_irq_counter = 0;
		vector = 0x0020;
	}
	else
	{
		state->m_irq_counter++;
		vector = 0x0030;
	}

	return (0xcd<<16) | vector;
}

void prestige_state::machine_start()
{
	UINT8 *ram = m_ram->pointer();
	memset(ram, 0x00, m_ram->size());

	m_maincpu->set_irq_acknowledge_callback(prestige_int_ack);

	membank("bank1")->configure_entries(0, 64, memregion("maincpu")->base(), 0x4000);
	membank("bank2")->configure_entries(0, 64, memregion("maincpu")->base(), 0x4000);
	membank("bank2")->configure_entries(64, 2, memregion("cart")->base(), 0x4000);
	membank("bank3")->configure_entries(0, 64, memregion("maincpu")->base(), 0x4000);
	membank("bank3")->configure_entries(64, 2, memregion("cart")->base(), 0x4000);
	membank("bank4")->configure_entries(0, 4, ram, 0x2000);
	membank("bank4")->configure_entries(4, 4, memregion("cart")->base(), 0x2000);
	membank("bank5")->configure_entries(0, 4, ram, 0x2000);

	membank("bank1")->set_entry(0);
	membank("bank2")->set_entry(0);
	membank("bank3")->set_entry(0);
	membank("bank4")->set_entry(0);
	membank("bank5")->set_entry(0);

	//pointer to the videoram
	m_vram = ram;
}

static PALETTE_INIT( prestige )
{
	palette_set_color(machine, 0, MAKE_RGB(39, 108, 51));
	palette_set_color(machine, 1, MAKE_RGB(16, 37, 84));
}

UINT32 prestige_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT16 addr = 0;

	for (int y = 0; y < 100; y++)
	{
		for (int sx = 0; sx < 30; sx++)
		{
			UINT8 data = m_vram[addr];

			for (int x = 0; x < 8; x++)
			{
				bitmap.pix16(y, (sx * 8) + x) = BIT(data, 7);

				data <<= 1;
			}

			addr++;
		}
	}

    return 0;
}

static TIMER_DEVICE_CALLBACK( irq_timer )
{
	timer.machine().device("maincpu")->execute().set_input_line(0, ASSERT_LINE);
}

static MACHINE_CONFIG_START( prestige, prestige_state )
    /* basic machine hardware */
    MCFG_CPU_ADD("maincpu",Z80, XTAL_4MHz)
    MCFG_CPU_PROGRAM_MAP(prestige_mem)
    MCFG_CPU_IO_MAP(prestige_io)

	MCFG_TIMER_ADD_PERIODIC("irq_timer", irq_timer, attotime::from_msec(10))

    /* video hardware */
    MCFG_SCREEN_ADD("screen", LCD)
    MCFG_SCREEN_REFRESH_RATE(50)
    MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(prestige_state, screen_update)
	MCFG_SCREEN_SIZE( 240, 100 )
	MCFG_SCREEN_VISIBLE_AREA( 0, 240-1, 0, 100-1 )

	MCFG_DEFAULT_LAYOUT( layout_lcd )

    MCFG_PALETTE_LENGTH(2)
    MCFG_PALETTE_INIT(prestige)

	/* cartridge */
	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("bin")
	MCFG_CARTSLOT_INTERFACE("prestige_cart")

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("32K")
	MCFG_RAM_EXTRA_OPTIONS("64K")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( prestige )
    ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "27-6020-02.u2", 0x00000, 0x100000, CRC(6bb6db14) SHA1(5d51fc3fd799e7f01ee99c453f9005fb07747b1e) )

	ROM_REGION( 0x8000, "cart", ROMREGION_ERASE )
	ROM_CART_LOAD( "cart", 0, 0x8000, 0 )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME       FLAGS */
COMP( 1998, prestige,  0,       0,	prestige,	prestige, driver_device,	 0,  "VTech",   "PreComputer Prestige Elite",		GAME_NOT_WORKING | GAME_NO_SOUND)
