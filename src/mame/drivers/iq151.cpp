// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

        IQ-151

        12/05/2009 Skeleton driver.

        07/June/2011 Added screen & keyboard (by looking at the Z80 code)


This computer depends on RAM just happening to be certain values at powerup.
If the conditions are not met, it may crash.

Monitor Commands:
C Call (address)
D Dump memory, any key to dump more, Return to finish
F Fill memory (start, end, withwhat)
G Goto (address)
L Cassette load
M Move (source start, source end, destination)
R Run
S Edit memory
W Cassette save (start, end, goto (0 for null))
X Display/Edit registers


ToDo:
- Add whatever devices may exist.

- Line 32 does not scroll, should it show?
  (could be reserved for a status line in a terminal mode)

- Note that the system checks for 3E at C000, if exist, jump to C000;
  otherwise then checks for non-FF at C800, if so, jumps to C800. Could be
  extra roms or some sort of boot device.

- Key beep sounds better if clock speed changed to 1MHz, but it is still
  highly annoying. Press Ctrl-G to hear the 2-tone bell.

- Cassette support is tested only in the emulator (monitor, BASIC and AMOS),
  needs to be tested with a real cassette dump.

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/pic8259.h"
#include "machine/i8255.h"
#include "sound/speaker.h"
#include "imagedev/cassette.h"

// cartridge slot
#include "bus/iq151/iq151.h"
#include "bus/iq151/rom.h"
#include "bus/iq151/disc2.h"
#include "bus/iq151/minigraf.h"
#include "bus/iq151/ms151a.h"
#include "bus/iq151/staper.h"
#include "bus/iq151/grafik.h"
#include "bus/iq151/video32.h"
#include "bus/iq151/video64.h"

#include "softlist.h"

class iq151_state : public driver_device
{
public:
	iq151_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_pic(*this, "pic8259"),
			m_speaker(*this, "speaker"),
			m_cassette(*this, "cassette")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;

	DECLARE_READ8_MEMBER(keyboard_row_r);
	DECLARE_READ8_MEMBER(keyboard_column_r);
	DECLARE_READ8_MEMBER(ppi_portc_r);
	DECLARE_WRITE8_MEMBER(ppi_portc_w);
	DECLARE_WRITE8_MEMBER(boot_bank_w);
	DECLARE_READ8_MEMBER(cartslot_r);
	DECLARE_WRITE8_MEMBER(cartslot_w);
	DECLARE_READ8_MEMBER(cartslot_io_r);
	DECLARE_WRITE8_MEMBER(cartslot_io_w);
	virtual void machine_reset() override;
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	UINT8 m_vblank_irq_state;
	UINT8 m_cassette_clk;
	UINT8 m_cassette_data;
	iq151cart_slot_device * m_carts[5];
	DECLARE_DRIVER_INIT(iq151);
	INTERRUPT_GEN_MEMBER(iq151_vblank_interrupt);
	DECLARE_INPUT_CHANGED_MEMBER(iq151_break);
	TIMER_DEVICE_CALLBACK_MEMBER(cassette_timer);
};

READ8_MEMBER(iq151_state::keyboard_row_r)
{
	char kbdrow[6];
	UINT8 data = 0xff;

	for (int i = 0; i < 8; i++)
	{
		sprintf(kbdrow,"X%X",i);
		data &= ioport(kbdrow)->read();
	}

	return data;
}

READ8_MEMBER(iq151_state::keyboard_column_r)
{
	char kbdrow[6];
	UINT8 data = 0x00;

	for (int i = 0; i < 8; i++)
	{
		sprintf(kbdrow,"X%X",i);
		if (ioport(kbdrow)->read() == 0xff)
			data |= (1 << i);
	}

	return data;
}

READ8_MEMBER(iq151_state::ppi_portc_r)
{
	UINT8 data = 0x00;

	if (m_cassette_data & 0x06)
	{
		// cassette read
		data |= ((m_cassette_clk & 1) << 5);
		data |= (m_cassette->input() > 0.00 ? 0x80 : 0x00);
	}
	else
	{
		// kb read
		data = ioport("X8")->read();
	}

	return (data & 0xf0) | (m_cassette_data & 0x0f);
}


WRITE8_MEMBER(iq151_state::ppi_portc_w)
{
	m_speaker->level_w(BIT(data, 3));
	m_cassette_data = data;
}

WRITE8_MEMBER(iq151_state::boot_bank_w)
{
	membank("boot")->set_entry(data & 1);
}


//**************************************************************************
//  Cartridge slot emulation
//**************************************************************************

READ8_MEMBER(iq151_state::cartslot_r)
{
	UINT8 data = 0xff;

	for (auto & elem : m_carts)
		elem->read(offset, data);

	return data;
}

WRITE8_MEMBER(iq151_state::cartslot_w)
{
	for (auto & elem : m_carts)
		elem->write(offset, data);
}

