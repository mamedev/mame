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

 VTech PC Super Color (Spain)
 __________________________________     _______________|||||||||||||____
 |                                 |   |               ||||||||||||| ___|
 |                                 |   |                             |
 |                                 |___|              TI CSM10233AN  |__
 |                                 ____      ____              __\_____ |
 |      _____________              ____     (GLOB)             |_______||
 |      |S2564RL-100 |             ____     _____                 _____ |
 |      |____________|             ____     |    |                |    ||
 |                                 ____     |Z80 |   S2564RL-100->|    ||
 |     ______________              ____     |    |                |    ||
 |     | 27-5560-01  |             ____     |    |<-Z84C0008PEC   |    ||
 |     |_____________|             |   |    |    | __________     |____||
  \                                |   |    |____| |_________|  ________|
   \_____              ____________|   |           SN74HC244N   |
         ||||||||||||||                |__|||||_____         ___|
         ||||||||||||||                            |||||||||||
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
#include "machine/timer.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"


namespace {

class prestige_state : public driver_device
{
public:
	prestige_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_cart(*this, "cartslot")
		, m_keyboard(*this, "KEY.%u", 0)
		, m_cart_type(*this, "CART_TYPE")
		, m_bank1(*this, "bank1")
		, m_bank2(*this, "bank2")
		, m_bank3(*this, "bank3")
		, m_bank4(*this, "bank4")
		, m_bank5(*this, "bank5")
	{ }

	void prestige_base(machine_config &config);
	void gl6000sl(machine_config &config);
	void snotec(machine_config &config);
	void glmcolor(machine_config &config);
	void glcolor(machine_config &config);
	void prestige(machine_config &config);
	void gl7007sl(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
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

	uint8_t m_bank[7];
	uint8_t m_kb_matrix;
	uint8_t m_irq_counter;
	uint8_t m_mousex;
	uint8_t m_mousey;
	uint8_t *m_vram;
	struct
	{
		uint16_t  addr1;
		uint16_t  addr2;
		uint8_t   lcd_w;
		uint8_t   lcd_h;
		uint8_t   fb_width;
		uint8_t   split_pos;
	} m_lcdc;

	memory_region *m_cart_rom;

	uint32_t screen_update(int bpp, screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_1bpp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_2bpp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint8_t bankswitch_r(offs_t offset);
	void bankswitch_w(offs_t offset, uint8_t data);
	uint8_t kb_r(offs_t offset);
	void kb_w(uint8_t data);
	uint8_t mouse_r(offs_t offset);
	void mouse_w(offs_t offset, uint8_t data);
	void lcdc_w(offs_t offset, uint8_t data);
	void prestige_palette(palette_device &palette) const;
	void glcolor_palette(palette_device &palette) const;
	TIMER_DEVICE_CALLBACK_MEMBER(irq_timer);
	IRQ_CALLBACK_MEMBER(prestige_int_ack);

	void glcolor_io(address_map &map) ATTR_COLD;
	void prestige_io(address_map &map) ATTR_COLD;
	void prestige_mem(address_map &map) ATTR_COLD;
};


uint8_t prestige_state::bankswitch_r(offs_t offset)
{
	return m_bank[offset];
}

void prestige_state::bankswitch_w(offs_t offset, uint8_t data)
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
				program.install_readwrite_bank(0x4000, 0x7fff, m_bank2);
			else
				program.unmap_write(0x4000, 0x7fff);

			if (data & 0x04)
				program.install_readwrite_bank(0x8000, 0xbfff, m_bank3);
			else
				program.unmap_write(0x8000, 0xbfff);
		}
		else
		{
			//cartridge memory is read-only
			program.unmap_write(0x4000, 0xbfff);
			program.install_read_bank(0x8000, 0xbfff, m_bank3);
		}
		break;
	case 6:
		break;
	}

	m_bank[offset] = data;
}

uint8_t prestige_state::kb_r(offs_t offset)
{
	uint8_t data = 0xff;

	for (int line=0; line<8; line++)
		if (!(m_kb_matrix & (1<<line)))
			data &= m_keyboard[offset * 8 + line]->read();

	return data;
}

