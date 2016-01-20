// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
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
#include "rendlay.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "softlist.h"

class prestige_state : public driver_device
{
public:
	prestige_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_ram(*this, RAM_TAG),
			m_cart(*this, "cartslot"),
			m_keyboard(*this, "KEY"),
			m_cart_type(*this, "CART_TYPE"),
			m_bank1(*this, "bank1"),
			m_bank2(*this, "bank2"),
			m_bank3(*this, "bank3"),
			m_bank4(*this, "bank4"),
			m_bank5(*this, "bank5")
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<generic_slot_device> m_cart;
	required_ioport_array<16> m_keyboard;
	required_ioport m_cart_type;
	required_memory_bank m_bank1;
	required_memory_bank m_bank2;
	required_memory_bank m_bank3;
	required_memory_bank m_bank4;
	required_memory_bank m_bank5;

	UINT8 m_bank[7];
	UINT8 m_kb_matrix;
	UINT8 m_irq_counter;
	UINT8 m_mousex;
	UINT8 m_mousey;
	UINT8 *m_vram;
	struct
	{
		UINT16  addr1;
		UINT16  addr2;
		UINT8   lcd_w;
		UINT8   lcd_h;
		UINT8   fb_width;
		UINT8   split_pos;
	} m_lcdc;

	virtual void machine_start() override;
	UINT32 screen_update(int bpp, screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_1bpp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_2bpp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	memory_region *m_cart_rom;

	DECLARE_READ8_MEMBER( bankswitch_r );
	DECLARE_WRITE8_MEMBER( bankswitch_w );
	DECLARE_READ8_MEMBER( kb_r );
	DECLARE_WRITE8_MEMBER( kb_w );
	DECLARE_READ8_MEMBER( mouse_r );
	DECLARE_WRITE8_MEMBER( mouse_w );
	DECLARE_WRITE8_MEMBER( lcdc_w );
	DECLARE_PALETTE_INIT(prestige);
	DECLARE_PALETTE_INIT(glcolor);
	TIMER_DEVICE_CALLBACK_MEMBER(irq_timer);
	IRQ_CALLBACK_MEMBER(prestige_int_ack);
};


READ8_MEMBER( prestige_state::bankswitch_r )
{
	return m_bank[offset];
}

WRITE8_MEMBER( prestige_state::bankswitch_w )
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	switch (offset)
	{
	case 0:
		m_bank1->set_entry(data & 0x3f);
		break;

	case 1:
		if (!(m_bank[5] & 0x01) && (m_bank[5] & 0x02) && (m_cart_type->read() == 0x02 || m_cart->exists()))
			m_bank2->set_entry(0x40 + (data & 0x1f));
		else
			m_bank2->set_entry(data & 0x3f);
		break;

	case 2:
		if (!(m_bank[5] & 0x01) && (m_bank[5] & 0x04) && (m_cart_type->read() == 0x02 || m_cart->exists()))
			m_bank3->set_entry(0x40 + (data & 0x1f));
		else
			m_bank3->set_entry(data & 0x3f);
		break;

	case 3:
		m_bank4->set_entry(data & 0x03);
		break;

	case 4:
		m_bank5->set_entry(data & 0x03);
		break;

	case 5:
		if (m_bank[5] == data)
			break;

		if (data & 0x20)
		{
			program.install_ram(0x8000, 0xbfff, m_vram);
		}
		else if (m_cart_type->read() == 0x01)
		{
			//cartridge memory is writable
			if (data & 0x02)
				program.install_readwrite_bank(0x4000, 0x7fff, "bank2");
			else
				program.unmap_write(0x4000, 0x7fff);

			if (data & 0x04)
				program.install_readwrite_bank(0x8000, 0xbfff, "bank3");
			else
				program.unmap_write(0x8000, 0xbfff);
		}
		else
		{
			//cartridge memory is read-only
			program.unmap_write(0x4000, 0xbfff);
			program.install_read_bank(0x8000, 0xbfff, "bank3");
		}
		break;
	case 6:
		break;
	}

	m_bank[offset] = data;
}

READ8_MEMBER( prestige_state::kb_r )
{
	UINT8 data = 0xff;

	for (int line=0; line<8; line++)
		if (!(m_kb_matrix & (1<<line)))
			data &= m_keyboard[offset * 8 + line]->read();

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
		case 0:     //x-axis
			data = (ioport("MOUSEX")->read() - m_mousex);
			break;
		case 1:     //y-axis
			data = (ioport("MOUSEY")->read() - m_mousey);
			break;
	}

	data = MIN(data, +127);
	data = MAX(data, -127);

	return 0x80 + data;
}