READ8_MEMBER(iq151_state::cartslot_io_r)
{
	UINT8 data = 0xff;

	for (auto & elem : m_carts)
		elem->io_read(offset, data);

	return data;
}

WRITE8_MEMBER(iq151_state::cartslot_io_w)
{
	for (auto & elem : m_carts)
		elem->io_write(offset, data);
}

static ADDRESS_MAP_START(iq151_mem, AS_PROGRAM, 8, iq151_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x07ff ) AM_RAMBANK("boot")
	AM_RANGE( 0x0800, 0x7fff ) AM_RAM
	AM_RANGE( 0xf000, 0xffff ) AM_ROM

	AM_RANGE( 0x0000, 0xffff ) AM_READWRITE(cartslot_r, cartslot_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(iq151_io, AS_IO, 8, iq151_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x80, 0x80 ) AM_WRITE(boot_bank_w)
	AM_RANGE( 0x84, 0x87 ) AM_DEVREADWRITE("ppi8255", i8255_device, read, write)
	AM_RANGE( 0x88, 0x89 ) AM_DEVREADWRITE("pic8259", pic8259_device, read, write)

	AM_RANGE( 0x00, 0xff ) AM_READWRITE(cartslot_io_r, cartslot_io_w)
ADDRESS_MAP_END


INPUT_CHANGED_MEMBER(iq151_state::iq151_break)
{
	m_pic->ir5_w(newval & 1);
}

/* Input ports */
static INPUT_PORTS_START( iq151 )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1 !") PORT_CODE(KEYCODE_1)       PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2 \"") PORT_CODE(KEYCODE_2)      PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3 #") PORT_CODE(KEYCODE_3)       PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4 $") PORT_CODE(KEYCODE_4)       PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5 %") PORT_CODE(KEYCODE_5)       PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6 &") PORT_CODE(KEYCODE_6)       PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7 '") PORT_CODE(KEYCODE_7)       PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8 (") PORT_CODE(KEYCODE_8)       PORT_CHAR('8') PORT_CHAR('(')

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)     PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)     PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)     PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)     PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)     PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)     PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)     PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)     PORT_CHAR('I') PORT_CHAR('i')

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)     PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)     PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)     PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)     PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)     PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)     PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)     PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)     PORT_CHAR('K') PORT_CHAR('k')

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)     PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)     PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)     PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)     PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)     PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)     PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)     PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA)       PORT_CHAR(',') PORT_CHAR('<')

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9 )") PORT_CODE(KEYCODE_9)       PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)     PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)     PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP)        PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER)     PORT_CHAR(13)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del") PORT_CODE(KEYCODE_DEL)     PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ins") PORT_CODE(KEYCODE_INSERT)      PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)           PORT_CHAR(UCHAR_MAMEKEY(F1))

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)     PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)     PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("; +") PORT_CODE(KEYCODE_COLON)       PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH)       PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\ |") PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DL") PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)           PORT_CHAR(UCHAR_MAMEKEY(F2))

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^ ~") PORT_CODE(KEYCODE_EQUALS)      PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[ {") PORT_CODE(KEYCODE_OPENBRACE)   PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("] }") PORT_CODE(KEYCODE_CLOSEBRACE)  PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5)           PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("IL") PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Home") PORT_CODE(KEYCODE_HOME)       PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4)           PORT_CHAR(UCHAR_MAMEKEY(F4))

	PORT_START("X7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("- =") PORT_CODE(KEYCODE_MINUS)       PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@ `") PORT_CODE(KEYCODE_TILDE)       PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(": *") PORT_CODE(KEYCODE_QUOTE)       PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)     PORT_CHAR(' ')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("_") PORT_CODE(KEYCODE_END) // its actually some sort of graphic character
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)       PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)           PORT_CHAR(UCHAR_MAMEKEY(F3))

	PORT_START("X8")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT)  PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("FA") PORT_CODE(KEYCODE_RSHIFT)       // Function A
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("FB") PORT_CODE(KEYCODE_RCONTROL)     // Function B

	PORT_START("BREAK")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("BREAK") PORT_CODE(KEYCODE_ESC)   PORT_CHANGED_MEMBER(DEVICE_SELF, iq151_state, iq151_break, 0)  PORT_CHAR(UCHAR_MAMEKEY(ESC))
INPUT_PORTS_END


INTERRUPT_GEN_MEMBER(iq151_state::iq151_vblank_interrupt)
{
	m_pic->ir6_w(m_vblank_irq_state & 1);
	m_vblank_irq_state ^= 1;
}

TIMER_DEVICE_CALLBACK_MEMBER(iq151_state::cassette_timer)
{
	m_cassette_clk ^= 1;

	m_cassette->output((m_cassette_data & 1) ^ (m_cassette_clk & 1) ? +1 : -1);
}

