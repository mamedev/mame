// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Werner Engineering 816

    https://github.com/danwerner21/we816

    The WE816 is a 16-bit 65816 based system with 512K of System RAM, 96K of
    System ROM, 32K of video RAM, a TMS9918 graphics processor and an AY-3-8910
    sound chip. The system runs a custom version of BASIC and supports Commodore
    compatible IEC disk drives.

****************************************************************************/

#include "emu.h"

#include "bus/cbmiec/cbmiec.h"
#include "bus/rs232/rs232.h"
#include "cpu/g65816/g65816.h"
#include "machine/6522via.h"
#include "machine/ds1302.h"
#include "machine/input_merger.h"
#include "machine/ins8250.h"
#include "sound/ay8910.h"
#include "video/tms9928a.h"

#include "screen.h"
#include "speaker.h"


namespace {

class we816_state : public driver_device
{
public:
	we816_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_vdp(*this, "vdp")
		, m_psg(*this, "psg")
		, m_iec(*this, CBM_IEC_TAG)
		, m_rtc(*this, "rtc")
		, m_kbd(*this, "ROW%u", 0U)
		, m_kb_caps_led(*this, "caps_led")
		, m_kb_graph_led(*this, "graph_led")
		, m_led(*this, "led%u", 1U)
	{ }