void prestige_state::kb_w(uint8_t data)
{
	m_kb_matrix = data;
}

uint8_t prestige_state::mouse_r(offs_t offset)
{
	int16_t data = 0;

	switch( offset )
	{
		case 0:     //x-axis
			data = (ioport("MOUSEX")->read() - m_mousex);
			break;
		case 1:     //y-axis
			data = (ioport("MOUSEY")->read() - m_mousey);
			break;
	}

	data = (std::min)(data, int16_t(+127));
	data = (std::max)(data, int16_t(-127));

	return 0x80 + data;
}

void prestige_state::mouse_w(offs_t offset, uint8_t data)
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

void prestige_state::lcdc_w(offs_t offset, uint8_t data)
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


void prestige_state::prestige_mem(address_map &map)
{
	map(0x0000, 0x3fff).bankr("bank1");
	map(0x4000, 0x7fff).bankr("bank2");
	map(0x8000, 0xbfff).bankr("bank3");
	map(0xc000, 0xdfff).bankrw("bank4");
	map(0xe000, 0xffff).bankrw("bank5");
}

void prestige_state::prestige_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x04, 0x05).rw(FUNC(prestige_state::mouse_r), FUNC(prestige_state::mouse_w));
	map(0x30, 0x3f).w(FUNC(prestige_state::lcdc_w));
	map(0x40, 0x40).w(FUNC(prestige_state::kb_w));
	map(0x41, 0x42).r(FUNC(prestige_state::kb_r));
	map(0x50, 0x56).rw(FUNC(prestige_state::bankswitch_r), FUNC(prestige_state::bankswitch_w));
}

void prestige_state::glcolor_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x30, 0x3f).w(FUNC(prestige_state::lcdc_w));
	map(0x40, 0x40).w(FUNC(prestige_state::kb_w));
	map(0x41, 0x42).r(FUNC(prestige_state::kb_r));
	map(0x50, 0x56).rw(FUNC(prestige_state::bankswitch_r), FUNC(prestige_state::bankswitch_w));
}

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
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"\u2018")   PORT_CODE(KEYCODE_OPENBRACE) // U+2018 = ‘
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
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"´") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("t")  PORT_CODE(KEYCODE_T)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ins") PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("j")  PORT_CODE(KEYCODE_J)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("x")  PORT_CODE(KEYCODE_X)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-")  PORT_CODE(KEYCODE_MINUS)

	PORT_START("KEY.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Mouse Left (KB)")    PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4")  PORT_CODE(KEYCODE_4)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"¡") PORT_CODE(KEYCODE_CLOSEBRACE)
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
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"ñ") PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("b")  PORT_CODE(KEYCODE_B)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Help") PORT_CODE(KEYCODE_PGUP)

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
	uint32_t vector;

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

	uint8_t *rom = memregion("maincpu")->base();
	uint8_t *cart = nullptr;
	if (m_cart_rom != nullptr)
	{
		cart = m_cart_rom->base();
	}
	else
	{
		cart = rom + 0x40000;   // internal ROM also includes extra contents that are activated by a cartridge that works as a jumper
	}
	uint8_t *ram = m_ram->pointer();
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

	m_irq_counter = 0;

	m_lcdc.addr1 = 0;
	m_lcdc.addr2 = 0;
	m_lcdc.lcd_w = 0;
	m_lcdc.lcd_h = 0;
	m_lcdc.fb_width = 0;
	m_lcdc.split_pos = 0;

	//pointer to the videoram
	m_vram = ram;
}

void prestige_state::prestige_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(39, 108, 51));
	palette.set_pen_color(1, rgb_t(16,  37, 84));
}

void prestige_state::glcolor_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0x3f, 0xbf, 0x3f));
	palette.set_pen_color(1, rgb_t(0xff, 0x3f, 0x5f));
	palette.set_pen_color(2, rgb_t(0x1f, 0x1f, 0x3f));
	palette.set_pen_color(3, rgb_t(0xff, 0xdf, 0x1f));
}