DRIVER_INIT_MEMBER(iq151_state,iq151)
{
	UINT8 *RAM = memregion("maincpu")->base();
	membank("boot")->configure_entry(0, RAM + 0xf800);
	membank("boot")->configure_entry(1, RAM + 0x0000);

	// keep machine pointers to slots
	m_carts[0] = machine().device<iq151cart_slot_device>("slot1");
	m_carts[1] = machine().device<iq151cart_slot_device>("slot2");
	m_carts[2] = machine().device<iq151cart_slot_device>("slot3");
	m_carts[3] = machine().device<iq151cart_slot_device>("slot4");
	m_carts[4] = machine().device<iq151cart_slot_device>("slot5");
}

void iq151_state::machine_reset()
{
	membank("boot")->set_entry(0);

	m_vblank_irq_state = 0;
}

// this machine don't have a built-in video controller, but uses external cartridge
UINT32 iq151_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	for (auto & elem : m_carts)
		elem->video_update(bitmap, cliprect);

	return 0;
}

static SLOT_INTERFACE_START(iq151_cart)
	SLOT_INTERFACE("video32", IQ151_VIDEO32)            // video32
	SLOT_INTERFACE("video64", IQ151_VIDEO64)            // video64
	SLOT_INTERFACE("grafik" , IQ151_GRAFIK)             // Grafik
	SLOT_INTERFACE("disc2"  , IQ151_DISC2)              // Disc 2
	SLOT_INTERFACE("minigraf" , IQ151_MINIGRAF)         // Aritma Minigraf 0507
	SLOT_INTERFACE("ms151a" , IQ151_MS151A)             // MS151A XY Plotter
	SLOT_INTERFACE("staper" , IQ151_STAPER)             // STAPER
	SLOT_INTERFACE("basic6" , IQ151_BASIC6)             // BASIC6
	SLOT_INTERFACE("basicg" , IQ151_BASICG)             // BASICG
	SLOT_INTERFACE("amos1"  , IQ151_AMOS1)              // AMOS cart 1
	SLOT_INTERFACE("amos2"  , IQ151_AMOS2)              // AMOS cart 2
	SLOT_INTERFACE("amos3"  , IQ151_AMOS3)              // AMOS cart 3
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( iq151, iq151_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8080, XTAL_2MHz)
	MCFG_CPU_PROGRAM_MAP(iq151_mem)
	MCFG_CPU_IO_MAP(iq151_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", iq151_state,  iq151_vblank_interrupt)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic8259", pic8259_device, inta_cb)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(iq151_state, screen_update)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 32*8-1, 0, 32*8-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_MONOCHROME_GREEN("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_PIC8259_ADD("pic8259", INPUTLINE("maincpu", 0), VCC, NULL)

	MCFG_DEVICE_ADD("ppi8255", I8255, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(iq151_state, keyboard_row_r))
	MCFG_I8255_IN_PORTB_CB(READ8(iq151_state, keyboard_column_r))
	MCFG_I8255_IN_PORTC_CB(READ8(iq151_state, ppi_portc_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(iq151_state, ppi_portc_w))

	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED)
	MCFG_CASSETTE_INTERFACE("iq151_cass")

	MCFG_TIMER_DRIVER_ADD_PERIODIC("cassette_timer", iq151_state, cassette_timer, attotime::from_hz(2000))

	/* cartridge */
	MCFG_DEVICE_ADD("slot1", IQ151CART_SLOT, 0)
	MCFG_DEVICE_SLOT_INTERFACE(iq151_cart, nullptr, false)
	MCFG_IQ151CART_SLOT_OUT_IRQ0_CB(DEVWRITELINE("pic8259", pic8259_device, ir0_w))
	MCFG_IQ151CART_SLOT_OUT_IRQ1_CB(DEVWRITELINE("pic8259", pic8259_device, ir1_w))
	MCFG_IQ151CART_SLOT_OUT_IRQ2_CB(DEVWRITELINE("pic8259", pic8259_device, ir2_w))
	MCFG_IQ151CART_SLOT_OUT_IRQ3_CB(DEVWRITELINE("pic8259", pic8259_device, ir3_w))
	MCFG_IQ151CART_SLOT_OUT_IRQ4_CB(DEVWRITELINE("pic8259", pic8259_device, ir4_w))
	MCFG_DEVICE_ADD("slot2", IQ151CART_SLOT, 0)
	MCFG_DEVICE_SLOT_INTERFACE(iq151_cart, nullptr, false)
	MCFG_IQ151CART_SLOT_OUT_IRQ0_CB(DEVWRITELINE("pic8259", pic8259_device, ir0_w))
	MCFG_IQ151CART_SLOT_OUT_IRQ1_CB(DEVWRITELINE("pic8259", pic8259_device, ir1_w))
	MCFG_IQ151CART_SLOT_OUT_IRQ2_CB(DEVWRITELINE("pic8259", pic8259_device, ir2_w))
	MCFG_IQ151CART_SLOT_OUT_IRQ3_CB(DEVWRITELINE("pic8259", pic8259_device, ir3_w))
	MCFG_IQ151CART_SLOT_OUT_IRQ4_CB(DEVWRITELINE("pic8259", pic8259_device, ir4_w))
	MCFG_DEVICE_ADD("slot3", IQ151CART_SLOT, 0)
	MCFG_DEVICE_SLOT_INTERFACE(iq151_cart, nullptr, false)
	MCFG_IQ151CART_SLOT_OUT_IRQ0_CB(DEVWRITELINE("pic8259", pic8259_device, ir0_w))
	MCFG_IQ151CART_SLOT_OUT_IRQ1_CB(DEVWRITELINE("pic8259", pic8259_device, ir1_w))
	MCFG_IQ151CART_SLOT_OUT_IRQ2_CB(DEVWRITELINE("pic8259", pic8259_device, ir2_w))
	MCFG_IQ151CART_SLOT_OUT_IRQ3_CB(DEVWRITELINE("pic8259", pic8259_device, ir3_w))
	MCFG_IQ151CART_SLOT_OUT_IRQ4_CB(DEVWRITELINE("pic8259", pic8259_device, ir4_w))
	MCFG_DEVICE_ADD("slot4", IQ151CART_SLOT, 0)
	MCFG_DEVICE_SLOT_INTERFACE(iq151_cart, nullptr, false)
	MCFG_IQ151CART_SLOT_OUT_IRQ0_CB(DEVWRITELINE("pic8259", pic8259_device, ir0_w))
	MCFG_IQ151CART_SLOT_OUT_IRQ1_CB(DEVWRITELINE("pic8259", pic8259_device, ir1_w))
	MCFG_IQ151CART_SLOT_OUT_IRQ2_CB(DEVWRITELINE("pic8259", pic8259_device, ir2_w))
	MCFG_IQ151CART_SLOT_OUT_IRQ3_CB(DEVWRITELINE("pic8259", pic8259_device, ir3_w))
	MCFG_IQ151CART_SLOT_OUT_IRQ4_CB(DEVWRITELINE("pic8259", pic8259_device, ir4_w))
	MCFG_DEVICE_ADD("slot5", IQ151CART_SLOT, 0)
	MCFG_DEVICE_SLOT_INTERFACE(iq151_cart, "video32", false)
	MCFG_IQ151CART_SLOT_OUT_IRQ0_CB(DEVWRITELINE("pic8259", pic8259_device, ir0_w))
	MCFG_IQ151CART_SLOT_OUT_IRQ1_CB(DEVWRITELINE("pic8259", pic8259_device, ir1_w))
	MCFG_IQ151CART_SLOT_OUT_IRQ2_CB(DEVWRITELINE("pic8259", pic8259_device, ir2_w))
	MCFG_IQ151CART_SLOT_OUT_IRQ3_CB(DEVWRITELINE("pic8259", pic8259_device, ir3_w))
	MCFG_IQ151CART_SLOT_OUT_IRQ4_CB(DEVWRITELINE("pic8259", pic8259_device, ir4_w))

	/* Software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list", "iq151_cart")
	MCFG_SOFTWARE_LIST_ADD("flop_list", "iq151_flop")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( iq151 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE )
	/* A number of bios versions here. The load address is shown for each */
	ROM_SYSTEM_BIOS( 0, "orig", "Original" )
	ROMX_LOAD( "iq151_monitor_orig.rom", 0xf000, 0x1000, CRC(acd10268) SHA1(4d75c73f155ed4dc2ac51a9c22232f869cca95e2),ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "disasm", "Disassembler" )
	ROMX_LOAD( "iq151_monitor_disasm.rom", 0xf000, 0x1000, CRC(45c2174e) SHA1(703e3271a124c3ef9330ae399308afd903316ab9),ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 2, "cpm", "CPM" )
	ROMX_LOAD( "iq151_monitor_cpm.rom", 0xf000, 0x1000, CRC(26f57013) SHA1(4df396edc375dd2dd3c82c4d2affb4f5451066f1),ROM_BIOS(3))
	ROM_SYSTEM_BIOS( 3, "cpmold", "CPM (old)" )
	ROMX_LOAD( "iq151_monitor_cpm_old.rom", 0xf000, 0x1000, CRC(6743e1b7) SHA1(ae4f3b1ba2511a1f91c4e8afdfc0e5aeb0fb3a42),ROM_BIOS(4))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT   COMPANY   FULLNAME       FLAGS */
COMP( 198?, iq151,  0,       0,      iq151,     iq151, iq151_state,   iq151, "ZPA Novy Bor", "IQ-151", 0 )
