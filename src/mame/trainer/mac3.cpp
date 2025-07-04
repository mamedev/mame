// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Mac III by L.J.Technical Systems

    Keys:
    0-9,A-F: hex input
    M      : memory display and edit
    P      : port display and edit
    L      : load from cassette
    S      : save to cassette
    G      : program run
    R      : register display and edit
    +      : up (use UP-arrow key)
    -      : down (use DOWN-arrow key)

    TODO:
    - fix cassette interface, maybe a SCN2681 issue.
    - keypad logic is guessed, need schematic.

****************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/m6502/m6502.h"
#include "imagedev/cassette.h"
#include "machine/6522via.h"
#include "machine/input_merger.h"
#include "machine/mc68681.h"

#include "speaker.h"

#include "mac3.lh"


#define VERBOSE 0
#include "logmacro.h"


namespace {

class mac3_state : public driver_device
{
public:
	static constexpr feature_type imperfect_features() { return feature::TAPE; }

	mac3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_duart(*this, "duart")
		, m_cassette(*this, "cassette")
		, m_keypad(*this, "X%u", 0U)
		, m_digits(*this, "digit%u", 0U)
	{ }

	void mac3(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<scn2681_device> m_duart;
	required_device<cassette_image_device> m_cassette;
	required_ioport_array<8> m_keypad;
	output_finder<8> m_digits;

	void mem_map(address_map &map) ATTR_COLD;

	TIMER_CALLBACK_MEMBER(cassette_timer);

	void display_latch_w(uint8_t data);
	void icm7228_write_w(int state);
	void icm7228_mode_w(int state);

	emu_timer *m_cassette_timer = nullptr;

	uint8_t m_disp_latch;
	uint8_t m_icm7228_control;
	uint8_t m_icm7228_digit;
	int m_icm7228_write;
	int m_icm7228_mode;
	uint8_t m_keycol;
};


void mac3_state::mem_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x7000, 0x7000).mirror(0x0fff).w(FUNC(mac3_state::display_latch_w));
	map(0x8000, 0x800f).mirror(0x0ff0).rw("duart", FUNC(scn2681_device::read), FUNC(scn2681_device::write));
	map(0x9000, 0x900f).mirror(0x0ff0).m("via", FUNC(via6522_device::map));
	map(0xa000, 0xbfff).rom().region("user", 0);
	map(0xc000, 0xffff).rom().region("monitor", 0);
}


static INPUT_PORTS_START( mac3 )
	PORT_START("X0")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M')

	PORT_START("X1")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P')

	PORT_START("X2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L')

	PORT_START("X3")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S')

	PORT_START("X4")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G')

	PORT_START("X5")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R')

	PORT_START("X6")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_CHAR('^')

	PORT_START("X7")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_CHAR('V')
INPUT_PORTS_END


TIMER_CALLBACK_MEMBER(mac3_state::cassette_timer)
{
	m_duart->ip6_w((m_cassette->input() > 0.0) ? 1 : 0);
}


void mac3_state::display_latch_w(uint8_t data)
{
	LOG("%s display_latch_w: %02x\n", machine().describe_context(), data);
	m_disp_latch = data;

	if (data == 0xfe)
		m_keycol = 0;
	else
		m_keycol = (m_keycol + 1) & 7;

	m_duart->ip3_w(BIT(m_keypad[m_keycol]->read(), 3));
	m_duart->ip4_w(BIT(m_keypad[m_keycol]->read(), 4));
	m_duart->ip5_w(BIT(m_keypad[m_keycol]->read(), 5));
}


void mac3_state::icm7228_write_w(int state)
{
	if (state && !m_icm7228_write)
	{
		switch (m_icm7228_mode)
		{
		case 0:
			if (BIT(m_icm7228_control, 7)) // data incoming
			{
				LOG("%s icm7228_display_w: digit %d seg %02x\n", machine().describe_context(), m_icm7228_digit, m_disp_latch);
				if (m_icm7228_digit < 8)
					m_digits[m_icm7228_digit++] = m_disp_latch ^ 0x80; // invert decimal point bit
			}
			break;

		case 1:
			// b4  0 - Shutdown,         1 - Normal Operation,
			// b5  0 - Decode,           1 - No Decode
			// b6  0 - Code B Decoding,  1 - Hexadecimal Decoding
			// b7  0 - No Data Incoming, 1 - Data Incoming
			m_icm7228_control = m_disp_latch;
			LOG("%s icm7228_control_w: %02x\n", machine().describe_context(), m_icm7228_control);
			if (BIT(m_icm7228_control, 7))
				m_icm7228_digit = 0;
			break;
		}
	}

	m_icm7228_write = state;
}

void mac3_state::icm7228_mode_w(int state)
{
	m_icm7228_mode = state;
}


void mac3_state::machine_start()
{
	m_digits.resolve();

	m_cassette_timer = timer_alloc(FUNC(mac3_state::cassette_timer), this);
	m_cassette_timer->adjust(attotime::from_hz(44100), 0, attotime::from_hz(44100));

	save_item(NAME(m_icm7228_control));
	save_item(NAME(m_icm7228_digit));
	save_item(NAME(m_keycol));
}

void mac3_state::machine_reset()
{
	m_icm7228_control = 0;
	m_icm7228_digit = 0;
	m_keycol = 0;
}


static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_2 )
DEVICE_INPUT_DEFAULTS_END


