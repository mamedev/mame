// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

ET-3400

2009-05-12 Skeleton driver.
2016-04-29 Added Accessory.

ETA-3400 Memory I/O Accessory
- Provides Tiny Basic, a Terminal, a Serial Interface, a Cassette
  interface, and 1k to 4k of expansion RAM. All parts are working.
- The roms are U105 (Monitor), U106 (Tiny Basic), both type NMOS2316E,
  and U108 (address decoder PROM).
- Navigating:
    LED to Monitor: D1400
    Monitor to Basic: G 1C00
    Monitor to LED: G FC00
    Basic to Monitor: BYE
- All commands in Basic and Monitor are UPPERCASE only.
- Terminal is defaulted to 9600 baud, 7 bits, 2 stop bits.


****************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "machine/74259.h"
#include "machine/6821pia.h"
#include "bus/rs232/rs232.h"
#include "imagedev/cassette.h"
#include "speaker.h"

#include "et3400.lh"

namespace {

class et3400_state : public driver_device
{
public:
	et3400_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pia(*this, "pia")
		, m_displatch(*this, "displatch%u", 1)
		, m_rs232(*this, "rs232")
		, m_cass(*this, "cassette")
		, m_x(*this, "X%u", 0U)
		, m_digit(*this, "digit%u", 1U)
	{ }

	void et3400(machine_config &config);

	void reset_key_w(int state);
	void segment_test_w(int state);

private:

	virtual void machine_start() override;

	uint8_t keypad_r(offs_t offset);
	void display_w(offs_t offset, uint8_t data);
	template <int Digit> void led_w(uint8_t data);
	uint8_t pia_ar();
	void pia_aw(uint8_t data);
	uint8_t pia_br();
	void pia_bw(uint8_t data);

	void mem_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<pia6821_device> m_pia;
	required_device_array<ls259_device, 6> m_displatch;
	required_device<rs232_port_device> m_rs232;
	required_device<cassette_image_device> m_cass;
	required_ioport_array<3> m_x;
	output_finder<6> m_digit;
};



void et3400_state::machine_start()
{
	m_digit.resolve();
}


uint8_t et3400_state::keypad_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (~offset & 4) data &= m_x[2]->read();
	if (~offset & 2) data &= m_x[1]->read();
	if (~offset & 1) data &= m_x[0]->read();

	return data;
}

void et3400_state::display_w(offs_t offset, uint8_t data)
{
	// A6-A4 decoded by IC22 (74LS42); D0 inverted by one gate of IC21 (74S00)
	uint8_t digit = (offset >> 4) & 7;
	if (digit >= 1 && digit <= 6)
		m_displatch[digit - 1]->write_bit(offset & 7, !BIT(data, 0));
}

template <int Digit>
void et3400_state::led_w(uint8_t data)
{
	// This computer sets each segment, one at a time.
	m_digit[Digit - 1] = bitswap<8>(~data, 7, 0, 1, 2, 3, 4, 5, 6);
}

// d1,2,3 = Baud rate
// d4 = gnd
// d7 = rs232 in
uint8_t et3400_state::pia_ar()
{
	return ioport("BAUD")->read() | (m_rs232->rxd_r() << 7);
}

// d0 = rs232 out
void et3400_state::pia_aw(uint8_t data)
{
	m_rs232->write_txd(BIT(data, 0));
}

// d7 = cass in
uint8_t et3400_state::pia_br()
{
	return (m_cass->input() > +0.0) << 7;
}

// d0 = cass out
void et3400_state::pia_bw(uint8_t data)
{
	m_cass->output(BIT(data, 0) ? -1.0 : +1.0);
}


void et3400_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).ram();
	map(0x1000, 0x1003).mirror(0x03fc).rw(m_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x1400, 0x23ff).rom().region("roms", 0);
	map(0xc000, 0xc0ff).r(FUNC(et3400_state::keypad_r));
	map(0xc100, 0xc1ff).w(FUNC(et3400_state::display_w));
	map(0xfc00, 0xffff).rom().region("roms", 0x1000);
}

void et3400_state::reset_key_w(int state)
{
	// delivered through MC6875 (or 74LS241 on ET-3400A)
	m_maincpu->set_input_line(INPUT_LINE_RESET, state ? CLEAR_LINE : ASSERT_LINE);

	// PIA also uses reset line
	if (!state)
		m_pia->reset();
}

void et3400_state::segment_test_w(int state)
{
	for (int d = 0; d < 6; d++)
		m_displatch[d]->clear_w(state);
}