uint32_t prestige_state::screen_update(int bpp, screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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
			uint8_t data = m_vram[(line_start + sx) & 0x1fff];

			for (int x = 0; x < 8 / bpp; x++)
			{
				int pix = 0;
				for (int b=0; b<bpp; b++)
					pix |= BIT(data, 7 - b) << b;

				if (cliprect.contains(sx * 8 / bpp + x, y))
					bitmap.pix(y, sx * 8 / bpp + x) = pix;

				data <<= bpp;
			}
		}
	}

	return 0;
}

uint32_t prestige_state::screen_update_1bpp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return screen_update(1, screen, bitmap, cliprect);
}

uint32_t prestige_state::screen_update_2bpp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return screen_update(2, screen, bitmap, cliprect);
}

TIMER_DEVICE_CALLBACK_MEMBER(prestige_state::irq_timer)
{
	m_maincpu->set_input_line(0, ASSERT_LINE);
}

void prestige_state::prestige_base(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(8'000'000));  // Z84C008
	m_maincpu->set_addrmap(AS_PROGRAM, &prestige_state::prestige_mem);
	m_maincpu->set_addrmap(AS_IO, &prestige_state::prestige_io);
	m_maincpu->set_irq_acknowledge_callback(FUNC(prestige_state::prestige_int_ack));

	TIMER(config, "irq_timer").configure_periodic(FUNC(prestige_state::irq_timer), attotime::from_hz(200));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(prestige_state::screen_update_1bpp));
	screen.set_size(240, 100);
	screen.set_visarea(0, 240-1, 0, 100-1);
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(prestige_state::prestige_palette), 2);

	/* cartridge */
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "genius_cart");

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("32K").set_extra_options("64K");
}

void prestige_state::glcolor(machine_config &config)
{
	prestige_base(config);

	m_maincpu->set_addrmap(AS_IO, &prestige_state::glcolor_io);

	/* video hardware */
	screen_device &screen(*subdevice<screen_device>("screen"));
	screen.set_screen_update(FUNC(prestige_state::screen_update_2bpp));
	screen.set_size(160, 80);
	screen.set_visarea(0, 160-1, 0, 80-1);

	subdevice<palette_device>("palette")->set_entries(4).set_init(FUNC(prestige_state::glcolor_palette));

	SOFTWARE_LIST(config, "cart_list").set_original("glcolor");
	SOFTWARE_LIST(config, "snotec_cart").set_compatible("snotec");
}

void prestige_state::glmcolor(machine_config &config)
{
	glcolor(config);

	m_maincpu->set_addrmap(AS_IO, &prestige_state::prestige_io);
}

void prestige_state::snotec(machine_config &config)
{
	glcolor(config);

	config.device_remove("snotec_cart");
	SOFTWARE_LIST(config.replace(), "cart_list").set_original("snotec");
	SOFTWARE_LIST(config, "glcolor_cart").set_compatible("glcolor");
}

void prestige_state::prestige(machine_config &config)
{
	prestige_base(config);

	SOFTWARE_LIST(config, "gl6000sl_cart").set_compatible("gl6000sl");
	SOFTWARE_LIST(config, "misterx_cart").set_compatible("misterx");
	SOFTWARE_LIST(config, "gl2000_cart").set_compatible("gl2000");
}

void prestige_state::gl6000sl(machine_config &config)
{
	prestige_base(config);

	SOFTWARE_LIST(config, "cart_list").set_original("gl6000sl");
	SOFTWARE_LIST(config, "misterx_cart").set_compatible("misterx");
	SOFTWARE_LIST(config, "gl2000_cart").set_compatible("gl2000");
}

void prestige_state::gl7007sl(machine_config &config)
{
	prestige_base(config);

	SOFTWARE_LIST(config, "gl6000sl_cart").set_compatible("gl6000sl");
	SOFTWARE_LIST(config, "gl2000_cart").set_compatible("gl2000");
	SOFTWARE_LIST(config, "misterx_cart").set_compatible("misterx");
}


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

