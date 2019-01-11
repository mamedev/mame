// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

        AlphaSmart Pro

        driver by Sandro Ronco

    TODO:
    - ADB and PS/2
    - charset ROM is wrong

****************************************************************************/

#include "emu.h"
#include "cpu/mc68hc11/mc68hc11.h"
#include "machine/nvram.h"
#include "machine/ram.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"

class alphasmart_state : public driver_device
{
public:
	alphasmart_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_lcdc0(*this, "ks0066_0")
		, m_lcdc1(*this, "ks0066_1")
		, m_nvram(*this, "nvram")
		, m_ram(*this, RAM_TAG)
		, m_rambank(*this, "rambank")
		, m_keyboard(*this, "COL.%u", 0)
		, m_battery_status(*this, "BATTERY")
	{
	}

	void alphasmart(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(kb_irq);

protected:
	required_device<mc68hc11_cpu_device> m_maincpu;
	required_device<hd44780_device> m_lcdc0;
	required_device<hd44780_device> m_lcdc1;
	required_device<nvram_device> m_nvram;
	required_device<ram_device> m_ram;
	required_memory_bank m_rambank;
	required_ioport_array<16> m_keyboard;
	required_ioport m_battery_status;

	virtual void machine_start() override;
	virtual void machine_reset() override;
	void alphasmart_palette(palette_device &palette) const;
	virtual uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER(kb_r);
	DECLARE_WRITE8_MEMBER(kb_matrixl_w);
	DECLARE_WRITE8_MEMBER(kb_matrixh_w);
	DECLARE_READ8_MEMBER(port_a_r);
	virtual DECLARE_WRITE8_MEMBER(port_a_w);
	DECLARE_READ8_MEMBER(port_d_r);
	DECLARE_WRITE8_MEMBER(port_d_w);
	void update_lcdc(bool lcdc0, bool lcdc1);

	void alphasmart_io(address_map &map);
	void alphasmart_mem(address_map &map);

	uint8_t           m_matrix[2];
	uint8_t           m_port_a;
	uint8_t           m_port_d;
	std::unique_ptr<bitmap_ind16> m_tmp_bitmap;
};

class asma2k_state : public alphasmart_state
{
public:
	asma2k_state(const machine_config &mconfig, device_type type, const char *tag)
		: alphasmart_state(mconfig, type, tag)
		, m_intram(*this, "internal_ram")
	{
	}

	void asma2k(machine_config &config);

private:
	required_shared_ptr<uint8_t> m_intram;

	DECLARE_READ8_MEMBER(io_r);
	DECLARE_WRITE8_MEMBER(io_w);
	virtual DECLARE_WRITE8_MEMBER(port_a_w) override;

	void asma2k_mem(address_map &map);

	uint8_t m_lcd_ctrl;
};

INPUT_CHANGED_MEMBER(alphasmart_state::kb_irq)
{
	m_maincpu->set_input_line(MC68HC11_IRQ_LINE, HOLD_LINE);
}

READ8_MEMBER(alphasmart_state::kb_r)
{
	uint16_t matrix = (m_matrix[1]<<8) | m_matrix[0];
	uint8_t data = 0xff;

	for(int i=0; i<16; i++)
		if (!(matrix & (1<<i)))
			data &= m_keyboard[i]->read();

	return data;
}

WRITE8_MEMBER(alphasmart_state::kb_matrixl_w)
{
	m_matrix[0] = data;
}

WRITE8_MEMBER(alphasmart_state::kb_matrixh_w)
{
	m_matrix[1] = data;
}

READ8_MEMBER(alphasmart_state::port_a_r)
{
	return (m_port_a & 0xfd) | (m_battery_status->read() << 1);
}

void alphasmart_state::update_lcdc(bool lcdc0, bool lcdc1)
{
	if (m_matrix[1] & 0x04)
	{
		uint8_t lcdc_data = 0;

		if (lcdc0)
			lcdc_data |= m_lcdc0->read(BIT(m_matrix[1], 1));

		if (lcdc1)
			lcdc_data |= m_lcdc1->read(BIT(m_matrix[1], 1));

		m_port_d = (m_port_d & 0xc3) | (lcdc_data>>2);
	}
	else
	{
		uint8_t lcdc_data = (m_port_d<<2) & 0xf0;

		if (lcdc0)
			m_lcdc0->write(BIT(m_matrix[1], 1), lcdc_data);

		if (lcdc1)
			m_lcdc1->write(BIT(m_matrix[1], 1), lcdc_data);
	}
}

WRITE8_MEMBER(alphasmart_state::port_a_w)
{
	uint8_t changed = (m_port_a ^ data) & data;
	update_lcdc(changed & 0x80, changed & 0x20);
	m_rambank->set_entry(((data>>3) & 0x01) | ((data>>4) & 0x02));
	m_port_a = data;
}

READ8_MEMBER(alphasmart_state::port_d_r)
{
	return m_port_d;
}

WRITE8_MEMBER(alphasmart_state::port_d_w)
{
	m_port_d = data;
}


void alphasmart_state::alphasmart_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).bankrw("rambank");
	map(0x0000, 0x003f).noprw();   // internal registers
	map(0x0040, 0x00ff).ram();   // internal RAM
	map(0x8000, 0xffff).rom().region("maincpu", 0);
	map(0x8000, 0x8000).rw(FUNC(alphasmart_state::kb_r), FUNC(alphasmart_state::kb_matrixh_w));
	map(0xc000, 0xc000).w(FUNC(alphasmart_state::kb_matrixl_w));
}