	void we816(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;

	uint8_t via1_pa_r();
	void via1_pa_w(uint8_t data);
	void via2_pb_w(uint8_t data);

	required_device<g65816_device> m_maincpu;
	required_device<tms9918a_device> m_vdp;
	required_device<ay8910_device> m_psg;
	required_device<cbm_iec_device> m_iec;
	required_device<ds1302_device> m_rtc;
	required_ioport_array<10> m_kbd;
	output_finder<> m_kb_caps_led;
	output_finder<> m_kb_graph_led;
	output_finder<2> m_led;

	uint8_t m_kbd_row = 0;
};


void we816_state::mem_map(address_map &map)
{
	map(0x000000, 0x07ffff).ram();
	map(0x008000, 0x00ffff).rom().region("maincpu", 0x8000);
	map(0x00fe00, 0x00fe07).rw("uart", FUNC(ns16550_device::ins8250_r), FUNC(ns16550_device::ins8250_w));
	map(0x00fe0a, 0x00fe0b).rw(m_vdp, FUNC(tms9918a_device::read), FUNC(tms9918a_device::write));
	map(0x00fe10, 0x00fe1f).m("via1", FUNC(via6522_device::map));
	map(0x00fe20, 0x00fe2f).m("via2", FUNC(via6522_device::map));
	map(0xff0000, 0xffffff).rom().region("maincpu", 0x0000);
}


void we816_state::machine_start()
{
}


uint8_t we816_state::via1_pa_r()
{
	uint8_t data = 0;

	data |= m_iec->clk_r()  << 0;
	data |= m_iec->data_r() << 1;
	data |= m_rtc->io_r()   << 5;
	data |= m_iec->atn_r()  << 7;

	return data;
}

void we816_state::via1_pa_w(uint8_t data)
{
	//m_psg->write_bc1_bc2();

	m_rtc->io_w(BIT(data, 5));
	m_rtc->ce_w(BIT(data, 6));
	m_iec->host_atn_w(!BIT(data, 7));
}


void we816_state::via2_pb_w(uint8_t data)
{
	m_kbd_row = data & 0x0f;

	m_kb_caps_led  = BIT(data, 4);
	m_kb_graph_led = BIT(data, 5);
	m_led[0]       = BIT(data, 6);
	m_led[1]       = BIT(data, 7);
}


static INPUT_PORTS_START(gameport)
	PORT_START("GAMEPORT_A")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("GAMEPORT_B")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(2)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


static INPUT_PORTS_START( we816 )
	PORT_START("ROW0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)           PORT_CHAR('0')  PORT_CHAR(')')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)           PORT_CHAR('1')  PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)           PORT_CHAR('2')  PORT_CHAR('@')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)           PORT_CHAR('3')  PORT_CHAR('#')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)           PORT_CHAR('4')  PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)           PORT_CHAR('5')  PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)           PORT_CHAR('6')  PORT_CHAR('^')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)           PORT_CHAR('7')  PORT_CHAR('&')

	PORT_START("ROW1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)           PORT_CHAR('8')  PORT_CHAR('*')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)           PORT_CHAR('9')  PORT_CHAR('(')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)       PORT_CHAR('-')  PORT_CHAR('_')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)      PORT_CHAR('=')  PORT_CHAR('+')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2)  PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)   PORT_CHAR('[')  PORT_CHAR('{')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)  PORT_CHAR(']')  PORT_CHAR('}')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)       PORT_CHAR(';')  PORT_CHAR(':')

	PORT_START("ROW2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)       PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)   PORT_CHAR('`')  PORT_CHAR('~')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)       PORT_CHAR(',')  PORT_CHAR('<')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)        PORT_CHAR('.')  PORT_CHAR('>')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)       PORT_CHAR('/')  PORT_CHAR('?')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RCONTROL)    PORT_CHAR(UCHAR_MAMEKEY(RCONTROL)) PORT_NAME("Dead")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)           PORT_CHAR('A')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)           PORT_CHAR('B')

	PORT_START("ROW3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)           PORT_CHAR('C')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)           PORT_CHAR('D')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)           PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)           PORT_CHAR('F')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)           PORT_CHAR('G')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)           PORT_CHAR('H')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)           PORT_CHAR('I')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)           PORT_CHAR('J')

	PORT_START("ROW4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)           PORT_CHAR('K')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)           PORT_CHAR('L')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)           PORT_CHAR('M')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)           PORT_CHAR('N')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)           PORT_CHAR('O')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)           PORT_CHAR('P')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)           PORT_CHAR('Q')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)           PORT_CHAR('R')

	PORT_START("ROW5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)           PORT_CHAR('S')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)           PORT_CHAR('T')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)           PORT_CHAR('U')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)           PORT_CHAR('V')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)           PORT_CHAR('W')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)           PORT_CHAR('X')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)           PORT_CHAR('Y')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)           PORT_CHAR('Z')

	PORT_START("ROW6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)  PORT_NAME("Shift")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)    PORT_CHAR(UCHAR_MAMEKEY(LCONTROL)) PORT_NAME("CTRL")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)        PORT_CHAR(UCHAR_MAMEKEY(LALT))     PORT_NAME("Graph")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK)    PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RALT)        PORT_CHAR(UCHAR_MAMEKEY(RALT))     PORT_NAME("Code/Kana")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)          PORT_CHAR(UCHAR_MAMEKEY(F1))       PORT_NAME("F1/F6")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)          PORT_CHAR(UCHAR_MAMEKEY(F2))       PORT_NAME("F2/F7")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)          PORT_CHAR(UCHAR_MAMEKEY(F3))       PORT_NAME("F3/F8")

	PORT_START("ROW7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)          PORT_CHAR(UCHAR_MAMEKEY(F4))       PORT_NAME("F4/F9")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)          PORT_CHAR(UCHAR_MAMEKEY(F5))       PORT_NAME("F5/F10")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)         PORT_CHAR(UCHAR_MAMEKEY(ESC))      PORT_NAME("ESC")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)         PORT_CHAR('\t')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_END)         PORT_CHAR(UCHAR_MAMEKEY(END))      PORT_NAME("Stop")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)   PORT_CHAR(8)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                                                                   PORT_NAME("Select")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)       PORT_CHAR(13)

	PORT_START("ROW8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)       PORT_CHAR(' ')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)        PORT_CHAR(UCHAR_MAMEKEY(HOME))     PORT_NAME("Home/CLS")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT)      PORT_CHAR(UCHAR_MAMEKEY(INSERT))   PORT_NAME("Insert")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)         PORT_CHAR(UCHAR_MAMEKEY(DEL))      PORT_NAME("Delete")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)        PORT_CHAR(UCHAR_MAMEKEY(LEFT))     PORT_NAME(u8"\u2190") // U+2190 = ←
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)          PORT_CHAR(UCHAR_MAMEKEY(UP))       PORT_NAME(u8"\u2191") // U+2191 = ↑
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)        PORT_CHAR(UCHAR_MAMEKEY(DOWN))     PORT_NAME(u8"\u2193") // U+2193 = ↓
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)       PORT_CHAR(UCHAR_MAMEKEY(RIGHT))    PORT_NAME(u8"\u2192") // U+2192 = →

	PORT_START("ROW9")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_INCLUDE(gameport)
INPUT_PORTS_END


void we816_state::we816(machine_config &config)
{
	G65816(config, m_maincpu, 16_MHz_XTAL / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &we816_state::mem_map);

	INPUT_MERGER_ANY_HIGH(config, "irqs").output_handler().set_inputline(m_maincpu, G65816_LINE_IRQ);

	TMS9918A(config, m_vdp, 10.738635_MHz_XTAL).set_screen("screen");
	m_vdp->set_vram_size(0x8000);
	m_vdp->int_callback().set("irqs", FUNC(input_merger_device::in_w<3>));

	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	SPEAKER(config, "mono").front_center();

	AY8910(config, m_psg, 21.477272_MHz_XTAL / 16);
	m_psg->port_a_read_callback().set_ioport("GAMEPORT_A");
	m_psg->port_b_read_callback().set_ioport("GAMEPORT_B");
	m_psg->add_route(ALL_OUTPUTS, "mono", 1.0);

	via6522_device &via1(W65C22S(config, "via1", 21.477272_MHz_XTAL / 4)); // W65C22N
	via1.readpa_handler().set(FUNC(we816_state::via1_pa_r));
	via1.writepa_handler().set(FUNC(we816_state::via1_pa_w));
	via1.readpb_handler().set(m_psg, FUNC(ay8910_device::data_r));
	via1.writepb_handler().set(m_psg, FUNC(ay8910_device::data_w));
	via1.ca2_handler().set(m_rtc, FUNC(ds1302_device::sclk_w));
	via1.irq_handler().set("irqs", FUNC(input_merger_device::in_w<1>));

	via6522_device &via2(W65C22S(config, "via2", 21.477272_MHz_XTAL / 4)); // W65C22N
	via2.readpa_handler().set([this]() { return m_kbd[m_kbd_row]->read(); });
	via2.writepb_handler().set(FUNC(we816_state::via2_pb_w));
	via2.ca2_handler().set(m_iec, FUNC(cbm_iec_device::host_clk_w)).invert();
	via2.cb1_handler().set(m_iec, FUNC(cbm_iec_device::host_srq_w)).invert();
	via2.cb2_handler().set(m_iec, FUNC(cbm_iec_device::host_data_w)).invert();
	via2.irq_handler().set("irqs", FUNC(input_merger_device::in_w<2>));

	DS1302(config, m_rtc, 32.768_kHz_XTAL);

	ns16550_device &uart(NS16550(config, "uart", 1843200));
	uart.out_tx_callback().set("serial", FUNC(rs232_port_device::write_txd));
	uart.out_rts_callback().set("serial", FUNC(rs232_port_device::write_rts));
	uart.out_int_callback().set("irqs", FUNC(input_merger_device::in_w<0>));

	rs232_port_device &serial(RS232_PORT(config, "serial", default_rs232_devices, nullptr));
	serial.rxd_handler().set("uart", FUNC(ns16550_device::rx_w));
	serial.cts_handler().set("uart", FUNC(ns16550_device::cts_w));

	cbm_iec_slot_device::add(config, m_iec, "c1571");
	m_iec->srq_callback().set("via2", FUNC(via6522_device::write_cb1));
}


ROM_START(we816)
	ROM_REGION(0x18000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("dbasic816.bin",  0x0000, 0x8000, CRC(1bd4e38f) SHA1(45473085f4251aa2697634b744254b8171709331))
	ROM_LOAD("rombios816.bin", 0x8000, 0x8000, CRC(7b6e27ad) SHA1(12a85588214911ae4650dfd58aabc1048167f688))
ROM_END

} // anonymous namespace


//    YEAR  NAME    PARENT  COMPAT MACHINE  INPUT   CLASS         INIT        COMPANY         FULLNAME     FLAGS
COMP( 2021, we816,  0,      0,     we816,   we816,  we816_state,  empty_init, "Dan Werner",   "WE816",     0 )