ROM_START( pcscolor )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "27-5560-01.u5", 0x00000, 0x080000, CRC(e21e7ecd) SHA1(f3eeb19a88f1856406b357f2966880113b7340dc) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROM_LOAD( "csm10233an.u1", 0x0000, 0x2000, NO_DUMP ) // TSP50C10 (8K bytes of ROM) labeled "51CTCJT VIDEO TECH CSM10233AN"
ROM_END

ROM_START( snotec )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "27-5616-01.u6", 0x00000, 0x080000, CRC(74093f5b) SHA1(3495b07e297315051888261d608680513a05c08b) )
ROM_END

ROM_START( snotecex )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "27-5758-00.u6", 0x00000, 0x080000, CRC(aac672be) SHA1(6ac09c3ae8c1c987072b2272cfcf34d9083431cb) )
ROM_END

ROM_START( snotecu )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD("27-6100-00.u1", 0x00000, 0x100000, CRC(b2f979d5) SHA1(d2a76e99351971d1fb4cf4df9fe5741a606eb844))
ROM_END

ROM_START( snotecug )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD("27-6100-02.u1", 0x00000, 0x100000, CRC(1e14e6ea) SHA1(3e3b8dbea5f559ff98f525e3c7029b9d55e5515b))
ROM_END

ROM_START( glmcolor )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "27-5673-00.u6", 0x00000, 0x100000, CRC(c4245392) SHA1(bb651aaf11b75f4155c0a0106de9394018110cc7) )
ROM_END

ROM_START( gmmc )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "27-5889-00.bin", 0x080000, 0x080000, CRC(5e2c6359) SHA1(cc01c7bd5c87224b63dd1044db5a36a5cb7824f1) BAD_DUMP ) // very likely underdumped
	ROM_RELOAD( 0x060000, 0x020000 )
	ROM_CONTINUE( 0x040000, 0x020000 )
	ROM_CONTINUE( 0x020000, 0x020000 )
	ROM_CONTINUE( 0x000000, 0x020000 )
ROM_END

} // Anonymous namespace


/* Driver */

//    YEAR  NAME      PARENT   COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY   FULLNAME                                FLAGS
COMP( 1994, glcolor,  0,       0,      glcolor,  glcolor,  prestige_state, empty_init, "VTech",  "Genius Leader Color (Germany)",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1994, glscolor, glcolor, 0,      glcolor,  glcolor,  prestige_state, empty_init, "VTech",  "Genius Leader Super Color (Germany)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1994, pcscolor, 0,       0,      glcolor,  glcolor,  prestige_state, empty_init, "VTech",  "PC Super Color (Spain)",               MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1995, snotec,   0,       0,      snotec,   glcolor,  prestige_state, empty_init, "Bandai", "Super Note Club (Japan)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1996, snotecex, 0,       0,      snotec,   glcolor,  prestige_state, empty_init, "Bandai", "Super Note Club EX (Japan)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1996, glmcolor, 0,       0,      glmcolor, glmcolor, prestige_state, empty_init, "VTech",  "Genius Leader Magic Color (Germany)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1997, gl6000sl, 0,       0,      gl6000sl, prestige, prestige_state, empty_init, "VTech",  "Genius Leader 6000SL (Germany)",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1998, snotecu,  0,       0,      snotec,   glcolor,  prestige_state, empty_init, "Bandai", u8"Super Note Club µ (Japan)",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1999, snotecug, snotecu, 0,      snotec,   glcolor,  prestige_state, empty_init, "Bandai", u8"Super Note Club µ girlish (Japan)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1998, gl7007sl, 0,       0,      gl7007sl, prestige, prestige_state, empty_init, "VTech",  "Genius Leader 7007SL (Germany)",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1998, prestige, 0,       0,      prestige, prestige, prestige_state, empty_init, "VTech",  "PreComputer Prestige Elite",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1999, gwnf,     0,       0,      prestige, prestige, prestige_state, empty_init, "VTech",  "Genius Winner Notebook Fun (Germany)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 199?, gmmc,     0,       0,      prestige, prestige, prestige_state, empty_init, "VTech",  "Genius Master Mega Color (Germany)",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