void alphasmart_state::alphasmart_io(address_map &map)
{
	map(MC68HC11_IO_PORTA, MC68HC11_IO_PORTA).rw(FUNC(alphasmart_state::port_a_r), FUNC(alphasmart_state::port_a_w));
	map(MC68HC11_IO_PORTD, MC68HC11_IO_PORTD).rw(FUNC(alphasmart_state::port_d_r), FUNC(alphasmart_state::port_d_w));
}

READ8_MEMBER(asma2k_state::io_r)
{
	if (offset == 0x2000)
		return kb_r(space, offset);

	//else printf("unknown r: %x\n", offset);

	return 0;
}

WRITE8_MEMBER(asma2k_state::io_w)
{
	if (offset == 0x2000)
		kb_matrixh_w(space, offset, data);
	else if (offset == 0x4000)
	{
		uint8_t changed = (m_lcd_ctrl ^ data) & data;
		update_lcdc(changed & 0x01, changed & 0x02);
		m_lcd_ctrl = data;
	}

	//else printf("unknown w: %x %x\n", offset, data);
}

WRITE8_MEMBER(asma2k_state::port_a_w)
{
	if ((m_port_a ^ data) & 0x40)
	{
		address_space &space = m_maincpu->space(AS_PROGRAM);

		if (data & 0x40)
			space.install_readwrite_bank(0x0000, 0x7fff, "rambank");
		else
			space.install_readwrite_handler(0x0000, 0x7fff, read8_delegate(FUNC(asma2k_state::io_r), this), write8_delegate(FUNC(asma2k_state::io_w), this));

		// internal registers / RAM
		space.nop_readwrite(0x00, 0x3f);
		space.install_ram(0x40, 0xff, m_intram.target());
	}

	m_rambank->set_entry(((data>>4) & 0x03));
	m_port_a = data;
}


void asma2k_state::asma2k_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).bankrw("rambank");
	map(0x0000, 0x003f).noprw();   // internal registers
	map(0x0040, 0x00ff).ram().share("internal_ram");   // internal RAM
	map(0x8000, 0xffff).rom().region("maincpu", 0);
	map(0x9000, 0x9000).w(FUNC(asma2k_state::kb_matrixl_w));
}