WRITE8_MEMBER( prestige_state::mouse_w )
{
	switch( offset )
	{
		case 0:     //x-axis
			m_mousex = ioport("MOUSEX")->read();
			break;
		case 1:     //y-axis
			m_mousey = ioport("MOUSEY")->read();
			break;
	}
}

WRITE8_MEMBER( prestige_state::lcdc_w )
{
	switch(offset)
	{
		case 0x02:
			m_lcdc.lcd_w = data;
			break;
		case 0x04:
			m_lcdc.lcd_h = data;
			break;
		case 0x06:
			m_lcdc.addr1 = (m_lcdc.addr1 & 0xff00) | data;
			break;
		case 0x07:
			m_lcdc.addr1 = (m_lcdc.addr1 & 0x00ff) | (data << 8);
			break;
		case 0x08:
			m_lcdc.addr2 = (m_lcdc.addr2 & 0xff00) | data;
			break;
		case 0x09:
			m_lcdc.addr2 = (m_lcdc.addr2 & 0x00ff) | (data << 8);
			break;
		case 0x0a:
			m_lcdc.split_pos = data;
			break;
		case 0x0d:
			m_lcdc.fb_width = data;
			break;
		default:
			logerror("Unknown LCDC reg write %x = %x\n", offset, data);
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
	AM_RANGE(0x30, 0x3f) AM_WRITE(lcdc_w)
	AM_RANGE(0x40, 0x40) AM_WRITE(kb_w)
	AM_RANGE(0x41, 0x42) AM_READ(kb_r)
	AM_RANGE(0x50, 0x56) AM_READWRITE(bankswitch_r, bankswitch_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( glcolor_io , AS_IO, 8, prestige_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x30, 0x3f) AM_WRITE(lcdc_w)
	AM_RANGE(0x40, 0x40) AM_WRITE(kb_w)
	AM_RANGE(0x41, 0x42) AM_READ(kb_r)
	AM_RANGE(0x50, 0x56) AM_READWRITE(bankswitch_r, bankswitch_w)
ADDRESS_MAP_END

/* Input ports */
INPUT_PORTS_START( prestige )
	PORT_START("CART_TYPE")
	PORT_CONFNAME( 0x01, 0x00, "Cartridge Type" )
	PORT_CONFSETTING( 0x00, "ROM" )
	PORT_CONFSETTING( 0x01, "RAM" )

	PORT_START("MOUSEX")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X ) PORT_SENSITIVITY(20) PORT_KEYDELTA(2)

	PORT_START("MOUSEY")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y ) PORT_SENSITIVITY(20) PORT_KEYDELTA(2)

	PORT_START("KEY.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left mouse button")  PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1")  PORT_CODE(KEYCODE_1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9")  PORT_CODE(KEYCODE_9)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("e")  PORT_CODE(KEYCODE_E)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xca\xbb")   PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("g")  PORT_CODE(KEYCODE_G)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left Shift") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",")  PORT_CODE(KEYCODE_COMMA)

	PORT_START("KEY.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right mouse button") PORT_CODE(MOUSECODE_BUTTON2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2")  PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0")  PORT_CODE(KEYCODE_0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("r")  PORT_CODE(KEYCODE_R)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("+")  PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("h")  PORT_CODE(KEYCODE_H)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("z")  PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".")  PORT_CODE(KEYCODE_STOP)

	PORT_START("KEY.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Mouse Up (KB)")  PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3")  PORT_CODE(KEYCODE_3)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("'")  PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("t")  PORT_CODE(KEYCODE_T)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ins") PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("j")  PORT_CODE(KEYCODE_J)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("x")  PORT_CODE(KEYCODE_X)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-")  PORT_CODE(KEYCODE_MINUS)

	PORT_START("KEY.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Mouse Left (KB)")    PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4")  PORT_CODE(KEYCODE_4)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xc2\xa1")   PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("y")  PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Caps Lock")  PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("k")  PORT_CODE(KEYCODE_K)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("c")  PORT_CODE(KEYCODE_C)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)

	PORT_START("KEY.4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Mouse Right (KB)")   PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5")  PORT_CODE(KEYCODE_5)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Backspace")  PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("u")  PORT_CODE(KEYCODE_U)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("a")  PORT_CODE(KEYCODE_A)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("l")  PORT_CODE(KEYCODE_L)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("v")  PORT_CODE(KEYCODE_V)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right Shift")    PORT_CODE(KEYCODE_RSHIFT)

	PORT_START("KEY.5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Mouse Down (KB)")    PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6")  PORT_CODE(KEYCODE_6)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("i")  PORT_CODE(KEYCODE_I)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("s")  PORT_CODE(KEYCODE_S)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xc3\xb1")   PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("b")  PORT_CODE(KEYCODE_B)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Help")   PORT_CODE(KEYCODE_PGUP)

	PORT_START("KEY.6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("??") PORT_CODE(KEYCODE_F10)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7")  PORT_CODE(KEYCODE_7)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("q")  PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("o")  PORT_CODE(KEYCODE_O)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("d")  PORT_CODE(KEYCODE_D)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("??") PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("n")  PORT_CODE(KEYCODE_N)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Symbol") PORT_CODE(KEYCODE_PGDN)

	PORT_START("KEY.7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("OFF")    PORT_CODE(KEYCODE_F9)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8")  PORT_CODE(KEYCODE_8)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("w")  PORT_CODE(KEYCODE_W)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("p")  PORT_CODE(KEYCODE_P)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("f")  PORT_CODE(KEYCODE_F)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter")  PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("m")  PORT_CODE(KEYCODE_M)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Answer") PORT_CODE(KEYCODE_END)

	PORT_START("KEY.8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space")  PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Word Games") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Player") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0xf8, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Alt")    PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Mathematics")    PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Level")      PORT_CODE(KEYCODE_F4)
	PORT_BIT(0xf8, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.10")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Repeat") PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Trivia") PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Cartridge")  PORT_CODE(KEYCODE_F6)
	PORT_BIT(0xf8, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.11")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left")   PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Logic Games")    PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Business Basics")    PORT_CODE(KEYCODE_F8)
	PORT_BIT(0xf8, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.12")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down")   PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0xfe, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.13")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right")  PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0xfe, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.14")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left mouse button (KB)") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0xfe, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.15")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right mouse button (KB)")    PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT(0xfe, IP_ACTIVE_LOW, IPT_UNUSED)

