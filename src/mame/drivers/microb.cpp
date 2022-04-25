// license:BSD-3-Clause
// copyright-holders:Robbbert, AJR
/***************************************************************************

BEEHIVE Micro B Series

2009-05-25 Skeleton driver [Robbbert]
2011-04-25 Added partial keyboard.
2011-06-26 Added modifier keys.

This is a series of conventional computer terminals using a serial link.
DM3270 is a clone of the IBM3276-2.

The character gen rom is not dumped. Using the one from 'c10'
 for the moment.

System beeps if ^G or ^Z pressed. Pressing ^Q is the same as Enter.

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/input_merger.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/i8257.h"
#include "machine/pit8253.h"
#include "video/i8275.h"
#include "bus/rs232/rs232.h"
#include "sound/beep.h"
#include "screen.h"
#include "speaker.h"

namespace {

class microb_state : public driver_device
{
public:
	microb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dmac(*this, "dmac")
		, m_p_chargen(*this, "chargen")
		, m_beep(*this, "beep")
		, m_usart(*this, "usart%u", 1U)
		, m_rs232(*this, "rs232%c", 'a')
		, m_io_keyboard(*this, "X%u", 0U)
	{ }

	void microb(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	DECLARE_WRITE_LINE_MEMBER(dmac_hrq_w);
	u8 dmac_mem_r(offs_t offset);
	void dmac_mem_w(offs_t offset, u8 data);
	I8275_DRAW_CHARACTER_MEMBER(draw_character);

	u8 ppi2_pa_r();
	void ppi2_pc_w(u8 data);

	void microb_io(address_map &map);
	void microb_mem(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<i8257_device> m_dmac;
	required_region_ptr<u8> m_p_chargen;
	required_device<beep_device> m_beep;
	required_device_array<i8251_device, 2> m_usart;
	required_device_array<rs232_port_device, 2> m_rs232;
	required_ioport_array<16> m_io_keyboard;

	u8 m_keyline = 0U;
};

WRITE_LINE_MEMBER(microb_state::dmac_hrq_w)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);
	m_dmac->hlda_w(state);
}

u8 microb_state::dmac_mem_r(offs_t offset)
{
	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}

void microb_state::dmac_mem_w(offs_t offset, u8 data)
{
	return m_maincpu->space(AS_PROGRAM).write_byte(offset, data);
}

u8 microb_state::ppi2_pa_r()
{
	return m_io_keyboard[m_keyline & 15]->read();
}

void microb_state::ppi2_pc_w(u8 data)
{
	m_keyline = data;
	m_beep->set_state(!BIT(data, 4));
}

void microb_state::microb_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x17ff).rom();
	map(0x8000, 0x8fff).ram();
}

void microb_state::microb_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x00, 0x09).rw(m_dmac, FUNC(i8257_device::read), FUNC(i8257_device::write));
	map(0x10, 0x13).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x20, 0x21).rw(m_usart[0], FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x30, 0x31).rw(m_usart[1], FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x50, 0x51).rw("crtc", FUNC(i8275_device::read), FUNC(i8275_device::write));
	map(0x60, 0x63).rw("ppi2", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xe0, 0xe3).rw("pit1", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0xf0, 0xf3).rw("pit2", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
}

/* Input ports */
static INPUT_PORTS_START( microb )
	PORT_START("X0")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q")  PORT_CODE(KEYCODE_Q)    PORT_CHAR('q') PORT_CHAR('Q')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W")  PORT_CODE(KEYCODE_W)    PORT_CHAR('w') PORT_CHAR('W')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E")  PORT_CODE(KEYCODE_E)    PORT_CHAR('e') PORT_CHAR('E')
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R")  PORT_CODE(KEYCODE_R)    PORT_CHAR('r') PORT_CHAR('R')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T")  PORT_CODE(KEYCODE_T)    PORT_CHAR('t') PORT_CHAR('T')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y")  PORT_CODE(KEYCODE_Y)    PORT_CHAR('y') PORT_CHAR('Y')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U")  PORT_CODE(KEYCODE_U)    PORT_CHAR('u') PORT_CHAR('U')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I")  PORT_CODE(KEYCODE_I)    PORT_CHAR('i') PORT_CHAR('I')

	PORT_START("X1")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O")  PORT_CODE(KEYCODE_O)    PORT_CHAR('o') PORT_CHAR('O')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P")  PORT_CODE(KEYCODE_P)    PORT_CHAR('p') PORT_CHAR('P')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A")  PORT_CODE(KEYCODE_A)    PORT_CHAR('a') PORT_CHAR('A')
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S")  PORT_CODE(KEYCODE_S)    PORT_CHAR('s') PORT_CHAR('S')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D")  PORT_CODE(KEYCODE_D)    PORT_CHAR('d') PORT_CHAR('D')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F")  PORT_CODE(KEYCODE_F)    PORT_CHAR('f') PORT_CHAR('F')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G")  PORT_CODE(KEYCODE_G)    PORT_CHAR('g') PORT_CHAR('G')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H")  PORT_CODE(KEYCODE_H)    PORT_CHAR('h') PORT_CHAR('H')

	PORT_START("X2")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J")  PORT_CODE(KEYCODE_J)    PORT_CHAR('j') PORT_CHAR('J')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K")  PORT_CODE(KEYCODE_K)    PORT_CHAR('k') PORT_CHAR('K')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L")  PORT_CODE(KEYCODE_L)    PORT_CHAR('l') PORT_CHAR('L')
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z")  PORT_CODE(KEYCODE_Z)    PORT_CHAR('z') PORT_CHAR('Z')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X")  PORT_CODE(KEYCODE_X)    PORT_CHAR('x') PORT_CHAR('X')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C")  PORT_CODE(KEYCODE_C)    PORT_CHAR('c') PORT_CHAR('C')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V")  PORT_CODE(KEYCODE_V)    PORT_CHAR('v') PORT_CHAR('V')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B")  PORT_CODE(KEYCODE_B)    PORT_CHAR('b') PORT_CHAR('B')

	PORT_START("X3")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N")  PORT_CODE(KEYCODE_N)    PORT_CHAR('n') PORT_CHAR('N')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M")  PORT_CODE(KEYCODE_M)    PORT_CHAR('m') PORT_CHAR('M')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("`")  PORT_CODE(KEYCODE_TILDE) PORT_CHAR('`') PORT_CHAR('~')
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[")  PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR(']')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(";")  PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("'")  PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('"')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("{")  PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('{') PORT_CHAR('}')

	PORT_START("X4")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("<")  PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('<') PORT_CHAR('>')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",")  PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".")  PORT_CODE(KEYCODE_STOP) PORT_CHAR('.')
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/")  PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1")  PORT_CODE(KEYCODE_1)    PORT_CHAR('1') PORT_CHAR('!')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2")  PORT_CODE(KEYCODE_2)    PORT_CHAR('2') PORT_CHAR('@')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3")  PORT_CODE(KEYCODE_3)    PORT_CHAR('3') PORT_CHAR('#')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4")  PORT_CODE(KEYCODE_4)    PORT_CHAR('4') PORT_CHAR('$')

	PORT_START("X5")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5")  PORT_CODE(KEYCODE_5)    PORT_CHAR('5') PORT_CHAR('%')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6")  PORT_CODE(KEYCODE_6)    PORT_CHAR('6') PORT_CHAR('^')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7")  PORT_CODE(KEYCODE_7)    PORT_CHAR('7') PORT_CHAR('&')
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8")  PORT_CODE(KEYCODE_8)    PORT_CHAR('8') PORT_CHAR('*')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9")  PORT_CODE(KEYCODE_9)    PORT_CHAR('9') PORT_CHAR('(')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0")  PORT_CODE(KEYCODE_0)    PORT_CHAR('0') PORT_CHAR(')')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-")  PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("=")  PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')

	PORT_START("X6")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7pad")   PORT_CODE(KEYCODE_7_PAD)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8pad")   PORT_CODE(KEYCODE_8_PAD)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9pad")   PORT_CODE(KEYCODE_9_PAD)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X7")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED) // Does a HOME
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Home")   PORT_CODE(KEYCODE_HOME)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4pad")   PORT_CODE(KEYCODE_4_PAD)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5pad")   PORT_CODE(KEYCODE_5_PAD)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6pad")   PORT_CODE(KEYCODE_6_PAD)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Backspace")  PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X8")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1pad")   PORT_CODE(KEYCODE_1_PAD)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2pad")   PORT_CODE(KEYCODE_2_PAD)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3pad")   PORT_CODE(KEYCODE_3_PAD)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc")    PORT_CODE(KEYCODE_ESC)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Show/Hide Status")
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down")   PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0pad")   PORT_CODE(KEYCODE_0_PAD)

	PORT_START("X9")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".pad")   PORT_CODE(KEYCODE_DEL_PAD)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-pad")   PORT_CODE(KEYCODE_MINUS_PAD)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Cursor Blink On/Off")
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED) // carriage return
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left")   PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))

	PORT_START("X10")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right")  PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space")  PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("XMIT")   PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	// This row is scanned but nothing happens
	PORT_START("X11")
		PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	// These probably not exist
	PORT_START("X12")
		PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("X13")
		PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("X14")
		PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("X15")
		PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("MODIFIERS")
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Capslock") PORT_CODE(KEYCODE_CAPSLOCK)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LCtrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RCtrl") PORT_CODE(KEYCODE_RCONTROL)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LShift") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RShift") PORT_CODE(KEYCODE_RSHIFT)
		PORT_BIT(0x83, IP_ACTIVE_LOW, IPT_UNUSED)

		// assumed to be dipswitches, purpose unknown, see code from 12D
	PORT_START("DIPS")
	PORT_DIPNAME( 0x01, 0x01, "Switch A") PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x02, 0x02, "Switch B") PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_BIT(0x3c, 0x2c, IPT_UNUSED) // this is required to sync keyboard and A-LOCK indicator
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED) // unused
	PORT_DIPNAME( 0x80, 0x80, "Switch C") PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x80, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