/* Input ports */
static INPUT_PORTS_START( et3400 )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("D DO")    PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("A AUTO")  PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("7 RTI")   PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("4 INDEX") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("1 ACCA")  PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("0")       PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0xc0, 0xc0, IPT_UNUSED )

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("E EXAM")  PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("B BACK")  PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("8 SS")    PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("5 CC")    PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("2 ACCB")  PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0xe0, 0xe0, IPT_UNUSED )

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("F FWD")   PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("C CHAN")  PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("9 BR")    PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("6 SP")    PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("3 PC")    PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0xe0, 0xe0, IPT_UNUSED )

	PORT_START("BAUD")
	PORT_DIPNAME( 0x0E, 0x02, "Baud Rate" )
	PORT_DIPSETTING(    0x0E, "110" )
	PORT_DIPSETTING(    0x00, "300" )
	PORT_DIPSETTING(    0x0A, "600" )
	PORT_DIPSETTING(    0x08, "1200" )
	PORT_DIPSETTING(    0x06, "2400" )
	PORT_DIPSETTING(    0x04, "4800" )
	PORT_DIPSETTING(    0x02, "9600" )

	PORT_START("RESET") // RESET is directly next to 0, but electrically separate from key matrix
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("RESET") PORT_CODE(KEYCODE_STOP) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, et3400_state, reset_key_w)

	PORT_START("TEST") // No input mechanism for "Segment Test" defined other than shorting pins together
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Segment Test") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, et3400_state, segment_test_w)
INPUT_PORTS_END


static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_2 )
DEVICE_INPUT_DEFAULTS_END

void et3400_state::et3400(machine_config &config)
{
	/* basic machine hardware */
	M6800(config, m_maincpu, XTAL(4'000'000) / 4 ); // 1MHz with memory i/o accessory, or 500khz without it
	m_maincpu->set_addrmap(AS_PROGRAM, &et3400_state::mem_map);

	/* video hardware */
	config.set_default_layout(layout_et3400);

	// Devices
	PIA6821(config, m_pia);
	m_pia->writepa_handler().set(FUNC(et3400_state::pia_aw));
	m_pia->writepb_handler().set(FUNC(et3400_state::pia_bw));
	m_pia->readpa_handler().set(FUNC(et3400_state::pia_ar));
	m_pia->readpb_handler().set(FUNC(et3400_state::pia_br));

	RS232_PORT(config, m_rs232, default_rs232_devices, "terminal").set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	for (std::size_t i = 0; i < 6; i++)
		LS259(config, m_displatch[i]);

	m_displatch[0]->parallel_out_cb().set(FUNC(et3400_state::led_w<1>));
	m_displatch[1]->parallel_out_cb().set(FUNC(et3400_state::led_w<2>));
	m_displatch[2]->parallel_out_cb().set(FUNC(et3400_state::led_w<3>));
	m_displatch[3]->parallel_out_cb().set(FUNC(et3400_state::led_w<4>));
	m_displatch[4]->parallel_out_cb().set(FUNC(et3400_state::led_w<5>));
	m_displatch[5]->parallel_out_cb().set(FUNC(et3400_state::led_w<6>));

	SPEAKER(config, "mono").front_center();

	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
}

/* ROM definition */
ROM_START( et3400 )
	ROM_REGION( 0x1420, "roms", 0 )
	ROM_LOAD( "monitor.u105",      0x0000, 0x0800, CRC(e4142682) SHA1(785966018dd6eb097ed9bd5c7def2354ab4347db) )
	ROM_LOAD( "basic.u106",        0x0800, 0x0800, CRC(bbd6a801) SHA1(088da24bd4d923d4f196b993154c538835d10605) )
	ROM_LOAD( "et3400.ic12",       0x1000, 0x0400, CRC(2eff1f58) SHA1(38b655de7393d7a92b08276f7c14a99eaa2a4a9f) )
	ROM_LOAD_OPTIONAL("prom.u108", 0x1400, 0x0020, CRC(273025c3) SHA1(136c1cdce2a4a796c1c46e8ea4f798cdee4b549b) ) // not used
ROM_END

} // Anonymous namespace

/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY          FULLNAME                                         FLAGS */
COMP( 1976, et3400, 0,      0,      et3400,  et3400, et3400_state, empty_init, "Heath Company", "Heathkit Model ET-3400 Microprocessor Trainer", MACHINE_SUPPORTS_SAVE )