void mac3_state::mac3(machine_config &config)
{
	M6502(config, m_maincpu, 1_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mac3_state::mem_map);

	INPUT_MERGER_ANY_HIGH(config, "irqs").output_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	config.set_default_layout(layout_mac3);

	via6522_device &via(MOS6522(config, "via", 1_MHz_XTAL));
	via.irq_handler().set("irqs", FUNC(input_merger_device::in_w<0>));

	// TODO: ICM7228A device.

	scn2681_device &duart(SCN2681(config, "duart", 3.6864_MHz_XTAL));
	duart.irq_cb().set("irqs", FUNC(input_merger_device::in_w<1>));
	duart.a_tx_cb().set("rs232a", FUNC(rs232_port_device::write_txd));
	duart.b_tx_cb().set("rs232b", FUNC(rs232_port_device::write_txd));
	duart.outport_cb().set([this] (uint8_t data) { LOG("%s outport_cb %02x\n", machine().describe_context(), data); });
	duart.outport_cb().append("rs232a", FUNC(rs232_port_device::write_rts)).bit(0);
	duart.outport_cb().append("rs232b", FUNC(rs232_port_device::write_rts)).bit(1);
	duart.outport_cb().append(FUNC(mac3_state::icm7228_write_w)).bit(4);
	duart.outport_cb().append(FUNC(mac3_state::icm7228_mode_w)).bit(5);
	duart.outport_cb().append([this](int state) { m_cassette->output(state ? +1.0 : -1.0); }).bit(7);

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, nullptr));
	rs232a.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));
	rs232a.rxd_handler().set("duart", FUNC(scn2681_device::rx_a_w));
	rs232a.cts_handler().set("duart", FUNC(scn2681_device::ip0_w));
	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set("duart", FUNC(scn2681_device::rx_b_w));
	rs232b.cts_handler().set("duart", FUNC(scn2681_device::ip1_w));

	SPEAKER(config, "mono").front_center();

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
}


ROM_START( mac3 )
	ROM_REGION(0x4000, "monitor", 0)
	ROM_SYSTEM_BIOS(0, "22ai", "V2.2 Applications, I/O") // supports DT35 Applications and DT34 Input/output devices modules
	ROMX_LOAD("mac3_6502_v2.2_ai.bin", 0x0000, 0x4000, CRC(439c30e3) SHA1(a0a0b23dd67167321bbe346659da3a6e0d52aee0), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "22a",  "V2.2 Applications")      // supports DT35 Applications module (has bug in Terminal mode)
	ROMX_LOAD("mac3_6502_v2.2_a.bin",  0x0000, 0x4000, CRC(bf8f777d) SHA1(c8b624ae93d0239c8893097bcf44664457a6b2fe), ROM_BIOS(1))

	ROM_REGION(0x2000, "user", ROMREGION_ERASE00)
ROM_END

} // anonymous namespace


//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT         COMPANY                    FULLNAME                      FLAGS
COMP( 1990, mac3,   0,      0,      mac3,    mac3,   mac3_state,   empty_init,  "L.J.Technical Systems",   "Mac III 6502 Microcomputer", MACHINE_NOT_WORKING )