INPUT_PORTS_END


void microb_state::machine_start()
{
	save_item(NAME(m_keyline));
}

I8275_DRAW_CHARACTER_MEMBER(microb_state::draw_character)
{
	u8 dots = lten ? 0xff : (vsp || linecount == 9) ? 0 : m_p_chargen[(charcode << 4) | linecount];
	if (rvv)
		dots ^= 0xff;

	// HLGT is active on status line
	rgb_t const fg = hlgt ? rgb_t(0xc0, 0xc0, 0xc0) : rgb_t::white();

	u32 *pix = &bitmap.pix(y, x);
	for (int i = 0; i < 8; i++)
	{
		*pix++ = BIT(dots, 7) ? fg : rgb_t::black();
		dots <<= 1;
	}
}

void microb_state::microb(machine_config &config)
{
	/* basic machine hardware */
	I8085A(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &microb_state::microb_mem);
	m_maincpu->set_addrmap(AS_IO, &microb_state::microb_io);

	INPUT_MERGER_ANY_HIGH(config, "usartint").output_handler().set_inputline(m_maincpu, I8085_RST55_LINE);

	I8257(config, m_dmac, 2'000'000);
	m_dmac->out_hrq_cb().set(FUNC(microb_state::dmac_hrq_w));
	m_dmac->in_memr_cb().set(FUNC(microb_state::dmac_mem_r));
	m_dmac->out_memw_cb().set(FUNC(microb_state::dmac_mem_w));
	m_dmac->out_iow_cb<2>().set("crtc", FUNC(i8275_device::dack_w));
	m_dmac->out_tc_cb().set_inputline(m_maincpu, I8085_RST75_LINE);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER, rgb_t::green()));
	screen.set_raw(1'620'000 * 8, 800, 0, 640, 324, 0, 300);
	//screen.set_raw(1'620'000 * 8, 800, 0, 640, 270, 0, 250);
	screen.set_screen_update("crtc", FUNC(i8275_device::screen_update));

	i8275_device &crtc(I8275(config, "crtc", 1'620'000));
	crtc.set_screen("screen");
	crtc.set_character_width(8);
	crtc.set_display_callback(FUNC(microb_state::draw_character));
	crtc.irq_wr_callback().set_inputline(m_maincpu, I8085_RST65_LINE);
	crtc.drq_wr_callback().set(m_dmac, FUNC(i8257_device::dreq2_w));

	i8255_device &ppi1(I8255(config, "ppi1"));
	ppi1.in_pb_callback().set_ioport("DIPS");

	i8255_device &ppi2(I8255(config, "ppi2"));
	ppi2.in_pa_callback().set(FUNC(microb_state::ppi2_pa_r));
	ppi2.in_pb_callback().set_ioport("MODIFIERS");
	ppi2.out_pc_callback().set(FUNC(microb_state::ppi2_pc_w));

	pit8253_device &pit1(PIT8253(config, "pit1"));
	pit1.set_clk<2>(1'536'000);
	pit1.out_handler<2>().set(m_usart[1], FUNC(i8251_device::write_rxc));

	pit8253_device &pit2(PIT8253(config, "pit2"));
	pit2.set_clk<0>(1'536'000);
	pit2.set_clk<1>(1'536'000);
	pit2.set_clk<2>(1'536'000);
	pit2.out_handler<0>().set(m_usart[0], FUNC(i8251_device::write_txc));
	pit2.out_handler<1>().set(m_usart[0], FUNC(i8251_device::write_rxc));
	pit2.out_handler<2>().set(m_usart[1], FUNC(i8251_device::write_txc));

	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, 1000).add_route(ALL_OUTPUTS, "mono", 0.5);

	I8251(config, m_usart[0], 0);
	m_usart[0]->txd_handler().set(m_rs232[0], FUNC(rs232_port_device::write_txd));
	m_usart[0]->dtr_handler().set(m_rs232[0], FUNC(rs232_port_device::write_dtr));
	m_usart[0]->rts_handler().set(m_rs232[0], FUNC(rs232_port_device::write_rts));
	m_usart[0]->rxrdy_handler().set("usartint", FUNC(input_merger_device::in_w<0>));
	m_usart[0]->txrdy_handler().set("usartint", FUNC(input_merger_device::in_w<1>));

	RS232_PORT(config, m_rs232[0], default_rs232_devices, nullptr);
	m_rs232[0]->rxd_handler().set(m_usart[0], FUNC(i8251_device::write_rxd));
	m_rs232[0]->dsr_handler().set(m_usart[0], FUNC(i8251_device::write_dsr));
	m_rs232[0]->cts_handler().set(m_usart[0], FUNC(i8251_device::write_cts));

	I8251(config, m_usart[1], 0);
	m_usart[1]->txd_handler().set(m_rs232[1], FUNC(rs232_port_device::write_txd));
	m_usart[1]->dtr_handler().set(m_rs232[1], FUNC(rs232_port_device::write_dtr));
	m_usart[1]->rts_handler().set(m_rs232[1], FUNC(rs232_port_device::write_rts));
	m_usart[1]->rxrdy_handler().set("usartint", FUNC(input_merger_device::in_w<2>));
	m_usart[1]->txrdy_handler().set("usartint", FUNC(input_merger_device::in_w<3>));

	RS232_PORT(config, m_rs232[1], default_rs232_devices, nullptr);
	m_rs232[1]->rxd_handler().set(m_usart[1], FUNC(i8251_device::write_rxd));
	m_rs232[1]->dsr_handler().set(m_usart[1], FUNC(i8251_device::write_dsr));
	m_rs232[1]->cts_handler().set(m_usart[1], FUNC(i8251_device::write_cts));
}