INPUT_PORTS_END

INPUT_PORTS_START( glcolor )
	PORT_START("CART_TYPE")
	PORT_CONFNAME( 0x03, 0x02, "Cartridge Type" )
	PORT_CONFSETTING( 0x00, "ROM" )
	PORT_CONFSETTING( 0x01, "RAM" )
	PORT_CONFSETTING( 0x02, "Internal" )

	PORT_START("KEY.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('^')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('/')

	PORT_START("KEY.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6) PORT_NAME("Help")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)  PORT_CHAR(0x00df) PORT_CHAR('?')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('=')

	PORT_START("KEY.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')

	PORT_START("KEY.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')

	PORT_START("KEY.4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) PORT_NAME("Spieler 1")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')

	PORT_START("KEY.5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5) PORT_NAME("Spielers")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3) PORT_NAME("Stufe")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4) PORT_NAME("Symbols")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR(';')

	PORT_START("KEY.6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL) PORT_NAME("Button 1")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_END) PORT_NAME("Button 2")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME) PORT_NAME("Button 3")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGUP) PORT_NAME("Button 4")

	PORT_START("KEY.7")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KEY.8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Backspace") PORT_CHAR(UCHAR_MAMEKEY(BACKSPACE))
	PORT_BIT(0xfc, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KEY.9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(0x00fc) PORT_CHAR(0x00dc)
	PORT_BIT(0xfc, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KEY.10")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(0x00e4) PORT_CHAR(0x00c4)
	PORT_BIT(0xfc, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KEY.11")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0xfe, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KEY.12")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(0x00f6) PORT_CHAR(0x00d6)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2) PORT_NAME("Spieler 2")
	PORT_BIT(0xfc, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KEY.13")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR(':')
	PORT_BIT(0xfe, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KEY.14")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KEY.15")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

INPUT_PORTS_END

INPUT_PORTS_START( glmcolor )
	PORT_INCLUDE(glcolor)

	PORT_MODIFY("KEY.14")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(MOUSECODE_BUTTON1) PORT_NAME("Mouse Button 1")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(MOUSECODE_BUTTON2) PORT_NAME("Mouse Button 2")

	PORT_START("MOUSEX")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(2)

	PORT_START("MOUSEY")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y ) PORT_SENSITIVITY(30) PORT_KEYDELTA(2)
INPUT_PORTS_END


IRQ_CALLBACK_MEMBER(prestige_state::prestige_int_ack)
{
	UINT32 vector;

	m_maincpu->set_input_line(0, CLEAR_LINE);

	if (m_irq_counter == 0x04)
	{
		m_irq_counter = 0;
		vector = 0x0020;
	}
	else
	{
		m_irq_counter++;
		vector = 0x0030;
	}

	return (0xcd<<16) | vector;
}

void prestige_state::machine_start()
{
	std::string region_tag;
	m_cart_rom = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());

	UINT8 *rom = memregion("maincpu")->base();
	UINT8 *cart = m_cart_rom->base();
	if (cart == nullptr)
		cart = rom + 0x40000;   // internal ROM also includes extra contents that are activated by a cartridge that works as a jumper
	UINT8 *ram = m_ram->pointer();
	memset(ram, 0x00, m_ram->size());

	m_bank1->configure_entries(0, 64, rom,  0x4000);
	m_bank1->configure_entries(64,32, cart, 0x4000);
	m_bank2->configure_entries(0, 64, rom,  0x4000);
	m_bank2->configure_entries(64,32, cart, 0x4000);
	m_bank3->configure_entries(0, 64, rom,  0x4000);
	m_bank3->configure_entries(64,32, cart, 0x4000);
	m_bank4->configure_entries(0, 4,  ram,  0x2000);
	m_bank5->configure_entries(0, 4,  ram,  0x2000);

	m_bank1->set_entry(0);
	m_bank2->set_entry(0);
	m_bank3->set_entry(0);
	m_bank4->set_entry(0);
	m_bank5->set_entry(0);

	//pointer to the videoram
	m_vram = ram;
}

PALETTE_INIT_MEMBER(prestige_state, prestige)
{
	palette.set_pen_color(0, rgb_t(39, 108, 51));
	palette.set_pen_color(1, rgb_t(16, 37, 84));
}

PALETTE_INIT_MEMBER(prestige_state, glcolor)
{
	palette.set_pen_color(0, rgb_t(0x3f,0xbf,0x3f));
	palette.set_pen_color(1, rgb_t(0xff,0x3f,0x5f));
	palette.set_pen_color(2, rgb_t(0x1f,0x1f,0x3f));
	palette.set_pen_color(3, rgb_t(0xff,0xdf,0x1f));
}

UINT32 prestige_state::screen_update(int bpp, screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int width = m_lcdc.fb_width + m_lcdc.lcd_w + 1;

	for (int y = 0; y <= m_lcdc.lcd_h; y++)
	{
		int line_start;
		if (y <= m_lcdc.split_pos)
			line_start = m_lcdc.addr1 + y * width;
		else
			line_start = m_lcdc.addr2 + (y - m_lcdc.split_pos - 1) * width;

		for (int sx = 0; sx <= m_lcdc.lcd_w; sx++)
		{
			UINT8 data = m_vram[(line_start + sx) & 0x1fff];

			for (int x = 0; x < 8 / bpp; x++)
			{
				int pix = 0;
				for (int b=0; b<bpp; b++)
					pix |= BIT(data, 7 - b) << b;

				if (cliprect.contains(sx * 8 / bpp + x, y))
					bitmap.pix16(y, sx * 8 / bpp + x) = pix;

				data <<= bpp;
			}
		}
	}

	return 0;
}

UINT32 prestige_state::screen_update_1bpp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return screen_update(1, screen, bitmap, cliprect);
}

UINT32 prestige_state::screen_update_2bpp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return screen_update(2, screen, bitmap, cliprect);
}

TIMER_DEVICE_CALLBACK_MEMBER(prestige_state::irq_timer)
{
	m_maincpu->set_input_line(0, ASSERT_LINE);
}

static MACHINE_CONFIG_START( prestige_base, prestige_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_8MHz)  // Z84C008
	MCFG_CPU_PROGRAM_MAP(prestige_mem)
	MCFG_CPU_IO_MAP(prestige_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(prestige_state,prestige_int_ack)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq_timer", prestige_state, irq_timer, attotime::from_hz(200))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(prestige_state, screen_update_1bpp)
	MCFG_SCREEN_SIZE( 240, 100 )
	MCFG_SCREEN_VISIBLE_AREA( 0, 240-1, 0, 100-1 )
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEFAULT_LAYOUT( layout_lcd )

	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(prestige_state, prestige)

	/* cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "genius_cart")

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("32K")
	MCFG_RAM_EXTRA_OPTIONS("64K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( glcolor, prestige_base )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(glcolor_io)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(prestige_state, screen_update_2bpp)
	MCFG_SCREEN_SIZE( 160, 80 )
	MCFG_SCREEN_VISIBLE_AREA( 0, 160-1, 0, 80-1 )

	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_ENTRIES(4)
	MCFG_PALETTE_INIT_OWNER(prestige_state, glcolor)

	MCFG_SOFTWARE_LIST_ADD("cart_list", "glcolor")
	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("snotec_cart", "snotec")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( glmcolor, glcolor )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(prestige_io)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( snotec, glcolor )
	MCFG_SOFTWARE_LIST_REMOVE("cart_list")
	MCFG_SOFTWARE_LIST_ADD("cart_list", "snotec")
	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("glcolor_cart", "glcolor")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( prestige, prestige_base )
	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("gl6000sl_cart", "gl6000sl")
	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("misterx_cart", "misterx")
	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("gl2000_cart", "gl2000")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( gl6000sl, prestige_base )
	MCFG_SOFTWARE_LIST_ADD("cart_list", "gl6000sl")
	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("misterx_cart", "misterx")
	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("gl2000_cart", "gl2000")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( gl7007sl, prestige_base )
	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("gl6000sl_cart", "gl6000sl")
	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("gl2000_cart", "gl2000")
	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("misterx_cart", "misterx")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( gjmovie, prestige_base )
	MCFG_SOFTWARE_LIST_ADD("cart_list", "gjmovie")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( gl6000sl )
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD( "27-5894-01",   0x000000, 0x080000, CRC(7336231c) SHA1(35a1f739994b5c8fb67a7f76d423e50d8154e9ea) )
ROM_END

ROM_START( gl7007sl )
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD( "27-6060-00", 0x000000, 0x100000, CRC(06b2a595) SHA1(654d00e55ee43627ff947d72676c8e48e0518123) )
ROM_END

ROM_START( prestige )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "27-6020-02.u2", 0x00000, 0x100000, CRC(6bb6db14) SHA1(5d51fc3fd799e7f01ee99c453f9005fb07747b1e) )
ROM_END

ROM_START( gwnf )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "27-6372-00.bin", 0x00000, 0x100000, CRC(1bb574bd) SHA1(04234a33405782e8641883ebd6dee46a24e014d5) )
ROM_END

ROM_START( glcolor )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "27-5488-00.u5", 0x00000, 0x080000, CRC(e6cf7702) SHA1(ce40418a7777b331bf8c4c881d51732aeb384582) )
ROM_END

ROM_START( glscolor )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "27-5488-00.u5", 0x00000, 0x080000, CRC(e6cf7702) SHA1(ce40418a7777b331bf8c4c881d51732aeb384582) )    // identical to 'Genius Leader Color'
ROM_END

ROM_START( snotec )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "27-5616-01.u6", 0x00000, 0x080000, CRC(74093f5b) SHA1(3495b07e297315051888261d608680513a05c08b) )
ROM_END

ROM_START( snotecex )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "27-5758-00.u6", 0x00000, 0x080000, CRC(aac672be) SHA1(6ac09c3ae8c1c987072b2272cfcf34d9083431cb) )
ROM_END

ROM_START( glmcolor )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "27-5673-00.u6", 0x00000, 0x100000, CRC(c4245392) SHA1(bb651aaf11b75f4155c0a0106de9394018110cc7) )
ROM_END

ROM_START( gj4000 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "27-05886-000-000.u4", 0x000000, 0x40000, CRC(5f6db95b) SHA1(fe683154e33a82ea38696096616d11e850e0c7a3))
ROM_END

ROM_START( gj5000 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "27-6019-01.u2", 0x000000, 0x80000, CRC(946e5b7d) SHA1(80963d6ad80d49e54c8996bfc77ac135c4935be5))
ROM_END

ROM_START( gjmovie )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "lh532hlk.bin", 0x000000, 0x40000, CRC(2e64c296) SHA1(604034f902e20851cb9af60964031a508ceef83e))
ROM_END

ROM_START( gjrstar )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "27-5740-00.u1", 0x000000, 0x40000, CRC(ff3dc3bb) SHA1(bc16dfc1e12b0008456c700c431c8df6263b671f))
ROM_END

ROM_START( gjrstar2 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "27-5740-00.u1", 0x000000, 0x40000, CRC(ff3dc3bb) SHA1(bc16dfc1e12b0008456c700c431c8df6263b671f))     // identical to 'Genius Junior Redstar'
ROM_END

ROM_START( gjrstar3 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "54-06056-000-000.u3", 0x000000, 0x040000, CRC(72522179) SHA1(ede9491713ad018012cf925a519bcafe126f1ad3))
ROM_END

ROM_START( gl6600cx )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "54-06400-00.u1", 0x000000, 0x200000, CRC(b05cd075) SHA1(b1d9eb02ca56350eb9e89518db89c0a2a845ebd8))
ROM_END

ROM_START( gkidabc )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD("27-5730-00.bin", 0x00000, 0x20000, CRC(64664708) SHA1(74212c2dec1caa41dbc933b50f857904a8ac623b))
ROM_END

ROM_START( cars2lap )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD("n25s16.u6", 0x00000, 0x200000, CRC(ec1ba96e) SHA1(51b8844ae77adf20f74f268d380d268c9ce19785))
ROM_END


/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME       FLAGS */
COMP( 1994, glcolor,   0,       0,  glcolor,    glcolor,  driver_device,     0,  "VTech",   "Genius Leader Color (Germany)",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1994, glscolor,  glcolor, 0,  glcolor,    glcolor,  driver_device,     0,  "VTech",   "Genius Leader Super Color (Germany)",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1995, snotec,    0,       0,  snotec,     glcolor,  driver_device,     0,  "Bandai",  "Super Note Club (Japan)",                MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1996, snotecex,  0,       0,  snotec,     glcolor,  driver_device,     0,  "Bandai",  "Super Note Club EX (Japan)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1996, glmcolor,  0,       0,  glmcolor,   glmcolor, driver_device,     0,  "VTech",   "Genius Leader Magic Color (Germany)",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1997, gl6000sl,  0,       0,  gl6000sl,   prestige, driver_device,     0,  "VTech",   "Genius Leader 6000SL (Germany)",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1998, gl7007sl,  0,       0,  gl7007sl,   prestige, driver_device,     0,  "VTech",   "Genius Leader 7007SL (Germany)",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1998, prestige,  0,       0,  prestige,   prestige, driver_device,     0,  "VTech",   "PreComputer Prestige Elite",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1999, gwnf,      0,       0,  prestige,   prestige, driver_device,     0,  "VTech",   "Genius Winner Notebook Fun (Germany)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)


// these systems need to be moved into a separate driver
COMP( 1996, gj4000,    0,       0,  prestige,   prestige, driver_device,     0,  "VTech",   "Genius Junior 4000 (Germany)", MACHINE_IS_SKELETON)
COMP( 1996, gkidabc,   0,       0,  prestige,   prestige, driver_device,     0,  "VTech",   "Genius KID ABC Fan (Germany)", MACHINE_IS_SKELETON)
COMP( 1993, gjmovie,   0,       0,  gjmovie,    prestige, driver_device,     0,  "VTech",   "Genius Junior Movie (Germany)", MACHINE_IS_SKELETON)
COMP( 1996, gjrstar,   0,       0,  prestige,   prestige, driver_device,     0,  "VTech",   "Genius Junior Redstar(Germany)", MACHINE_IS_SKELETON)
COMP( 1996, gjrstar2,  gjrstar, 0,  prestige,   prestige, driver_device,     0,  "VTech",   "Genius Junior Redstar 2 (Germany)", MACHINE_IS_SKELETON)
COMP( 1998, gjrstar3,  0,       0,  prestige,   prestige, driver_device,     0,  "VTech",   "Genius Junior Redstar 3 (Germany)", MACHINE_IS_SKELETON)
COMP( 1998, gj5000,    0,       0,  prestige,   prestige, driver_device,     0,  "VTech",   "Genius Junior 5000 (Germany)", MACHINE_IS_SKELETON)
COMP( 2012, cars2lap,  0,       0,  prestige,   prestige, driver_device,     0,  "VTech",   "CARS 2 Laptop (Germany)", MACHINE_IS_SKELETON)


// gl6600cx use a NSC1028 system-on-a-chip designed by National Semiconductor specifically for VTech
// http://web.archive.org/web/19991127134657/http://www.national.com/news/item/0,1735,425,00.html
COMP( 1999, gl6600cx,  0,       0,  prestige,   prestige, driver_device,     0,  "VTech",   "Genius Leader 6600CX (Germany)", MACHINE_IS_SKELETON)