/* Input ports */
static INPUT_PORTS_START( alphasmart )
	PORT_START("COL.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)    PORT_CHAR(UCHAR_MAMEKEY(F1))   PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)    PORT_CHAR(UCHAR_MAMEKEY(F7))   PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)     PORT_CHAR('l') PORT_CHAR('L')  PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)     PORT_CHAR('9') PORT_CHAR('(')  PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)     PORT_CHAR('o') PORT_CHAR('O')  PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)  PORT_CHAR('.') PORT_CHAR('>')  PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_START("COL.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)     PORT_CHAR('5') PORT_CHAR('%')  PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)     PORT_CHAR('g') PORT_CHAR('G')  PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)     PORT_CHAR('t') PORT_CHAR('T')  PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)     PORT_CHAR('b') PORT_CHAR('B')  PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)     PORT_CHAR('f') PORT_CHAR('F')  PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)     PORT_CHAR('4') PORT_CHAR('$')  PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)     PORT_CHAR('r') PORT_CHAR('R')  PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)     PORT_CHAR('v') PORT_CHAR('V')  PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_START("COL.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)     PORT_CHAR('6') PORT_CHAR('^')  PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)     PORT_CHAR('h') PORT_CHAR('H')  PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)     PORT_CHAR('y') PORT_CHAR('Y')  PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)     PORT_CHAR('n') PORT_CHAR('N')  PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)     PORT_CHAR('j') PORT_CHAR('J')  PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)     PORT_CHAR('7') PORT_CHAR('&')  PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)     PORT_CHAR('u') PORT_CHAR('U')  PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)     PORT_CHAR('m') PORT_CHAR('M')  PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_START("COL.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')  PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'')    PORT_CHAR('\"') PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')  PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')  PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')  PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)     PORT_CHAR('0') PORT_CHAR(')')  PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)     PORT_CHAR('p') PORT_CHAR('P')  PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("COL.4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter") PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("COL.5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))   PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))   PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_END)  PORT_CHAR(UCHAR_MAMEKEY(END))    PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_START("COL.6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LWIN) PORT_CODE(KEYCODE_PGUP)  PORT_NAME("Left Command") PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RWIN) PORT_CODE(KEYCODE_PGDN) PORT_NAME("Right Command") PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)//
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)  PORT_NAME("Clear File") PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME))   PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("COL.7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F12) PORT_NAME("Send") PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)   PORT_CHAR(UCHAR_MAMEKEY(UP))     PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT)   PORT_CHAR(UCHAR_MAMEKEY(INSERT)) PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_START("COL.8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL)) PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("COL.9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT) PORT_NAME("Left Alt/Option") PORT_CHAR(UCHAR_MAMEKEY(LALT))   PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RALT) PORT_NAME("Right Alt/Option") PORT_CHAR(UCHAR_MAMEKEY(LALT))   PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_START("COL.10")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=')    PORT_CHAR('+') PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)   PORT_CHAR(UCHAR_MAMEKEY(F6)) PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')    PORT_CHAR('}') PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)    PORT_CHAR('k')  PORT_CHAR('K')   PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)    PORT_CHAR('8')  PORT_CHAR('*')   PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)    PORT_CHAR('i')  PORT_CHAR('I')   PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')   PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_START("COL.11")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9)   PORT_NAME("Pause") PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)        PORT_CHAR(UCHAR_MAMEKEY(F5))     PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Backspace") PORT_CHAR(UCHAR_MAMEKEY(BACKSPACE)) PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)     PORT_CHAR(' ') PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|') PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)  PORT_NAME("ScrLk") PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)   PORT_NAME("Return") PORT_CHAR(13) PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_START("COL.12")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)   PORT_CHAR(UCHAR_MAMEKEY(F2))    PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)   PORT_CHAR(UCHAR_MAMEKEY(F4))    PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)   PORT_CHAR(UCHAR_MAMEKEY(F3))    PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)    PORT_CHAR('d')  PORT_CHAR('D')  PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)    PORT_CHAR('3')  PORT_CHAR('#')  PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)    PORT_CHAR('e')  PORT_CHAR('E')  PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)    PORT_CHAR('c')  PORT_CHAR('C')  PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_START("COL.13")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)   PORT_CHAR(UCHAR_MAMEKEY(F1))    PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)    PORT_CHAR('s')  PORT_CHAR('S')  PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)    PORT_CHAR('2')  PORT_CHAR('@')  PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)    PORT_CHAR('w')  PORT_CHAR('W')  PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)    PORT_CHAR('x')  PORT_CHAR('X')  PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_START("COL.14")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('`') PORT_CHAR('~')  PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)  PORT_CHAR(UCHAR_MAMEKEY(ESC))   PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)  PORT_CHAR('\t')   PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)    PORT_CHAR('a')  PORT_CHAR('A')  PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)    PORT_CHAR('1')  PORT_CHAR('!')  PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)    PORT_CHAR('q')  PORT_CHAR('Q')  PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)    PORT_CHAR('z')  PORT_CHAR('Z')  PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_START("COL.15")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1) PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_MAMEKEY(RSHIFT)) PORT_CHANGED_MEMBER(DEVICE_SELF, alphasmart_state, kb_irq, nullptr)

	PORT_START("BATTERY")
	PORT_CONFNAME(0x01, 0x01, "Battery status")
	PORT_CONFSETTING (0x00, DEF_STR(Low))
	PORT_CONFSETTING (0x01, DEF_STR(Normal))