/* ROM definition */
ROM_START( dm3270 )
	ROM_REGION( 0x1800, "maincpu", 0 )
	ROM_LOAD( "dm3270-1.rom", 0x0000, 0x0800, CRC(781bde32) SHA1(a3fe25baadd2bfc2b1791f509bb0f4960281ee32) )
	ROM_LOAD( "dm3270-2.rom", 0x0800, 0x0800, CRC(4d3476b7) SHA1(627ad42029ca6c8574cda8134d047d20515baf53) )
	ROM_LOAD( "dm3270-3.rom", 0x1000, 0x0800, CRC(dbf15833) SHA1(ae93117260a259236c50885c5cecead2aad9b3c4) )

	/* character generator not dumped, using the one from 'c10' for now */
	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "c10_char.bin", 0x0000, 0x2000, BAD_DUMP CRC(cb530b6f) SHA1(95590bbb433db9c4317f535723b29516b9b9fcbf))
ROM_END

} // Anonymous namespace

//    YEAR  NAME    PARENT  COMPAT  MACHINE INPU    CLASS         INIT        COMPANY                  FULLNAME                               FLAGS
COMP( 1982, dm3270, 0,      0,      microb, microb, microb_state, empty_init, "Beehive International", "DM3270 Control Unit Display Station", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