INPUT_PORTS_END

void alphasmart_state::alphasmart_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}

uint32_t alphasmart_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_lcdc0->screen_update(screen, *m_tmp_bitmap, cliprect);
	copybitmap(bitmap, *m_tmp_bitmap, 0, 0, 0, 0, cliprect);
	m_lcdc1->screen_update(screen, *m_tmp_bitmap, cliprect);
	copybitmap(bitmap, *m_tmp_bitmap, 0, 0, 0, 18,cliprect);
	return 0;
}

void alphasmart_state::machine_start()
{
	uint8_t* ram = m_ram->pointer();

	m_rambank->configure_entries(0, 4, ram, 0x8000);
	m_nvram->set_base(ram, 0x8000*4);

	m_tmp_bitmap = std::make_unique<bitmap_ind16>(6 * 40, 9 * 4);
}

void alphasmart_state::machine_reset()
{
	m_rambank->set_entry(0);
	m_matrix[0] = m_matrix[1] = 0;
	m_port_a = 0;
	m_port_d = 0;
}

MACHINE_CONFIG_START(alphasmart_state::alphasmart)
	/* basic machine hardware */
	MC68HC11(config, m_maincpu, XTAL(8'000'000)/2);  // MC68HC11D0, XTAL is 8 Mhz, unknown divider
	m_maincpu->set_addrmap(AS_PROGRAM, &alphasmart_state::alphasmart_mem);
	m_maincpu->set_addrmap(AS_IO, &alphasmart_state::alphasmart_io);
	m_maincpu->set_config(0, 192, 0x00);

	KS0066_F05(config, m_lcdc0, 0);
	m_lcdc0->set_lcd_size(2, 40);
	KS0066_F05(config, m_lcdc1, 0);
	m_lcdc1->set_lcd_size(2, 40);

	RAM(config, RAM_TAG).set_default_size("128K");

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(alphasmart_state, screen_update)
	MCFG_SCREEN_SIZE(6*40, 9*4)
	MCFG_SCREEN_VISIBLE_AREA(0, (6*40)-1, 0, (9*4)-1)
	MCFG_SCREEN_PALETTE("palette")

	PALETTE(config, "palette", FUNC(alphasmart_state::alphasmart_palette), 2);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
MACHINE_CONFIG_END

MACHINE_CONFIG_START(asma2k_state::asma2k)
	alphasmart(config);
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_PROGRAM_MAP(asma2k_mem)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( asmapro )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "alphasmartpro212.rom",  0x0000, 0x8000, CRC(896ddf1c) SHA1(c3c6a421c9ced92db97431d04b4a3f09a39de716) )   // Checksum 8D24 on label
ROM_END

ROM_START( asma2k )
	ROM_REGION( 0x10000, "maincpu", 0 )
	/*
	    These dumps 33,253 bytes each, probably contain 32768 bytes of rom,
	    plus the remaining area is pal data for the mapper/io pal, all of
	    which is integrated onto one plcc44 chip called a zpsd211r.
	*/
	ROM_SYSTEM_BIOS( 0, "v314", "v3.14" )
	ROMX_LOAD( "alphasmart__2000__v3.1.4__h4.zpsd211r.plcc44.bin",  0x0000, 0x81e5, CRC(49487f6d) SHA1(e0b777dc68c671c31ba808e214fb9d2573b9a853), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v308", "v3.08" )
	ROMX_LOAD( "alphasmart__2000__v3.0.8.zpsd211r.plcc44.bin",  0x0000, 0x81e5, CRC(0b3b1a0c) SHA1(97878819188a1ec40052fbce9d5a5059728d5aec), ROM_BIOS(1) )

	ROM_REGION( 0x8000, "spellcheck", 0 )
	ROM_LOAD( "spellcheck.bin",  0x0000, 0x8000, NO_DUMP )
ROM_END


//    YEAR  NAME     PARENT  COMPAT  MACHINE     INPUT       CLASS             INIT        COMPANY                           FULLNAME           FLAGS
COMP( 1995, asmapro, 0,      0,      alphasmart, alphasmart, alphasmart_state, empty_init, "Intelligent Peripheral Devices", "AlphaSmart Pro" , MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1997, asma2k,  0,      0,      asma2k,     alphasmart, asma2k_state,     empty_init, "Intelligent Peripheral Devices", "AlphaSmart 2000", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
